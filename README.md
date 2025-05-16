F429486

Temperature Sensor Control


This project uses an Arduino in order to read temperatures in cycles and determine the correct sample rate based on past readings and variations in temperature. It dynamically adjusts its power mode based on real-time termperature data. In the final task it implements a moving average in order to predict future trends. 
Hardware requirements that I have used in my project include an Arduino Uno and a Grove Temperature Sensor. The software used in this project were, Google colab, GitHub, Arduino Ide, Python, C and Excel.

For task 1, I have used GitHub to update and track my updates of the project. I store the program and data files of my project so that I can acess them from anywhere. I commit changes so that my changes can be tracked and rolled back on if need be. 

In task 2, I analysed the given code in order to chose the answers for the multiple choice questions. Once i had set up my arduino to run the code, I took a photo of it to upload to the repository.

For task 3.1, I implemented the function collect_temperature_data() and moved the code about collecting temperature into it, I then added a for loop to iterate through a number of times, i then made a variable and set that to be 180 for 3 minutes. It had a delay of 1 second
at the end of the loop so that it would take 3 minutes to collect data spread out every second interval. Every second it would print the new temperature.

Task 3.2 required me to apply a Discrete Fourier Transform, to do this, the function creates a malloc half the size of the signal as it would only mirror the first half. It then iterates through half of the frequencies for the same reason, computes the real and imaginary components
of the signal to work out the magnitude of it. It stores this in the malloc for magnitudes and then returns a pointer to this.

For question 3.3 I made the function send_data_to_pc(). For the Time domain part, it iterates through the number of samples and prints the time in seconds,the temperature and two 0s, all seperated by commas for Csv format. It works out the time based on the iteration through the for
loop and the current interval time of the system. It gets the temperature from the collected temperature data from task 3.1. For the frequency domain, it prints two 0s, then the frequency, worked out by the iteration through half of the sample count multiplied by the sample frequency
all divided by the sample count. It then prints the magnitude of the current iteration with the data from apply_dft().

In question 3.4, I had to make a function that changed the power mode between 3 different ones laid out in the coursework brief. It iterated through and found the average frequency of each cycle and returned different values based on how much the temperature was fluctuating.

In question 3.5, to convert data from my program to a csv file, I seperated the printing of the Time, Temperature, Frequency and Magnitude with commas to print in the format of csv.
#I then used the Microsoft Data Streamer for Excel to read the time-domain and frequency-domain data, changed the size of the Data Streamer to be 270 tiles, as Frequency Domain
#can be half the size to use less memory as it reflects after the 0.5 point. I then pasted these values into a fully empty Excel file and saved it as a .csv.

Question 3.6 required me to use this csv file in python to draw two graphs, one for each domain. In this, it loads the csv file and allocates each row to a variable, time, temperature, frequency and magnitude. It then seperates it into frequency domain and time domain by checking
which ones are 0 and the other is bigger than 0 and making masks to determine when they start and end. Then it plots two graphs for both.

In question 4, I followed the specification by making the cycles where it collects data for one minute in active mode, then it runs the power mode function to determine the next mode. If it swaps to Idle mode, it repeats 5 times before going to power down unless it swaps to another beforehand.
It also calculates the dominant frequency and changes the sampling rate based on this by doubling it for Nyquist's theorem. It also stays between 0.5 and 4.0 Hz to maintain efficiency. I also implemented a moving average of the past 10 cycled. It uses this to help predict the next
mode. For memory limitations, it clears the arrays at the end of the loop. In situations like the frequency domain calculations, it only iterates through the sample size divided by 2 as it reflects halfway. This helps to run into less storage issues. I changed the ouput to
look like the brief:

Collecting temperature data for 1 minute...
Predicted Variation: 0.00, Dominant Frequency: 0.07 Hz, New Sampling Rate: 0.50 Hz, Power Mode: IDLE

I have 6 files in my reporsitory that I have added and updated since starting this project, I have the notebook page 24WSA024_JNB_Coursework_Arduino_Programming.ipynb that has all of the instructions and answers for the different tasks in it. I also have a picture of my arduino
setup titled ArduinoTemp.jpg for proof of arduino connections. TemperatureData.ino is the arduino code after completing task 4 on the jupyter notebook. The Csv file threeminstemperature_F429486 has the temperature data over 3 minues automatically captured by Excel.
graph.py is the python code that plots graphs with the data in the Csv file.
