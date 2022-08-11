/*******************************************************************************
                                    SERVIDOR
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
#include <netdb.h>
#include <regex.h>
#include <dirent.h>
#include <sodium.h>

#define BUFLEN 1024
#define SA struct sockaddr


//Global variables

struct sockaddr_in serveraddr, clientaddr;
int len;

//Inteiros
int tcpfd, udpfd, client, recv_udp, maxfd;
int conta = 0;

//Arrays
char client_address[BUFLEN+1];
char buffer[BUFLEN];

//Multiplexing
/*fd_set read_set;
FD_ZERO(&read_set);*/

//Directory
DIR *sevfiles;
struct dirent *lista;

//encriptação
unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES];


void erro(char *s)
{
    perror(s);
    exit(1);
}

void cleanup(){
    wait(NULL);
    fflush(stdout);
    exit(0);
}

//Função que vai decidir se o servidor recebeu chamada do protocolo udp ou tcp
/*int decider(int x, int y){
    if(x > y)
        return x;
    else
        return y;
}*/

void process_client(int client_fd, struct sockaddr_in* addr) {
    //Converte um endereço ipv4 para string
    char mensagem[BUFLEN + 1];
    strcpy(mensagem, "");
    inet_ntop(AF_INET, (struct sockaddr_in *) addr, client_address, INET_ADDRSTRLEN);
    printf("[TCP] Client connected! Address: %s\n", client_address);
    while (1) {
        printf("Waiting on clients command...\n");
        strcpy(mensagem, "");
        int nread = read(client_fd, mensagem, BUFLEN - 1);
        mensagem[nread] = '\0';
        printf("Message: %s\n", mensagem);

        char *token = strtok(mensagem, " ");
        if (!strcmp(token, "LIST")) {
            if ((sevfiles = opendir("/media/sf_PIRC/server_files")) != NULL) {
                /*imprime todos os ficheiros da diretoria*/
                while ((lista = readdir(sevfiles)) != NULL) {
                    if(strcmp(lista->d_name,".") != 0 && strcmp(lista->d_name,"..") != 0) {
                        strcat(buffer, lista->d_name);
                        strcat(buffer, "\n");
                    }
                }
                closedir(sevfiles);
                write(client_fd, buffer, BUFLEN + 1);
                strcpy(buffer, "");
            } else {
                /* could not open directory */
                erro("Nao conseguiu abrir a diretoria\n");
            }
        } else if (!strcmp(token, "DOWNLOAD")) {
            token=strtok(NULL," ");
            if (!strcmp(token, "TCP")) {
                token = strtok(NULL, " ");
                if (!strcmp(token, "ENC")) {
                    //crypto_secretstream_xchacha20poly1305_keygen(key);
                    //write(client_fd, key, sizeof(key));

                } else if (!strcmp(token, "NOR")) {
                    char nome_fich[20];
                    char path[30];
                    strcpy(path,"/media/sf_PIRC/server_files");
                    token = strtok(NULL," ");
                    int check = 1;
                    if ((sevfiles = opendir(path)) != NULL) {
                        while ((lista = readdir(sevfiles)) != NULL) {
                            if (!strcmp(lista->d_name, token)){
                                strcat(path,"/");
                                strcpy(nome_fich,lista->d_name);
                                check = 0;
                                break;
                            }
                        }
                        closedir(sevfiles);
                        strcpy(buffer, "");
                        if (check == 1) {
                            printf("File not found\n");
                            write(client_fd,"erro",40);
                        } else {
                            strcat(path,nome_fich);
                            char *token2 = strtok(token, ".");
                            token2=strtok(NULL,".");
                            printf("%s\n",path);
                            if (!strcmp(token2, "txt")) {
                                FILE *fich;
                                char info[BUFLEN];
                                fich = fopen(path, "r");
                                if (fich == NULL) {
                                    printf("Could not open file %s\n", path);
                                } else {
                                    while (fgets(info, BUFLEN, fich) != NULL) strcat(buffer, info);
                                    write(client_fd, buffer, BUFLEN + 1);
                                    strcpy(buffer, "");
                                    strcpy(path,"");
                                    fclose(fich);
                                }
                            }else if(!strcmp(token2,"jpg")){

                            }

                            else if(!strcmp(token2,"wav")){

                            }

                        }
                    }
                    else {
                        /* could not open directory */
                        erro("Nao conseguiu abrir a diretoria");
                    }
                }
                else if (!strcmp(token, "UDP")) {

                }


            }
        }
    }
    exit(0);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("server <port> <nº of clients>\n");
        exit(-1);
    }

    //Serveraddr treatment
    bzero((void *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((short)atoi(argv[1]));

    /*TCP listening socket*/
    // Cria um socket para recepção de pacotes TCP
    if((tcpfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        erro("tcp socket error");
    }
    // Associa o socket à informação de endereço
    if((bind(tcpfd, (SA*)&serveraddr, sizeof(serveraddr))) == -1)
        erro("bind tcp error");

    if(listen(tcpfd, atoi(argv[2]))<0)
        erro("listen error");

    printf("SERVER STARTED\n\n");
    signal(SIGINT, cleanup);
    len=sizeof(clientaddr);

    while(1){
        //clean finished child processes, avoiding zombies
        //must use WNOHANG or would block whenever a child process was still working
        while(waitpid(-1,NULL,WNOHANG)>0);
        // wait for new connection
        if( (client = accept(tcpfd,(SA*)&clientaddr, (socklen_t *)&len)) == -1)
            erro("client accept error");
        if (client > 0) {
            if (fork() == 0) {
                close(tcpfd);
                process_client(client,&clientaddr);
                exit(0);
            }
            close(client);
        }

    }
}
