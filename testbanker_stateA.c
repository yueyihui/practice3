/**
 * @file   testebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the ebbchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/ebbchar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define  STATE_A    0 //allocation=(1,4,5) max=(4,4,8)
#define  STATE_B    1 //allocation=(1,4,6) max=(4,6,8)

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        perror("must be state PROCESS_A or PROCESS_B or PROCESS_C");
        return -1;
    }

    int ret, fd;
    fd = open("/dev/bankeralgorithm", O_RDWR);             // Open the device with read/write access
    if (fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    int identify;//process id for kernel module
    char request;
    char initReq;
    unsigned int need;

    if (!strcmp(argv[1], "PROCESS_A")) {
        need = 6;
        identify = 0;
        request = 3;
        initReq = 1;
    } else if (!strcmp(argv[1], "PROCESS_B")) {
        need = 4;
        identify = 1;
        request = 0;
        initReq = 4;
    } else if (!strcmp(argv[1], "PROCESS_C")) {
        need = 8;
        identify = 2;
        request = 3;
        initReq = 5;
    } else {
        perror("must be parameter PROCESS_A or PROCESS_B or PROCESS_C");
        return -1;
    }

    ioctl(fd, need, identify);//tell the driver how am i

    read(fd, &initReq, 1); // get resource

    sleep(1); //wait for three process are achieves State A

    read(fd, &request, 1); // get resource

    write(fd, NULL, 0);// release resource
    
    return 0;
}