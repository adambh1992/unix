#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */

int main(int argc, char *argv[]){
    char * str = "Hello World";
    if (argc < 3){
        printf("error!\n");
        return 1;
    }
    int fd = open(argv[2], O_RDWR);
    if (strcmp(argv[1], "WRITE") == 0){
        int count = write(fd, str, strlen(str));
        printf("wrote %d bytes to %s\n", count, argv[2]);
    }
    if (strcmp(argv[1], "READ") == 0){
        char buff[100] = {0};
        int count = read(fd, buff, 100);
        puts("read!\n");
        // fprintf("read %d bytes from %s: %s\n",count, argv[2], buff);
    }
    close(fd);
}
