# Author: Andrea Martelloni (a.martelloni@qmul.ac.uk)
# Provided as-is, use at own risk.

# Name of your module: MUST match the name of the C++ containing BOOST_PYTHON_MODULE
PYTHON_LIB_NAME=DSPPythonWrapper
# Test executable target name
TARGET_NAME=main

### C++ TEST EXECUTABLE ###

# Add here any (more) source folders
SRC_DIR ?= ./src
OBJ_DIR = ./build

SRCS := $(shell find $(SRC_DIR) -name '*.s' -or -name '*.c' -or -name '*.cpp')
python: SRCS += $(shell find $(SRC_DIR) -name '$(PYTHON_LIB_NAME)*')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# Add here any (more) include folders
INC_DIRS := include src
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Change if you fancy using Clang
CC := g++
CFLAGS := -Wall -c -fPIC -MMD -MP -std=c++11 -ggdb
CInc := $(INC_FLAGS)

CLinkFlagsExec =


### PYTHON LIBRARY ###

# Name of the macro you want to use to toggle Python bindings in your code
PYTHON_EXT_MACRO=PYTHON_WRAPPER

# Change with own Python version from the "include" directory
PYTHON_VERSION := 3.8
# Change to own Python's path (system path, NOT venv)
PYTHON_INC := /usr/include/python$(PYTHON_VERSION)
# Change to location of B+= oost library (either local or system-wide)
BOOST_INC := /usr/include
BOOST_LIB_LOCATION := /usr/local/lib
# Change to names of libboost_python and libboost_numpy
BOOST_LIB_FILE := boost_python38
BOOST_NUMPY_FILE := boost_numpy38

python: CFLAGS += -D$(PYTHON_EXT_MACRO)
python: CInc += -I$(BOOST_INC) -I$(PYTHON_INC) 
python: CLinkFlags = -shared -Wl,-soname,$@ -Wl,-rpath,$(BOOST_LIB_LOCATION) -L$(BOOST_LIB_LOCATION) -l$(BOOST_LIB_FILE) -l$(BOOST_NUMPY_FILE)

PHONY: all
all: $(TARGET_NAME)

$(TARGET_NAME): $(OBJS)
	$(CC) $^ $(CLinkFlagsExec) -o $@

python: $(PYTHON_LIB_NAME).so

%.so: $(OBJS)
	$(CC) $^ $(CLinkFlags) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CInc) -c -o $@ $<

.PHONY: clean

clean:
	rm -rf ./build $(PYTHON_LIB_NAME).so $(TARGET_NAME)

-include $(DEPS)
