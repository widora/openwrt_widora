# build testio executable when user executes "make"

appreg: appreg.o
	$(CC) $(LDFLAGS) appreg.o -o appreg

appreg.o: appreg.c
	$(CC) $(CFLAGS) -c appreg.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o appreg
