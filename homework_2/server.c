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
#define MAX_CLIENTS 20
#define MAX_SIZE_NAME 20

struct users{
    char name[MAX_SIZE_NAME];
    mqd_t mqd;
};
struct message{
    char user[21];
    char user_send[20];
    char msg[70];
};
struct users user[MAX_CLIENTS];
struct message messeg;
char buff[MAX_BUFFER_SIZE];
mqd_t mqdd, mqd, msg_mqd;

void nam(char buff[]);
void errorExit(char err[]);

void errorExit(char err[]){
    perror(err);
    exit(EXIT_FAILURE);
}

void nam(char buff[]){
    int q = 0;
    printf("%s\n", buff);

    for(int i = 0; i < MAX_CLIENTS; i++){

        if(!user[i].mqd && !q){
            strcpy(user[i].name, buff);                              //копирует все имена в структуру с именами очередей
            
            user[i].mqd = mq_open(user[i].name, O_RDWR);            //открывает очереди, клиентов
            if(user[i].mqd == (mqd_t) - 1)
                errorExit("mq_open");
                
            q = 1;
        }
    }
}

void *client_name(void *args){ 

    while(1){

        if(mq_receive(mqdd, buff, MAX_BUFFER_SIZE , NULL) == -1)  //принимает имена новой очереди
            errorExit("mq_receive");
            
        nam(buff);
    
        memset(buff,0,strlen(buff));
    }
}

void *server(void *args){
   
    while(1){
        
        if(mq_receive(msg_mqd, (char*) &messeg, MAX_BUFFER_SIZE  , NULL) == -1)          //принимает сообщения с именами клиентов  /*принимает структуру*/
            errorExit("mq_receive11");
           
        
        if(strcmp(messeg.msg, "exit\n") == 0){

            for(int i = 0; i < MAX_CLIENTS; i++){

                if(strlen(user[i].name) && !strcmp(user[i].name,messeg.user)){
                    mq_unlink(user[i].name);
                    memset(user[i].name, 0, strlen(user[i].name));
                }
            }
        }else{
            printf("----------------------\n");

            for(int i = 0; i < MAX_CLIENTS; i++){

                if(strlen(user[i].name) != 0){
                    printf("%s\n", user[i].name);

                    if(mq_send(user[i].mqd, (char*)&messeg, sizeof(messeg) + 1, 0) == -1)       //отпраляет сообщения всем клиентам  /*отправяет структуру*/
                        errorExit("mq_send11");
                        
                }
            }
        }
    }   
}

int main(void){
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_curmsgs = 0;
    pthread_t thread[2];
    char ex[9];

    printf("Для выхода из сервера напишите: exit\n");

    mqdd = mq_open(NAMEE, O_RDWR | O_CREAT, QUEUE, &attr);
    if(mqdd == (mqd_t) - 1)
        errorExit("mq_open");
        
    msg_mqd = mq_open(NAME, O_RDWR | O_CREAT, QUEUE, &attr);
    if(msg_mqd == (mqd_t) - 1)
        errorExit("mq_open");
        

    if(pthread_create(&thread[0], NULL, client_name, NULL) == -1)
        errorExit("pthread_create");
    
    if(pthread_create(&thread[1], NULL, server, &mqd) == -1)
        errorExit("pthread_create");

    while(1){
        struct message del;
        memset(del.msg, 0, sizeof(del.msg));
        fgets(del.msg, 5,stdin);

        if(strcmp(del.msg, "exit") == 0){

            for(int i = 0; i < 2; i++){
                pthread_cancel(thread[i]);
            }

            for(int i = 0; i < MAX_CLIENTS; i++){

                if(strlen(user[i].name) != 0){

                    if(mq_send(user[i].mqd, (char*)&del, sizeof(del) + 1,0) == -1)       //отпраляет сообщения всем клиентам 
                        errorExit("mq_send11");
                        
                    mq_unlink(user[i].name);
                }
            }
            
            mq_unlink(NAME);
            mq_unlink(NAMEE);

            exit(EXIT_SUCCESS); 
        }   
    }
}