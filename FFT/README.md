# FFT

## Author

Kim Bjerge

## Description

Implements an FFT filter with a size of 8192 samples.

It computes the magnitude that is transmitted over the COM port every 1 second.

The ASCII values are transmitted over the COM port (9600 BAUD) as:



\#BEGIN


Frequency,Amplitude


f1.f1,a1.a1


...


f4096.f4096,a4096.a4096


#END

