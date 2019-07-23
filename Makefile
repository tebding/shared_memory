#------------------------------------------------------------------------------

FLAGS   = -std=gnu99 -Wall
SOURCES = shared_memory.c #test_shared_memory.c
OBJECTS = shared_memory.o #test_shared_memory.o
#HEADERS = shared_memory.h
EXEBIN  = shared_memory

all: $(EXEBIN)

$(EXEBIN) : $(OBJECTS) #$(HEADERS)
	gcc -ggdb -o $(EXEBIN) $(OBJECTS) -lrt

$(OBJECTS) : $(SOURCES) #$(HEADERS)
	gcc -ggdb -c $(FLAGS) $(SOURCES)

clean :
	rm -f $(EXEBIN) $(OBJECTS)

check:
	valgrind --leak-check=full $(EXEBIN)
