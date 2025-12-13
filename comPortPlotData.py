# -*- coding: utf-8 -*-
"""
Created on Sun Nov 30 09:01:32 2025

@author: Kim Bjerge
"""

import sys
import serial
import pandas as pd
import matplotlib.pyplot as plt
from io import StringIO
import numpy as np

BAUD = 9600   # change if needed
EPS = 1e-12   # prevents log(0)
USE_REF_LEVEL = False  # Normalize 0 dB to peak amplitude in each frame
ALPHA = 0.5 # ALPHA = 1.0 no filtering alternative expontential filtering 

class ExpFilter:

    def __init__(self, alpha):
        self.alpha = alpha
        self.y_1 = 0.0
    
    def Process(self, x):    
        y = self.alpha*x + (1.0 - self.alpha)*self.y_1
        self.y_1 = y
        return y
    
    def ProcessBlock(self, X):
        Y = np.zeros(len(X))
        for i in range(len(X)):
            Y[i] = self.Process(X[i])
        return Y            

def update_plot(df: pd.DataFrame):
    
    # Convert amplitude to dB
    amp = df["Amplitude"].values + EPS
    expFilter = ExpFilter(ALPHA)
    amp_filt = expFilter.ProcessBlock(amp)
    
    length = len(df["Frequency"])
    ref = np.max(amp) if USE_REF_LEVEL else length
    amp_db = 20 * np.log10(amp_filt / ref)
    
    line.set_xdata(df["Frequency"])
    #line.set_ydata(df["Amplitude"])
    line.set_ydata(amp_db)
    ax.set_ylabel("Amplitude (dB)")
    ax.relim()
    ax.autoscale_view()
    ax.set_xlim(20, 15000) # Limit frequency to audio band
    ax.set_xscale('log') # Log frequency axis
    plt.draw()
    plt.pause(0.01)

if __name__ == '__main__':

    # -----------------------------
    # Read COM port from argument
    # -----------------------------
    if len(sys.argv) < 2:
        print("Usage: python comPortPlotData.py <COM_PORT>")
    #    print("Example: python comPortPlotData.py COM4")
        PORT = "COM4"
        sys.exit(1)
    else:
        PORT = sys.argv[1]
            
    buffer = ""
    block_active = False
    
    plt.ion()
    fig, ax = plt.subplots()
    line, = ax.plot([], [], '.g')
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Amplitude")
    ax.set_title("Real-time Spectrum")

    print(f"Opening serial port {PORT} ...")
    ser = serial.Serial(PORT, BAUD, timeout=1)
    
    print("Listening...  (CTRL+C to stop)")
    
    try:
        while True:
            data = ser.readline().decode(errors='ignore').strip()
    
            if "#BAND" in data:
                if "#BAND R1" in data:
                    print("===========================")
                print(data)
                
            if data == "#BEGIN":
                block_active = True
                buffer = ""
                continue
    
            if data == "#END":
                block_active = False
                try:
                    df = pd.read_csv(StringIO(buffer))
                    update_plot(df)
                except Exception as e:
                    print("CSV parse error:", e)
                continue
    
            if block_active:
                buffer += data + "\n"
    
    except KeyboardInterrupt:
        print("\nStopping...")
    
    finally:
        ser.close()
        plt.ioff()
        plt.close(fig)
        print("Serial port closed. Program exited.")
        