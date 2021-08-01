# Project Name
TARGET = ChorusPetal

# Sources
CPP_SOURCES = main.cpp
CPP_SOURCES += chorus_processor.cpp
CPP_SOURCES += dsp/chorus_engine.cpp
CPP_SOURCES += dsp/oscillator.cpp

# Library Locations
LIBDAISY_DIR = ../../DaisyExamples/libdaisy
DAISYSP_DIR = ../../DaisyExamples/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
