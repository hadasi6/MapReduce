depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)
tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

CC      = g++
AR      = ar
RANLIB  = ranlib

SRC_DIR = src
EXAMPLES_DIR = examples

LIBSRC  = $(SRC_DIR)/MapReduceFramework.cpp $(SRC_DIR)/Barrier.cpp
LIBOBJ  = $(LIBSRC:.cpp=.o)
INCS    = -I$(SRC_DIR)
CFLAGS  = -Wall -std=c++11 -pthread $(INCS)

LIBNAME = libMapReduceFramework.a
TARGETS = $(LIBNAME)

.PHONY: all clean example

all: $(TARGETS)

$(LIBNAME): $(LIBOBJ)
	$(AR) rcs $@ $^
	$(RANLIB) $@

example: all
	$(CC) $(CFLAGS) -o sample_client $(EXAMPLES_DIR)/SampleClient.cpp -L. -lMapReduceFramework

clean:
	rm -f $(TARGETS) $(LIBOBJ) sample_client *~ *core