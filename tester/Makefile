all: test rot_test selector

selector: selector.c
	arm-linux-gnueabi-gcc selector.c -o selector

test: test.c
	arm-linux-gnueabi-gcc test.c -o test
rot_test: rot_test.c
	arm-linux-gnueabi-gcc rot_test.c -o rot_test

clean:
	rm -f *.o test rot_test selector
