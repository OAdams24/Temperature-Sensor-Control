#include <math.h>
#include <stdlib.h>

// Thermistor parameters
const int B = 4275000;            // B-value
const int R0 = 100000;            // Resistance at 25°C
const int pinTempSensor = A0;     // Analog pin for temp sensor

// Serial interface setup
#if defined(ARDUINO_ARCH_AVR)
#define debug  Serial
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
#define debug  SerialUSB
#else
#define debug  Serial
#endif

// Power mode states
#define ACTIVE 1
#define IDLE 2
#define POWER_DOWN 3

// Global state
int mode = 0;
int idleCycleCount = 0;
float threshold = 5.0;

// Sampling rate boundaries
float samplingRate = 1.0;
float minSamplingRate = 0.5;
float maxSamplingRate = 4.0;

// History buffer for temp variation trend
#define HISTORY_SIZE 10
float tempDiffHistory[HISTORY_SIZE] = {0};
int historyIndex = 0;
bool historyFull = false;
int lowFluctuationCount = 0;

// Rolling average of temp variation
float compute_moving_average() {
    float sum = 0.0;
    int count = historyFull ? HISTORY_SIZE : historyIndex;

    for (int i = 0; i < count; i++) {
        sum += tempDiffHistory[i];
    }

    return (count > 0) ? (sum / count) : 0.0;
}

// Output system status over serial
void print_status_report(float trendAvg, float dominantFreq, float desiredRate, int mode) {
    Serial.print("Predicted Variation: ");
    Serial.print(trendAvg, 2);
    Serial.print(", Dominant Frequency: ");
    Serial.print(dominantFreq, 2);
    Serial.print(" Hz, New Sampling Rate: ");
    Serial.print(desiredRate, 2);
    Serial.print(" Hz, Power Mode: ");

    switch (mode) {
        case ACTIVE:
            Serial.println("ACTIVE");
            break;
        case IDLE:
            Serial.println("IDLE");
            break;
        case POWER_DOWN:
            Serial.println("POWER DOWN");
            break;
        default:
            Serial.println("UNKNOWN");
    }
}

// Read temperature values at fixed intervals
float* collect_temperature_data(float interval, int duration, int* outSampleCount) {
    int totalSamples = duration / interval;
    *outSampleCount = totalSamples;

    float* temperatureData = (float*) malloc(totalSamples * sizeof(float));

    for (int x = 0; x < totalSamples; x++) {
        int a = analogRead(pinTempSensor);
        float R = 1023.0 / a - 1.0;
        R = R0 * R;

        float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
        temperatureData[x] = temperature;

        //Serial.println(temperature);
        delay(1000 * interval);  // Delay in ms
    }

    return temperatureData;
}

// Basic DFT implementation
float* apply_dft(float* data, int N, float sampFrequency, float* outDominantFreq) {
    float* magnitudes = (float*) malloc((N / 2) * sizeof(float));
    float maxMag = 0.0;
    int dominantIndex = 0;

    for (int k = 0; k < N / 2; k++) {
        float real = 0.0;
        float imag = 0.0;

        for (int n = 0; n < N; n++) {
            float angle = 2.0 * PI * k * n / N;
            real += data[n] * cos(angle);
            imag -= data[n] * sin(angle);
        }

        float mag = sqrt(real * real + imag * imag);
        magnitudes[k] = mag;

        if (mag > maxMag && k != 0) {
            maxMag = mag;
            dominantIndex = k;
        }
    }

    *outDominantFreq = (dominantIndex * sampFrequency) / N;
    return magnitudes;
}

// Print both time and frequency data
void send_data_to_pc(float* timeData, float* freqMag, int sampleCount, float interval) {
    float sampFreq = 1.0 / interval;

    for (int i = 0; i < sampleCount; i++) {
        float time = i * interval;
        Serial.print(time, 2);            // Time (s)
        Serial.print(", ");
        Serial.print(timeData[i], 4);     // Temperature
        Serial.print(", 0, 0\n");         // No freq/mag for time domain
    }

    for (int k = 0; k < sampleCount / 2; k++) {
        float freq = (k * sampFreq) / sampleCount;
        Serial.print("0, 0, ");
        Serial.print(freq, 4);            // Frequency
        Serial.print(", ");
        Serial.println(freqMag[k], 4);    // Magnitude
    }
}

// Pick a power mode based on frequency activity
int decide_power_mode(float* freqMagnitude, int sampleCount, float sampFreq) {
    float sum = 0.0;
    float magSum = 0.0;
    int number = sampleCount / 2;

    for (int x = 1; x < number; x++) {
        float freq = (x * sampFreq) / sampleCount;
        float mag = freqMagnitude[x];

        sum += freq * mag;
        magSum += mag;
    }

    float averageFreq = (magSum != 0) ? (sum / magSum) : 0.0;

    if (averageFreq > 0.5) return ACTIVE;
    else if (averageFreq > 0.1) return IDLE;
    else return POWER_DOWN;
}

void setup() {
    Serial.begin(9600);
}

void loop() {
    int duration = 60;
    int sampleCount = 0;
    float interval = 1.0 / samplingRate;

    Serial.println("Collecting temperature data for 1 minute...");

    float* tempData = collect_temperature_data(interval, duration, &sampleCount);
    float dominantFreq = 0.0;
    float* freqMagnitudes = apply_dft(tempData, sampleCount, samplingRate, &dominantFreq);

    //send_data_to_pc(tempData, freqMagnitudes, sampleCount, interval);

    // Compute short-term variation
    float variation = 0.0;
    for (int i = 1; i < sampleCount; i++) {
        variation += fabs(tempData[i] - tempData[i - 1]);
    }

    float avgDiff = variation / (sampleCount - 1);

    // Update trend history
    tempDiffHistory[historyIndex++] = avgDiff;
    if (historyIndex >= HISTORY_SIZE) {
        historyIndex = 0;
        historyFull = true;
    }

    float trendAvg = compute_moving_average();
    bool predictedStable = trendAvg < threshold;

    // Pick power mode
    int freqMode = decide_power_mode(freqMagnitudes, sampleCount, samplingRate);

    if (predictedStable && freqMode == IDLE) {
        lowFluctuationCount++;
        mode = (lowFluctuationCount >= 5) ? POWER_DOWN : IDLE;
    } else if (!predictedStable || freqMode == ACTIVE) {
        mode = ACTIVE;
        lowFluctuationCount = 0;
    } else {
        mode = freqMode;
    }

    // Adapt sampling rate
    float desiredRate = dominantFreq * 2.0;
    if (predictedStable) desiredRate *= 0.75;

    if (desiredRate < minSamplingRate) desiredRate = minSamplingRate;
    else if (desiredRate > maxSamplingRate) desiredRate = maxSamplingRate;

    samplingRate = desiredRate;

    // Show status
    print_status_report(trendAvg, dominantFreq, desiredRate, mode);

    // Cleanup
    free(tempData);
    free(freqMagnitudes);
}
