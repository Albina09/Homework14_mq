#include <fcntl.h> 
#include <sys/stat.h> 
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define NAME "/mq"

void errorExit(char err[]);

void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}

int main(void){
    mqd_t mqd;
    unsigned int prio;
    char buff_out[] = "Client: Hello";
    char *buff_in;
    struct mq_attr attr;

    mqd = mq_open(NAME, O_RDWR);
    if(mqd == (mqd_t) - 1)
        errorExit("mq_open");

    if(mq_send(mqd, buff_out, strlen(buff_out), 0) == -1)
        errorExit("mq_send");

    if (mq_getattr(mqd, &attr) == -1)
        errorExit("mq_getattr");

    buff_in = malloc(attr.mq_msgsize);
    if (buff_in == NULL)
        errorExit("malloc");

    if(mq_receive(mqd, buff_in, attr.mq_msgsize, NULL) == -1)
        errorExit("mq_receive");
    printf("%s\n", buff_in);

    mq_unlink(NAME);
    exit(EXIT_SUCCESS);
}