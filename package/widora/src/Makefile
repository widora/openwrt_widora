CC = gcc
CFLAGS = -Wall
OBJS = pwm_test.o

all:pwm_test

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)
clean:
	rm -f *.o pwm_test
