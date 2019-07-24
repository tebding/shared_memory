#------------------------------------------------------------------------------

FLAGS   = -std=c99 -Wall
SOURCES = shared_memory.c 
OBJECTS = shared_memory.o 
#HEADERS = shared_memory.h
EXEBIN  = shared_memory

all: $(EXEBIN)

$(EXEBIN) : $(OBJECTS) #$(HEADERS)
	gcc -ggdb -o $(EXEBIN) $(OBJECTS) -lrt

$(OBJECTS) : $(SOURCES) #$(HEADERS)
	gcc -ggdb -c $(FLAGS) -D_XOPEN_SOURCE=500 $(SOURCES)

clean :
	rm -f $(EXEBIN) $(OBJECTS)

check:
	valgrind --leak-check=full $(EXEBIN)
