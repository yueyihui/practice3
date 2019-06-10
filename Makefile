ccflags-y := -std=gnu11 -Wno-declaration-after-statement

obj-m+=ebbchar.o

all: practice5 testebbchar
	make -C /lib/modules/$(shell uname -r)/build/ DBGFLAGS=-DDBG M=$(PWD) modules

practice5: practice5.c
	gcc -Wall -g3 -o $@ $^

testebbchar: testebbchar.c
	gcc -Wall -g3 -o $@ $^

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	find -type f -executable -delete
	find -type f -name *.o -delete
