all:
	gcc -Wall -std=c99 -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s main.c -o tmp-3

clean:
	rm -f *.o tmp-3
