# DaisyAudio
This repository contains audio processing C++ examples for the Daisy platform family (DaisyPod)

This repository requires the Daisy tool chain to be installed. See tutorial below.

https://daisy.audio/tutorials/cpp-dev-env/

The DaisyAutio must be cloned to the Daisy/DaisyExamples/ folder of the Daisy installation.

Install the Python library piserial (pip install piserial) to execute the comPortPlotData.py.

The comPortPlotData.py needs a parameter for the COM port connected to the Daisy Pod hardware.

It receives data from the programs "FFT" or "EqualizerFFT" and plots the magnitude of the FFT.


