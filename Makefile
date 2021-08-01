# Project Name
TARGET = ChorusPetal

# Sources
CPP_SOURCES = ChorusPetal.cpp
CPP_SOURCES += ChorusProcessor.cpp
CPP_SOURCES += dsp/modulated_delay.cpp
CPP_SOURCES += dsp/oscillator.cpp

# Library Locations
LIBDAISY_DIR = ../../DaisyExamples/libdaisy
DAISYSP_DIR = ../../DaisyExamples/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
