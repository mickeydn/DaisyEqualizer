# Equalizer

## Author

Kim Bjerge

## Description

Implements a four-band stereo equalizer with peak EQ bands.



Switch 1 turns the equalizer on/off (LED 1).

Switch 2 selects the EQ band. LED 2 shows the active EQ band (Red band 1, Green band 2, Blue band 3, White band 4)

The encoder adjusts the EQ gain of the active EQ band in the interval 0.1 - 6.0.



The EQ settings and processing timer are transmitted over a USB COM port at 9600 BAUD.



Band 0 Freq 100 Q 8 Gain 5.0 Duration 4370 ns (166667 ns)

Band 1 Freq 500 Q 8 Gain 2.0 Duration 4330 ns (166667 ns)

Band 2 Freq 2000 Q 8 Gain 0.5 Duration 4320 ns (166667 ns)

Band 3 Freq 8000 Q 8 Gain 2.0 Duration 4330 ns (166667 ns)



The duration 4370 ns is the processing time for all four bands, and the 166.7 us is the time it takes to receive 8 samples at a 48 kHz sampling rate.

