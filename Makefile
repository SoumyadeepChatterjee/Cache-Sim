CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
NUM = -std=c++11
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB) $(NUM)

# List all your .cc/.cpp files here (source files, excluding header files)
SIM_SRC = sim.cc sim_definitions.cc

# List corresponding compiled object files here (.o files)
SIM_OBJ = sim.o sim_definitions.o
 
#################################

# default rule

all: sim
	@echo "my work is done here..."


# rule for making sim

sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH sim-----------"


# generic rule for converting any .cc file to any .o file
 
.cc.o:
	$(CC) $(CFLAGS) -c $*.cc

# generic rule for converting any .cpp file to any .o file

.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp


# type "make clean" to remove all .o files plus the sim binary

clean:
	rm -f *.o sim


# type "make clobber" to remove all .o files (leaves sim binary)

clobber:
	rm -f *.o


