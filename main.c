#include <stdio.h>
#include <stdlib.h>

#include <sys/inotify.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void sendUDP(char * addr, char * msg){
    if (!sock){
        if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            puts("socket creation failed"); 
            exit(EXIT_FAILURE); 
        }
    }

    struct sockaddr_in dstAddr;
    memset(&dstAddr, 0, sizeof(dstAddr));
    
    dstAddr.sin_family = AF_INET; 
    dstAddr.sin_port = htons(UDP_PORT); 
    dstAddr.sin_addr.s_addr = inet_addr(addr);

    if (sendto(sock, msg, strlen(msg)+1, 0, // +1 to include terminator
           (const struct sockaddr *)&dstAddr, sizeof(dstAddr)) < 0){
        puts("cannot send message");
        close(sock);
        exit(EXIT_FAILURE); 
    }
}

char* getTime(){
    const int TIME_BUFF_SIZE = 64;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[TIME_BUFF_SIZE];
    char * buff = (char *)malloc(sizeof(char) * TIME_BUFF_SIZE);
    assert(strftime(s, sizeof(s), "%c", tm));
    sprintf(buff,"%s", s);
    return buff;
}

void writeLog(char* filename, bool is_read){
    printf("name = %s\n", filename);
    if(is_read){
        printf("ACCESS: READ\n");
    }
    else{
        printf("ACCESS: WRITE\n");
    }
    char * t = getTime();
    printf("TIME OF ACCESS: %s \n", t);
    free(t);
}



static void             /* Display information from inotify_event structure */
displayInotifyEvent(struct inotify_event *i)
{   if (!i->len){
        return;
    }   
    if (i->mask & IN_ACCESS)        writeLog(i->name, true);
    if (i->mask & IN_MODIFY)        writeLog(i->name, false);
}


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
int
main(int argc, char *argv[])
{
    int inotifyFd, wd, j;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead;
    char *p;
    struct inotify_event *event;

    if (argc != 2 || strcmp(argv[1], "--help") == 0){
        printf("myFileSystemMonitor pathname\n");
        return 1;
    }

    inotifyFd = inotify_init();                 /* Create inotify instance */
    if (inotifyFd == -1){
        printf("failed at inotify_init");
        exit(1);
    }

    /* For each command-line argument, add a watch for all events */
    wd = inotify_add_watch(inotifyFd, argv[1], IN_ALL_EVENTS);
    if (wd == -1){
        printf("failed at inotify_add_watch");
        exit(1);
    }
    printf("Watching %s using wd %d\n", argv[j], wd);

    for (;;) {                                  /* Read events forever */
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0){
            printf("read() from inotify fd returned 0!");
            exit(1);
        }

        if (numRead == -1){
            puts("failed reading from inotify buffers");
            exit(1);
        }

        /* Process all of the events in buffer returned by read() */
        for (p = buf; p < buf + numRead; ) {
            event = (struct inotify_event *) p;
            displayInotifyEvent(event);

            p += sizeof(struct inotify_event) + event->len;
        }
    }

    exit(EXIT_SUCCESS);
}
