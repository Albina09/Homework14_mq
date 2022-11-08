#include <fcntl.h> 
#include <sys/stat.h> 
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>


#define NAMEE "/aq"
#define NAME "/mq"

#define QUEUE 0666
#define MAX_SIZE 256
#define MAX_MESSAGES 10
#define MAX_BUFFER_SIZE 266
#define MAX_SIZE_NAME 20

char buff[MAX_BUFFER_SIZE];

struct message{
    char user[21];
    char user_send[20];
    char msg[70];
};
struct message messag_send;
struct mq_attr attr;
mqd_t mqdw;

void name(struct mq_attr attr);
void errorExit(char err[]);

void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}
  
void *send(void *args){
    
    mqd_t mqd = mq_open(NAME, O_WRONLY);
    if(mqd == (mqd_t) - 1)
        errorExit("mq_open");
        
    while(1){

        fgets(messag_send.msg, sizeof(messag_send.msg), stdin);                              
        
        if(strcmp(messag_send.msg, "exit\n") == 0){

            if(mq_send(mqd, (char*)&messag_send, sizeof(messag_send), 0) == -1)             
                errorExit("mq_send");
            pthread_exit(send);

        }else{
            if(mq_send(mqd, (char*)&messag_send, sizeof(messag_send), 0) == -1)          
                errorExit("mq_send");
        }
    }

}
void *receive(void *args){
    int b = 1;

    mqd_t mqd = mq_open(messag_send.user, O_RDONLY);                             
    if(mqd == (mqd_t) - 1)
        errorExit("mq_open");
        
    while(b){
        struct message messag_rece;
        
        if(mq_receive(mqd, (char*)&messag_rece, MAX_BUFFER_SIZE , NULL) == -1)   
            errorExit("mq_receive");
           
        if(strcmp(messag_rece.msg, "exit\n") == 0)
            printf("Сервер отключён\n");

        else
            printf("%s: %s\n",messag_rece.user_send,messag_rece.msg);                             
        
    }
}
void name(struct mq_attr attr){

    messag_send.user[0] = '/';

    printf("Введите имя:");
    fgets(messag_send.user_send, MAX_SIZE_NAME,stdin);                           
    
    for(int i = 0; i< MAX_SIZE_NAME; i++){

        if(messag_send.user_send[i] == '\n')
            messag_send.user_send[i] = '\0';
    }
    strcat(messag_send.user, messag_send.user_send);

    
    mqd_t mqd = mq_open(NAMEE, O_WRONLY);                         
    if(mqd == (mqd_t) - 1)
        errorExit("mq_open");
        
    if(mq_send(mqd,messag_send.user, strlen(messag_send.user) + 1, 0) == -1)  
        errorExit("mq_send");
        
    mq_close(mqd);
}
int main(void){
    
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_maxmsg = MAX_MESSAGES;
    pthread_t thread[2]; 
    
    name(attr);
    printf("Для выхода из чата напишите: exit\n"); 

    mqdw = mq_open(messag_send.user, O_RDWR | O_CREAT, QUEUE, &attr);
    if(mqdw == (mqd_t) - 1)
        errorExit("mq_open");
       
    if(pthread_create(&thread[0], NULL, send, NULL) == -1)
        errorExit("pthread_create");

    if(pthread_create(&thread[1], NULL, receive, NULL) == -1)
        errorExit("pthread_cteate");
    
    pthread_join(thread[0], NULL);
    pthread_cancel(thread[1]);
    
    exit(EXIT_SUCCESS);
}