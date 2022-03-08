# Project Name
TARGET = ChorusPetal

# Sources
CPP_SOURCES = main.cpp
CPP_SOURCES += chorus_processor.cpp

# Additional source files need to be added manually
# It's more trouble than its worth to alter the core makefile
DINGUS_DSP_DIR = ../../DingusDSP/source
CPP_SOURCES += $(DINGUS_DSP_DIR)/effects/chorus_engine.cpp
CPP_SOURCES += $(DINGUS_DSP_DIR)/oscillators/oscillator.cpp
CPP_SOURCES += $(DINGUS_DSP_DIR)/filters/biquad.cpp
CPP_SOURCES += $(DINGUS_DSP_DIR)/filters/cascade.cpp

# Library Locations
LIBDAISY_DIR = ../../DaisyExamples/libdaisy
DAISYSP_DIR = ../../DaisyExamples/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
