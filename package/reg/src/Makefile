# build testio executable when user executes "make"

appreg: reg.o
	$(CC) $(LDFLAGS) reg.o -o reg

appreg.o: reg.c
	$(CC) $(CFLAGS) -c reg.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o appreg
