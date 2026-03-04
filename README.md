# DaisyAudio
This repository contains audio processing C++ examples for the Daisy platform family (DaisyPod)

This repository requires the Daisy tool chain to be installed. See tutorial below.

https://daisy.audio/tutorials/cpp-dev-env/

The DaisyAudio repository must be cloned to the Daisy/DaisyExamples/ folder of the Daisy installation.

Install the Python library pyserial (pip install pyserial) to execute the comPortPlotData.py.

$ pip install pyserial 

$ python comPortPlotData.py COM4

The comPortPlotData.py needs a parameter for the COM port connected to the Daisy Pod hardware.

It receives data from the programs "FFT" or "EqualizerFFT" and plots the magnitude of the FFT.

# Visual Code

Use file -> open folder for the project to execute

Run: 

- task build_all
- task build_and_program  - require STM debugger
- task build_and_debug - require STM debugger
- task build_and_program_dfu - Set Pod in programming mode 



