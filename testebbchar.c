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

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main(int argc, const char *argv[])
{
    if (argc < 3) {
        perror("must be state STATE_A or STATE_B");
        return -1;
    }

    int ret, fd;
    char stringToSend[BUFFER_LENGTH];
    fd = open("/dev/ebbchar", O_RDWR);             // Open the device with read/write access
    if (fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    int state;
    if (!strcmp(argv[1], "STATE_A"))
    {
        state = 0;
    }
    else if (!strcmp(argv[1], "STATE_B"))
    {
        state = 1;
    }
    else
    {
        perror("must be state STATE_A or STATE_B");
        return -1;
    }

    int identify;
    if (!strcmp(argv[2], "PROCESS_A")) {
        identify = 0;
    } else if (!strcmp(argv[2], "PROCESS_B")) {
        identify = 1;
    } else if (!strcmp(argv[2], "PROCESS_C")) {
        identify = 2;
    } else {
        perror("must be parameter PROCESS_A or PROCESS_B or PROCESS_C");
        return -1;
    }

    ioctl(fd, state, identify);
    //ret = write(fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
    //if (ret < 0){
    //    perror("Failed to write the message to the device.");
    //    return errno;
    //}

    printf("Press ENTER to read back from the device...\n");
    getchar();

    ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
    if (ret < 0) {
        perror("Failed to read the message from the device.");
        return errno;
    }
    close(fd);
    return 0;
}
