###############################################################
#
# Purpose: Makefile for "TellDarwin"
# Author.: robotis (modded by NaN)
# Version: 0.1
# License: GPL
#
###############################################################

TARGET = TellDarwin

INCLUDE_DIRS = -I../../include -I../../../Framework/include -I./UGA -I../../Linux/include

CXX = g++
#CXXFLAGS += -DLINUX -g -Wall $(INCLUDE_DIRS)
CXXFLAGS += -DLINUX -Wall $(INCLUDE_DIRS)
LFLAGS += -lpthread -lrt -lasound

GARBAGE = ./*~ \
	./src/Interpreter/*~ \
	./src/Communication/*~ \
	./src/Communication/Linux/*~ \
	./src/RequestListeners/*~ \
	./src/RequestListeners/MAX/*~ \
	./src/RequestListeners/Netcat/*~ \
	./src/RequestListeners/StdIO/*~ \
	./src/MediaStreamers/*~ \
	./src/MediaStreamers/Audio/*~ \
	./src/MediaStreamers/Audio/Linux/*~ \
	./src/MediaStreamers/JitterStreamers/*~ \
	
OBJECTS = main.o \
	src/Interpreter/Interpreter.o \
	src/Interpreter/Performer.o \
        src/MediaStreamers/Audio/Linux/Audio.o \
	src/MediaStreamers/JitterStreamers/JitterTypes.o \
	src/MediaStreamers/JitterStreamers/JitterCameraStreamer.o \
	src/MediaStreamers/JitterStreamers/JitterMicrophoneStreamer.o \
	src/MediaStreamers/JitterStreamers/JitterSpeakerStreamer.o \
        src/MediaStreamers/JitterStreamers/JitterLineInStreamer.o \
	src/RequestListeners/MAX/MAXTypes.o \
	src/RequestListeners/MAX/MAXRequestListener.o \
	src/RequestListeners/Netcat/NetcatRequestListener.o \
	src/RequestListeners/StdIO/StdIORequestListener.o \
	src/Communication/Linux/Communication.o \
	src/Communication/Linux/TCPCommunication.o \
	src/Communication/Linux/UDPCommunication.o \

all: darwin.a $(TARGET)

libclean:
	make -C ../../build clean

distclean: clean libclean

darwin.a:
	make -C ../../build

$(TARGET): $(OBJECTS) ../../lib/darwin.a
	$(CXX) $(CFLAGS) $(OBJECTS) ../../lib/darwin.a -o $(TARGET) $(LFLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET) $(GARBAGE)

