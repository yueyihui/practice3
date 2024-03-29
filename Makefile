obj-m+=banker.o

all: practice3 testbanker_stateA testbanker_stateB
	make -C /lib/modules/$(shell uname -r)/build/ DBGFLAGS=-DDBG M=$(PWD) modules

practice3: practice3.c
	gcc -Wall -g3 -o $@ $^

testbanker_stateA: testbanker_stateA.c
	gcc -Wall -g3 -o $@ $^

testbanker_stateB: testbanker_stateB.c
	gcc -Wall -g3 -o $@ $^

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	find -type f -executable -delete
	find -type f -name *.o -delete
