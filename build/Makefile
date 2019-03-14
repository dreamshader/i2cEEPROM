SHELL := /bin/bash
#################################################################################
# Makefile for i2cEEPROM - a library for transparent access to I2C EEPROMs
#
# benoetigt sudo apt-get install libi2c.dev
#################################################################################
# This Makefile is part of my i2cEEPROM project
#	https://github.com/dreamshader/i2cEEPROM
#
#  Copyright (C) 2019 Dreamshader (aka Dirk Schanz)
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#################################################################################
CXX = g++
CC  = gcc
AR  = ar
RANLIB  = ranlib
LDCONFIG?=ldconfig
#
CCDEBUG  = -g
CXXDEBUG = -g
#
CXXFLAGS = -Wall
CXXLIBSOFLAGS = -fPIC -shared 
#
# CXXEXTRAFLAGS = -DLINUX
# CXXEXTRAFLAGS = -DLINUX -DDEBUG -DDEBUG_STATUS_BITS
# CXXEXTRAFLAGS = -DRASPBERRY
# CXXEXTRAFLAGS = -DRASPBERRY -DDEBUG
#
SOURCEDIR = ../src
#
SOLIBNAME = libi2cEEPROM.so
STATLIBNAME = libi2cEEPROM.a
#
LIB_SRC = $(SOURCEDIR)/i2cCore.cpp $(SOURCEDIR)/i2cEEPROM.cpp
LIB_INC = $(SOURCEDIR)/i2cCore.h $(SOURCEDIR)/i2cEEPROM.h
LIB_OBJ = i2cCore.o i2cEEPROM.o

EXAMPLE_SRC = $(SOURCEDIR)/eeTestrun.cpp
EXAMPLE_NAME = eeTestrun

INIT_SRC = $(SOURCEDIR)/eeInit.cpp
INIT_NAME = eeInit

BUILD_FLAGS = -I. -L ../build
#
#
EXTRALIBS = -lrt -lpthread
CXXEXTRAFLAGS = -DLINUX -DDEBUG -DDEBUG_STATUS_BITS -DRASPBERRY
#

#
all: $(STATLIBNAME) $(SOLIBNAME) $(EXAMPLE_NAME) $(INIT_NAME)


#$(LIB_SRC) $(LIB_INC)
$(STATLIBNAME):	$(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) -c $(LIB_SRC)
	$(AR) rcs $(STATLIBNAME)  $(LIB_OBJ)
	$(RANLIB) $(STATLIBNAME)

#$(LIB_SRC) $(LIB_INC)
$(SOLIBNAME):	$(LIB_OBJ)
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) $(CXXDEBUG) $(CXXLIBSOFLAGS) -c $(LIB_SRC)
	$(CXX) -shared  -Wl,-soname,$(SOLIBNAME) -o $(SOLIBNAME) $(LIB_OBJ)

$(LIB_OBJ):
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) -c $(LIB_SRC)

$(EXAMPLE_NAME): $(EXAMPLE_SRC) $(STATLIBNAME) $(SOLIBNAME)
	$(CXX) -o $(EXAMPLE_NAME) $(CXXDEBUG) $(CXXEXTRAFLAGS) $(EXAMPLE_SRC) $(SOLIBNAME) $(BUILD_FLAGS) ${EXTRALIBS}

$(INIT_NAME): $(INIT_SRC) $(STATLIBNAME) $(SOLIBNAME)
	$(CXX) -o $(INIT_NAME) $(CXXDEBUG) $(CXXEXTRAFLAGS) $(INIT_SRC) $(SOLIBNAME) $(BUILD_FLAGS) ${EXTRALIBS}




install: $(STATLIBNAME) $(SOLIBNAME)
	sudo install -m 0755 -d                        /usr/local/include
	sudo install -m 0644 $(SOURCEDIR)/i2cCore.h    /usr/local/include
	sudo install -m 0644 $(SOURCEDIR)/i2cEEPROM.h  /usr/local/include
	sudo install -m 0755 -d                        /usr/local/lib
	sudo install -m 0644 libi2cEEPROM.a            /usr/local/lib
	sudo install -m 0644 libi2cEEPROM.so           /usr/local/lib
	$(LDCONFIG)

uninstall:
	sudo rm -f /usr/local/include/i2cCore.h
	sudo rm -f /usr/local/include/i2cEEPROM.h
	sudo rm -f /usr/local/lib/libi2cEEPROM.a
	sudo rm -f /usr/local/lib/libi2cEEPROM.so
	$(LDCONFIG)

clean:
	$rm *.o *.so *.a

