# Project Name
TARGET = PitchDetector

# Sources
CPP_SOURCES = Application.cpp Controller.cpp PitchDetector.cpp AutoCorrelation.cpp Algorithm.cpp YinDetector.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Library Flags
LDFLAGS += -u _printf_float

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
