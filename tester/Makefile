all: test rot_test

test: test.c
	arm-linux-gnueabi-gcc test.c -o test
rot_test: rot_test.c
	arm-linux-gnueabi-gcc rot_test.c -o rot_test

clean:
	rm -f test test.o rot_test rot_test.o
