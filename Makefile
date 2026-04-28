CC ?= gcc
MPICC ?= mpicc
CFLAGS ?= -std=c11 -O3

# Sources (allow for either underscored or spaced main filenames)
MAIN_NEARLY_UNDERSCORE = main_nearly.c
MAIN_NEARLY_SPACE = "main nearly.c"
MAIN_MPI = main_mpi.c

HEAT = heat.c
READER = file_reader.c

.PHONY: all clean gccnearly gcccomplete

all: gccnearly

# build nearlyserial with gcc
gccnearly:
	$(CC) $(CFLAGS) $(READER) $(HEAT) $(MAIN_NEARLY_UNDERSCORE) -o "heat_nearly"
	@if [ -f "heat_nearly" ]; then echo "Built: heat nearly"; else echo "Build failed"; exit 1; fi 
# for error checking

# build complete MPI version with mpicc
gcccomplete:
	$(MPICC) $(CFLAGS) $(READER) $(HEAT) $(MAIN_MPI) -o "heat_complete"
	@if [ -f "heat_complete" ]; then echo "Built: heat complete"; else echo "Build failed (check mpicc and main_mpi.c)"; exit 1; fi

# clean up the files made 
clean:
	rm -f "heat_nearly" "heat_complete" program *.o
