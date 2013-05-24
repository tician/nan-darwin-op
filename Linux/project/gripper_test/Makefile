###############################################################
#
# Purpose: Makefile for "gripper_test"
# Author.: robotis
# Version: 0.1
# License: GPL
#
###############################################################

TARGET = gripper_test

INCLUDE_DIRS = -I../../include -I../../../Framework/include

CXX = g++
CXXFLAGS += -O2 -pedantic -DLINUX -Wall $(INCLUDE_DIRS)
#CXXFLAGS += -O2 -DDEBUG -DLINUX -Wall $(INCLUDE_DIRS)
#CXXFLAGS += -O2 -ggdb -pedantic -DLINUX -Wall -shared $(INCLUDE_DIRS)
#CXXFLAGS += -O0 -DLINUX -Wall -shared $(INCLUDE_DIRS)
LFLAGS += -lpthread -ldl -lrt
#LFLAGS += -ggdb -lpthread -ldl -lrt

OBJECTS =   main.o

all: $(TARGET)

clean:
	rm -f *.a *.o $(TARGET) core *~ *.so *.lo

darwin.a:
	make -C ../../build

$(TARGET): darwin.a $(OBJECTS)
	$(CXX) $(CFLAGS) $(OBJECTS) ../../lib/darwin.a -o $(TARGET) $(LFLAGS)
	chmod 755 $(TARGET)

# useful to make a backup "make tgz"
tgz: clean
	mkdir -p backups
	tar czvf ./backups/gripper_test_`date +"%Y_%m_%d_%H.%M.%S"`.tgz --exclude backups *
