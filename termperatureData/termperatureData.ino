#include <math.h>
#include <stdlib.h>

const int B = 4275000;            // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A0;     // Grove - Temperature Sensor connect to A0

#if defined(ARDUINO_ARCH_AVR)
#define debug  Serial
#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
#define debug  SerialUSB
#else
#define debug  Serial
#endif

int interval = 1;

// Collect temperature data dynamically
float* collect_temperature_data(int interval, int duration, int* outSampleCount) {
    int totalSamples = duration / interval;
    *outSampleCount = totalSamples;

    float* temperatureData = (float*) malloc(totalSamples * sizeof(float));

    for (int x = 0; x < totalSamples; x++) {
        int a = analogRead(pinTempSensor);
        float R = 1023.0 / a - 1.0;
        R = R0 * R;

        float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
        temperatureData[x] = temperature;

        Serial.print("temperature = ");
        Serial.println(temperature);

        delay(1000 * interval);  // delay in milliseconds
    }

    return temperatureData;
}

// Apply DFT to the collected data
float* apply_dft(float* data, int N, float sampFrequency) {
    float* magnitudes = (float*) malloc((N / 2) * sizeof(float));  // Only half due to symmetry

    for (int k = 0; k < N / 2; k++) {
        float real = 0.0;
        float imag = 0.0;

        for (int n = 0; n < N; n++) {
            float angle = 2.0 * PI * k * n / N;
            real += data[n] * cos(angle);
            imag -= data[n] * sin(angle);
        }

        magnitudes[k] = sqrt(real * real + imag * imag);
        float freq = (k * sampFrequency) / N;

    }

    return magnitudes;
}

void send_data_to_pc(float* timeData, float* freqMag, int sampleCount, float interval){
    float sampFreq = 1.0/interval;
     for (int i = 0; i < sampleCount; i++) {
        float time = i*interval;
        Serial.print(time,2);                       // Time in seconds
        Serial.print(", ");
        Serial.print(timeData[i], 4);          // Temperature
        Serial.print(", ");
        Serial.print("0");                     // Frequency (not applicable for time domain)
        Serial.print(", ");
        Serial.println("0");                   // Magnitude (not applicable for time domain)
    }


     for (int k = 0; k < sampleCount / 2; k++) {
        float freq = (k * sampFreq) / sampleCount;

        Serial.print("0");                     // Time (not applicable for frequency domain)
        Serial.print(", ");
        Serial.print("0");                     // Temperature (not applicable for frequency domain)
        Serial.print(", ");
        Serial.print(freq, 4);                 // Frequency
        Serial.print(", ");
        Serial.println(freqMag[k], 4);  // Magnitude
    }
}

int decide_power_mode(float* freqMagnitude, int sampleCount, float sampFreq){
    float sum =0.0;
    float magSum = 0.0;
    int number = sampleCount/2;

    for(int x =0;x<number;x++){
        float freq = (x* sampFreq) /sampleCount;
        float mag = freqMagnitude[x];

        sum += freq * mag;
        magSum += mag;
    }

    float averageFreq = (magSum != 0) ? (sum / magSum) : 0.0;

    if(averageFreq > 0.5){
        return 1;
    }
    else if(averageFreq <= 0.5 && averageFreq >= 0.1){
        return 5;
    }
    else{
        return 30;
    }
}

void setup() {
    Serial.begin(9600);
}

void loop() {
                
    int duration = 3 * 60;       
    int sampleCount = 0;

    float* tempData = collect_temperature_data(interval, duration, &sampleCount);
    float* freqMagnitudes = apply_dft(tempData, sampleCount, 1.0 / interval);

    send_data_to_pc(tempData, freqMagnitudes, sampleCount, 1.0 / interval);

    free(tempData);
    free(freqMagnitudes);

    interval = decide_power_mode(freqMagnitudes, sampleCount, 1.0/interval);

    while (true);
}
