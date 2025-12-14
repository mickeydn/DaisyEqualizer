# EqualizerFFT

## Author

Kim Bjerge

## Description

Implements a four-band stereo equalizer with peak EQ bands (Default Q=8).

Switch 1 turns the equalizer on/off (LED 1).

Switch 2 selects the EQ band. LED 2 shows the active EQ band (Red band 1, Green band 2, Blue band 3, White band 4)

The encoder adjusts the EQ gain of the active EQ band in the interval 0.1 - 6.0.

A FFT with 4096 bins sampling at 48 kHz is processed.

The FFT magnitude and EQ settings are transmitted over a USB COM port at 9600 BAUD.

The EQ settings are transmitted as:

\#BAND 1 Freq 100.0 Gain x.x
#BAND 2 Freq 500.0 Gain x.x
#BAND 3 Freq 2000.0 Gain x.x
#BAND 4 Freq 8000.0 Gain x.x

The FFT magnitude is transmitted as:

\#BEGIN
Frequency,Amplitude
f1.f1,a1.a1
...
f2048.f2048,a2048.a2048
#END

