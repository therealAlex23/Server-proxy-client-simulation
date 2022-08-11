/*******************************************************************************
                                    CLIENT
 *******************************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <regex.h>
#include <sys/stat.h>
#include <sodium.h>

#define BUFLEN 1024
#define SA struct sockaddr

//Global variables
struct sockaddr_in addr;
struct hostent *hostPtr;
struct stat sta = {0};

//Inteiros
int udpfd, tcpfd,fd;

//Arguments treats
int nread = 0;
//Arrays
char server_dest[100];
char proxy_dest[100];
char buffer[BUFLEN];

//encriptação
unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES];

//Erros
void erro(char *s)
{
    perror(s);
    exit(1);
}


int main(int argc, char *argv[]) {

    char comando[BUFLEN];
    if (argc != 5) {
        printf("cliente <proxy> <server> <port> <protocol>\n");
        exit(-1);
    }

    strcpy(proxy_dest, argv[1]);
    if ((hostPtr = gethostbyname(proxy_dest)) == 0) {
        erro("Nao consegui obter endereço proxy");
    }

    strcpy(server_dest, argv[2]);
    if ((hostPtr = gethostbyname(server_dest)) == 0) {
        erro("Nao consegui obter endereço server");
    }


    if(strcmp(argv[4],"tcp")){
        erro("Protocolo inválido: escreva tcp");
    }

    bzero((void *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[3]));

    if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)  erro("socket");
    if(connect(fd,(SA *)&addr,sizeof (addr)) < 0) erro("Connect");


    while(1) {
        printf("\nINSIRA UM COMANDO: ");
        fgets(comando,30,stdin);


        write(fd, comando, strlen(comando) - 1);
        char *token = strtok(comando, " ");
        if (!strcmp(token, "LIST\n")) {
            printf("Waiting for list...\n");
            nread = read(fd,buffer,BUFLEN+1);
            fflush(stdout);
            buffer[nread] = '\0';
            printf("Lista dos ficheiros:\n\n%s",buffer);
            strcpy(buffer,"");
        }
        else if (!strcmp(token, "DOWNLOAD")) {
            token=strtok(NULL," ");
            if (token == NULL) {
                printf("nao existe mais string,erro\n");
            }
            else {
                    if (!strcmp(token, "TCP")) {
                        token = strtok(NULL, " ");
                        if (token == NULL) {
                            printf("nao existe mais string,erro\n");
                        }
                        else if (!strcmp(token, "ENC")) {
                            //read(fd, key, sizeof(key));

                        }
                        else if (!strcmp(token, "NOR")) {
                            token = strtok(NULL, " ");
                            if (token == NULL) {
                                printf("nao existe mais string,erro\n");
                            }

                            else {
                                printf("Waiting for download...\n");
                                read(fd,buffer,BUFLEN+1);
                                if(strcmp(buffer,"erro")) {
                                    printf("A armazenar o ficheiro na diretoria de downloads...\n");
                                    char path[30];
                                    strcpy(path,"/media/sf_PIRC/downloads");
                                    if (stat(path, &sta) == -1) {
                                        mkdir(path, 0700);
                                    }
                                    strcat(path,"/");
                                    strcat(path,token);
                                    char* pos;
                                    if((pos=strchr(path,'\n'))!=NULL) *pos='\0';
                                    FILE *txt;
                                    txt = fopen(path, "w");
                                    printf("%s\n",path);
                                    if (txt == NULL) {
                                        erro("Não foi possível criar o ficheiro\n");
                                    }
                                    fputs(buffer, txt);
                                    fclose(txt);
                                    printf("Ficheiro criado com sucesso!\n");
                                    strcpy(buffer,"");
                                    strcpy(path,"");
                                }else{
                                    printf("FILE DOESENT EXIST\n");
                                }
                            }
                        }
                        else{
                            printf("protocolo nao esta certo\n");
                        }
                    }
                    else if (!strcmp(token, "UDP")) {

                    }
                    else{
                        printf("protocolo nao esta certo\n");
                    }
            }

        }

        else if (strcmp(token, "QUIT")) {
            printf("Bye bye\n");
            close(fd);
            exit(0);
        } else {
            printf("Comando inválido. Insira um novo comando\n");
        }
    }
}
