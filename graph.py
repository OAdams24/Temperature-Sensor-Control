import numpy as np
import matplotlib.pyplot as plt

filename = "threeminstemperature_F429486.csv"
data = np.genfromtxt(filename, delimiter=',')

time = data[:, 0]
temperature = data[:, 1]
frequency = data[:, 2]
magnitude = data[:, 3]

freqMask = (frequency > 0) & (time == 0)
timeMask = (time > 0) & (frequency == 0)

timeVals = time[timeMask]
tempVals = temperature[timeMask]

freqVals = frequency[freqMask]
magVals = magnitude[freqMask]

plt.subplot(2, 1, 1)
plt.plot(timeVals, tempVals, marker='o', linestyle='-')
plt.title("Temperature over 3 Minutes (Time Domain)")
plt.xlabel("Time (s)")
plt.ylabel("Temperature (C)")
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(freqVals, magVals, marker='x', linestyle='-', color='r')
plt.title("Frequency Components (Frequency Domain)")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude")
plt.grid(True)

plt.tight_layout()
plt.show()

avgTemp = np.mean(tempVals)
peakFreqIndex = np.argmax(magVals)
peakFreq = freqVals[peakFreqIndex]
