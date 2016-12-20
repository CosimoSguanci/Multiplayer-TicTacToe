
    #include <stdio.h>
    #include <stdlib.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <string.h>
    #include <unistd.h>

    int checkResult(char map[],char simbol, char simbol_vs);

    int main(){
        char addr[50], buf[10], name[50], msg[1024], simbolo, simbolo_vs;

        char map[10];

        int sd, i, port, c, len[100], x, cellNum, nameLen, move, result, shouldExit = 0, shouldRead = 1;

        struct sockaddr_in to;

        map[1] = '\0';
        map[2] = '\0';
        map[3] = '\0';
        map[4] = '\0';
        map[5] = '\0';
        map[6] = '\0';
        map[7] = '\0';
        map[8] = '\0';
        map[9] = '\0';

        printf("TIC TAC TOE Client_Cosimo Sguanci\n\n");

        //La chiamata gethostbyname è superflua dal momento che chiediamo l'IP del server all'utente.

        printf("Inserisci le informazioni relative al Server\n");
        printf("Indirizzo IP: ");
        scanf("%s",addr);
        printf("Porta: ");
        scanf("%d",&port);
        /*strcpy(addr,"127.0.0.1");
        port = 8888;*/

        if((sd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0){
            puts("Errore nella creazione del socket.");
            return 1;
        }

        to.sin_family = AF_INET;
        to.sin_addr.s_addr = inet_addr(addr);
        to.sin_port = htons(port);

        if(connect(sd,(struct sockaddr*)&to,sizeof(to)) < 0){
            puts("Errore nella connessione con il Server.");
            return 1;
        }

        printf("\nConnesso al server %s sulla porta %d\n",addr,port);
        printf("\nInserisci il tuo username: ");
        scanf("%s",buf);
        write(sd,buf,1024);

        for(;;){

            if(shouldExit)
                break;
            scanf("%s",buf);


            if(strcmp(buf,"!quit") == 0)
                break;

            if(strcmp(buf,"!help") == 0){
                printf("Sono disponibili i seguenti comandi:\n");
                printf("* !help --> mostra l'elenco dei comandi disponibili\n");
                printf("* !who --> mostra l'elenco dei client connessi al server\n");
                printf("* !create --> crea una nuova partita e attendi un avversario\n");
                printf("* !join --> unisciti ad una partita e inizia a giocare\n");
                printf("* !disconnect --> disconnetti il client dall'attuale partita (solo in partita) \n");
                printf("* !quit --> disconnetti il client dal server\n");
                printf("* !show_map --> mostra la mappa di gioco (solo in partita) \n");
                printf("* !hit <num_cell> --> marca la casella <num_cell> (solo in partita) \n");
                continue;
            }

            if(strcmp(buf,"!who") == 0){
                write(sd,buf,10);
                read(sd,&c,sizeof(c));
                i = 0;
                x = 0;

                while(x < c && read(sd,len[x],sizeof(int)) > 0) // &len?
                    x++;

                while(i < c && (read(sd,name,len[i])) > 0)
                {
                         printf("%s\n",name);
                         i++;

                }
                continue;
            }

            if(strcmp(buf,"!create") == 0){
                    write(sd,buf,10);
                    puts("In attesa di uno sfidante...");
                    read(sd,buf,1024); // Aspetto di essere risvegliato
                    printf("\n%s si e' unito alla partita.",buf);
                    printf("\nIl tuo simbolo e' X. Sta a te.\n");
                    simbolo = 'X';
                    simbolo_vs = 'O';

                    for(;;){
                        if(shouldExit)
                            break;
                        result = checkResult(map,simbolo,simbolo_vs);
                        switch(result){

                                case 0: //La partita non è finita
                                    scanf("%s",buf);
                                    if(strcmp(buf, "!hit") == 0){
                                        for(;;){
                                        printf("Cella: ");
                                        scanf("%d",&cellNum);
                                        if(cellNum > 0 && cellNum <= 9){
                                            if(map[cellNum] == '\0'){
                                                map[cellNum] = simbolo;
                                                write(sd,&cellNum,sizeof(int));
                                                if((result = checkResult(map,simbolo,simbolo_vs))!=0)
                                                {
                                                    shouldExit = 1;
                                                    write(sd,&shouldExit,sizeof(int));
                                                    switch(result){
                                                        case 1:
                                                            printf("\nVittoria\n");
                                                            break;
                                                        case 2:
                                                            printf("\nSconfitta\n");
                                                            break;
                                                        case 3:
                                                            printf("\nPareggio\n");
                                                            break;

                                                        }
                                                    break;
                                                }
                                                else
                                                {
                                                    write(sd,&shouldExit,sizeof(int));
                                                	printf("\nSta al tuo avversario.\n");
                                                	read(sd,&cellNum,sizeof(int));
                                                	if(cellNum == -1)
                                                    {
                                                        printf("\nAvversario disconnesso\n");
                                                        goto END;
                                                    }
                                                    map[cellNum] = simbolo_vs;
                                                    printf("\nIl tuo avversario ha colpito la casella numero %d\n", cellNum);
                                                	break;
                                                }
                                            }
                                            else
                                                printf("\nCella occupata\n");

                                        }
                                        else
                                        	printf("\nInserisci un numero di cella valido (da 0 a 9)\n");

                                        break;
                                        }
                                    }

                                    if(strcmp(buf,"!disconnect") == 0){ //Non funziona
                                        printf("\nDisconnessione avvenuta con successo, ti sei arreso.\n");
                                        shouldExit = 1;
                                        int ex = -1;
                                        write(sd,&ex,sizeof(int));
                                        break;
                                    }

                                    if(strcmp(buf,"!show_map") == 0){
                                        printf("\n\n");
                                        printf(" %c | %c | %c\n",   map[7], map[8], map[9]);
                                        printf("----------\n");
                                        printf(" %c | %c | %c\n",   map[4], map[5], map[6]);
                                        printf("----------\n");
                                        printf(" %c | %c | %c\n\n", map[1], map[2], map[3]);
                                    }

                                    break;



                                 case 1:
                                    printf("\nHai vinto\n");
                                    shouldExit = 1;
                                    write(sd,&shouldExit,sizeof(int));
                                    break;
                                 case 2:
                                    printf("\nHai perso\n");
                                    shouldExit = 1;
                                    write(sd,&shouldExit,sizeof(int));
                                    break;
                                case 3:
                                    printf("\nPareggio\n");
                                    shouldExit = 1;
                                    write(sd,&shouldExit,sizeof(int));
                                    break;
                        }



                    }



            }

            if(strcmp(buf,"!join") == 0){
                    write(sd,buf,10);
                    printf("Avversario: ");
                    scanf("%s",name);
                    write(sd,name,strlen(name));
                    read(sd,msg,1024); // Leggo dal server se il giocatore cercato è stato trovato
                    if(strcmp(msg,"Giocatore non presente o occupato") == 0){
                        printf("%s\n",msg);
                        continue;
                    }
                    else{
                        printf("\nConnessione avvenuta con successo.\n");
                        printf("\nLa partita e' iniziata.");
                        printf("\nIl tuo simbolo e' O.\n");
                        printf("Sta al tuo avversario\n");
                        simbolo = 'O';
                        simbolo_vs = 'X';
                        for(;;){
                            if(shouldExit)
                                break;
                                if(shouldRead){
                                    read(sd,&move,sizeof(int));
                                    if(move == -1)
                                    {
                                        printf("\nAvversario disconnesso\n");
                                        goto END;
                                    }
                                    map[move] = simbolo_vs;
                                    printf("Il tuo avversario ha colpito la casella %d", move);}
                                    result = checkResult(map,simbolo,simbolo_vs);
                                    switch(result){
                                        case 0:
                                            LOOP : printf("\nSta a te\n"); //GOTO label
                                            scanf("%s",buf);

                                            if(strcmp(buf, "!show_map") == 0){
                                            printf("\n\n");
                                            printf(" %c | %c | %c\n",   map[7], map[8], map[9]);
                                            printf("----------\n");
                                            printf(" %c | %c | %c\n",   map[4], map[5], map[6]);
                                            printf("----------\n");
                                            printf(" %c | %c | %c\n\n", map[1], map[2], map[3]);
                                            shouldRead = 0;
                                            break;
                                            }

                                            if(strcmp(buf, "!disconnect") == 0){
                                                printf("\nDisconnessione avvenuta con successo, ti sei arreso.\n");
                                                shouldExit = 1;
                                                int ex = -1;
                                                write(sd,&ex,sizeof(int));
                                                break;
                                            }

                                            if(strcmp(buf,"!hit") == 0){

                                                for(;;){
                                                    shouldRead = 1;
                                                    printf("Cella: ");
                                                    scanf("%d",&cellNum);
                                                    if(cellNum > 0 && cellNum <= 9){
                                                        if(map[cellNum] == '\0'){
                                                            map[cellNum] = simbolo;
                                                            write(sd, &cellNum, sizeof(int));
                                                            if((result = checkResult(map,simbolo,simbolo_vs)) != 0)
                                                            {
                                                                shouldExit = 1;
                                                                write(sd,&shouldExit,sizeof(int));
                                                                switch(result){
                                                                    case 1:
                                                                        printf("\nVittoria\n");
                                                                        break;
                                                                    case 2:
                                                                        printf("\nSconfitta");
                                                                        break;
                                                                    case 3:
                                                                        printf("\nPareggio");
                                                                        break;
                                                                }
                                                                break;
                                                            }
                                                            else {
                                                                write(sd,&shouldExit,sizeof(int));
                                                                printf("\nSta al tuo avversario.\n");
                                                                break;
                                                            }

                                                        }
                                                        else {
                                                            printf("\nCella occupata\n");
                                                            goto LOOP;
                                                        }

                                                }
                                                else {
                                                    printf("\nInserisci un numero di cella valido (da 0 a 9)\n");
                                                    goto LOOP;

                                                }


                                            break;
                                            }
                                     }
                                     break;









                                case 1:
                                    printf("\n\nHai vinto\n");
                                    shouldExit = 1;
                                    write(sd,&shouldExit,sizeof(int));
                                    break;
                                case 2:
                                    printf("\n\nHai perso\n");
                                    shouldExit = 1;
                                    write(sd,&shouldExit,sizeof(int));
                                    break;
                                case 3:
                                    printf("\n\nPareggio\n");
                                    shouldExit = 1;
                                    write(sd,&shouldExit,sizeof(int));
                                    break;

                            }

                        }
                    }

                }

        if(!shouldExit)
                    printf("\nInserito comando errato. Digita !help per la lista dei comandi.\n");

    }
        END : close(sd);
        return 0;
    }


    int checkResult(char map[],char simbol, char simbol_vs){
        //1=win 2=lose 3=draw 4=not finished
        //Win

        if(map[1] == simbol && map[2] == simbol && map[3] == simbol)
            return 1;
        if(map[4] == simbol && map[5] == simbol && map[6] == simbol)
            return 1;
        if(map[7] == simbol && map[8] == simbol && map[9] == simbol)
            return 1;
        if(map[1] == simbol && map[4] == simbol && map[7] == simbol)
            return 1;
        if(map[2] == simbol && map[5] == simbol && map[8] == simbol)
            return 1;
        if(map[3] == simbol && map[6] == simbol && map[9] == simbol)
            return 1;
        if(map[1] == simbol && map[5] == simbol && map[9] == simbol)
            return 1;
        if(map[7] == simbol && map[5] == simbol && map[3] == simbol)
            return 1;
        //Lose
        if(map[1] == simbol_vs && map[2] == simbol_vs && map[3] == simbol_vs)
            return 2;
        if(map[4] == simbol_vs && map[5] == simbol_vs && map[6] == simbol_vs)
            return 2;
        if(map[7] == simbol_vs && map[8] == simbol_vs && map[9] == simbol_vs)
            return 2;
        if(map[1] == simbol_vs && map[4] == simbol_vs && map[7] == simbol_vs)
            return 2;
        if(map[2] == simbol_vs && map[5] == simbol_vs && map[8] == simbol_vs)
            return 2;
        if(map[3] == simbol_vs && map[6] == simbol_vs && map[9] == simbol_vs)
            return 2;
        if(map[1] == simbol_vs && map[5] == simbol_vs && map[9] == simbol_vs)
            return 2;
        if(map[7] == simbol_vs && map[5] == simbol_vs && map[3] == simbol_vs)
            return 2;
        //Draw
        if(!(map[1]) == '\0' && !(map[2]) == '\0' && !(map[3]) == '\0' && !(map[4]) == '\0' && !(map[5]) == '\0' && !(map[6]) == '\0' && !(map[7]) == '\0' && !(map[8]) == '\0' && !(map[9]) == '\0')
            return 3;
        //Not finished
        return 0;
    }



