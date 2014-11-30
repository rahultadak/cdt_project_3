CC = g++
OPT = -O0
MEM = -m32
DBG = -g
WARN = -w
CFLAGS = $(OPT) $(MEM) $(WARN) $(INC) $(LIB) $(DBG)

# List all your .cc files here (source files, excluding header files)
SIM_SRC = main.cc classes.cc

# List corresponding compiled object files here (.o files)
SIM_OBJ = main.o classes.o
 
#################################

# default rule

all: sim
	@echo "my work is done here..."


# rule for making sim

sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM-----------"


# generic rule for converting any .cc file to any .o file
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc


# type "make clean" to remove all .o files plus the sim binary

clean:
	rm -f *.o sim

debug:
	$(CC) $(MEM) $(DBG) $(SIM_SRC)
	@echo "built for debug"
