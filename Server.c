    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <pthread.h>
    #include <malloc.h>
    #include <string.h>
    #include <unistd.h>
    #define MAX_PLAYERS 50

    typedef struct players{
        char name[50];
        char stato[20];
        int socket;

    }player;

    player players[MAX_PLAYERS];
    int i=0;
    pthread_mutex_t indexMutex; // Semaforo anche per la struttura condivisa?

    void* comunicationHandler(void *);
    int isConnected(char name[]);

    int main(){
        int sd,c,fd,*newsock;
        struct sockaddr_in from,server;
        sd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        server.sin_family=AF_INET;
        server.sin_addr.s_addr=INADDR_ANY;
        server.sin_port=htons(8888);
        bind(sd,(struct sockaddr*)&server,sizeof(server));
        listen(sd,3);
        c=sizeof(struct sockaddr_in);
        while(fd=accept(sd,(struct sockaddr*)&from,(socklen_t*)&c)){
            pthread_t dataThread;
            newsock=malloc(4);
            *newsock=fd;
            if(pthread_create(&dataThread, NULL, comunicationHandler, (void*)newsock)<0){
                puts("Errore nella creazione del thread");
                return 1;
            }
            puts("Thread assegnato");


        }
        if(fd<0){
            puts("Errore nella accept");
            return 1;
        }


        return 0;
    }

    void* comunicationHandler(void *socket){

        int current,x,move,matchFinished;
        int sock=*(int*)socket;
        char msg[1024];
        if(i>15){
            strcpy(msg,"Server Pieno.");
            write(sock,msg,strlen(msg));
            close(sock);
            pthread_exit(NULL);
        }
        pthread_mutex_lock(&indexMutex);
        i++;
        current=i;
        pthread_mutex_unlock(&indexMutex);
        read(sock,msg,1024);
        strcpy(players[current].name,msg);
        strcpy(players[current].stato,"Connected");
        players[current].socket=sock;

        while(recv(sock,msg,1024,0)>0){

            if(strcmp(msg,"!who")==0){
                write(sock,&i,sizeof(i));

                for(int c=0;c<i;c++)
                    write(sock,strlen(players[c].name),sizeof(int));

                for(int c=1;c<=i;c++)
                    write(sock,players[c].name,strlen(players[c].name)); //Manda i nomi senza andare a capo

            }

            if(strcmp(msg,"!create")==0){
                strcpy(players[current].stato,"Waiting");
                pthread_exit(0);
            }

            if(strcmp(msg,"!join")==0){ // x=indice giocatore che ha fatto la create, current=indice del giocatore che ha fatto la join
                memset(msg,0,strlen(msg));
                read(sock,msg,1024);

                if((x=isConnected(msg))>0){
                    write(players[x].socket,players[current].name,1024); //Risveglio il client che aveva fatto la create, mandandogli il nome dell'avversario
                    write(sock,"Connessione avvenuta.",1024);
                    for(;;){
                        read(players[x].socket,&move,sizeof(int)); //Leggo dal client che aveva fatto la create
                        read(players[x].socket,&matchFinished,sizeof(int));

                        if(matchFinished){
                            write(players[current].socket,&move,sizeof(int));  //Scrivo al client che aveva fatto la join
                            break;
                        }

                        write(players[current].socket,&move,sizeof(int));  //Scrivo al client che aveva fatto la join
                        read(players[current].socket,&move,sizeof(int));
                        read(players[current].socket,&matchFinished,sizeof(int));

                        if(matchFinished){
                            write(players[x].socket,&move,sizeof(int));
                            break;
                        }

                        write(players[x].socket,&move,sizeof(int));

                    }
                    puts("\nLa partita e' terminata\n");


                }
                else
                    write(sock,"Giocatore non presente o occupato",1024);
            }

        }

        puts("\nClient Disconnected");
        pthread_mutex_lock(&indexMutex);
        players[current]=players[i-1];
        i--;
        pthread_mutex_unlock(&indexMutex);
        close(sock);



    }

    int isConnected(char name[]){
        for(int c=1;c<=i;c++){
            if(strcmp(name,players[c].name)==0 && strcmp(players[c].stato,"Waiting")==0)
                return c;
        }
        return -1;
    }


