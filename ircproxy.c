/*******************************************************************************
                                    PROXY
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

#define PROXY_PORT 9020
#define BUFLEN 1024
#define SA struct sockaddr

//Global variables
struct sockaddr_in serveraddr, clientaddr;
int tcp_server_fd, tcp_client_fd, client,server_fd;
int proxy_port;

fd_set read_set;

//Erros
void erro(char *s)
{
    perror(s);
    exit(1);
}

//Cleanup
void cleanup(){
    wait(NULL);
    fflush(stdout);
    if(close(tcp_server_fd) != 0){
        erro("closing tcp socket error");
    }
    if(close(tcp_client_fd) != 0){
        erro("closing udp socket error");
    }
    exit(0);
}

void process_chat(int client_fd, struct sockaddr_in clientaddr) {
    //Converte um endereço ipv4 para string
    //char fich[20];
    char mensagem_client[BUFLEN + 1];
    char mensagem_server[BUFLEN + 1];
    strcpy(mensagem_client,"");
    strcpy(mensagem_server,"");
    int nread = 0;
    printf("Proxy started\n\n");
    while(1){
        strcpy(mensagem_client,"");

        //printf("Waiting for clients command...\n");
        nread = read(client_fd, mensagem_client, BUFLEN + 1);
        mensagem_client[nread] = '\0';
        strcpy(mensagem_server,mensagem_client);
        printf("Received from client %s\n",mensagem_client);
        fflush(stdout);

        bzero((void *)&clientaddr, sizeof(clientaddr));
        clientaddr.sin_family = AF_INET;
        clientaddr.sin_port = htons(proxy_port);

        if((server_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
            erro("tcp socket client error");
        if(connect(server_fd,(SA *)&clientaddr,sizeof (clientaddr)) < 0)
            erro("Connect");

        char* token=strtok(mensagem_client," ");
        if(!strcmp(token,"LIST")){
            printf("Client requested the list of files\n");
            write(server_fd, mensagem_client, BUFLEN + 1);
            nread = read(server_fd,mensagem_client,BUFLEN+1);
            mensagem_client[nread] = '\0';
            write(client_fd,mensagem_client,BUFLEN+1);
            fflush(stdout);
            printf("Server sent the list of the files to the client\n\n");
        }
        else if(!strcmp(token,"DOWNLOAD")){
            printf("Client requested a download\n");
            write(server_fd, mensagem_server, BUFLEN + 1);
            strcpy(mensagem_server,"");
            read(server_fd,mensagem_server,BUFLEN + 1);
            if(!strcmp("erro",mensagem_server)){
                printf("Download failed\n");
            }else{
                write(client_fd,mensagem_server,BUFLEN + 1);
                printf("Server sent the download to the client successfully!\n\n");
            }
        }
        else if(!strcmp(token,"QUIT\n")){
            printf("Client disconnected\n");
        }
        else{
            printf("Client sent an invalid command\n\n");
        }

    }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("ircproxy <port>\n");
        exit(-1);
    }

    proxy_port = (short)atoi(argv[1]);

    if (proxy_port <= 0)
        erro("Port introduzido incorreto");

    bzero((void *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PROXY_PORT);

    if((tcp_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        erro("tcp socket server error");

    if((bind(tcp_server_fd, (SA*)&serveraddr, sizeof(serveraddr))) == -1)
        erro("bind tcp server error");

    if(listen(tcp_server_fd, 10)<0)
        erro("listen server error");


    int len=sizeof(clientaddr);

    signal(SIGINT, cleanup);
    while(1){
        //clean finished child processes, avoiding zombies
        //must use WNOHANG or would block whenever a child process was working
        while(waitpid(-1,NULL,WNOHANG)>0);
        if((client = accept(tcp_server_fd,(SA*)&clientaddr, (socklen_t *)&len)) == -1)
            erro("Client accept error");

        if (client > 0) {
            if (fork() == 0) {
                close(tcp_server_fd);
                process_chat(client, clientaddr);
                exit(0);
            }
            close(client);
        }
    }


}



/*



int main(int argc, const char * argv[])
{
    char *buffer;
    FILE *picture;
    FILE *newPicture;
    struct stat st;
    long fileSize = 0;


    picture = fopen("PATH/root/game-of-thrones-poster.jpg", "rb");
    fstat(picture, &st);
    fileSize = st.st_size;
    if(fileSize > 0) {
        buffer = malloc(fileSize);

        if(read(picture, buffer, fileSize) < 0) {
            printf("Error reading file");
        }
        fclose(picture);

        newPicture = fopen("PATH/root/new.jpg", "wb");
        write(newPicture, buffer, fileSize);
    }
    free(buffer);
}



#include <fstream>

using namespace std;

int main(){

    const char *g="file1.WAV";
    const char *h="out.txt";

    FILE *file;
    FILE *pFile;

    char *buffer;
    unsigned long fileLen;

    //Open file
    file = fopen(g, "rb");

    if (!file){fprintf(stderr, "Unable to open file %s", "file1.WAV");}

    //Get file length
    fseek(file, 0, SEEK_END); //Aqui podemos usar a função rewind
    fileLen=ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    buffer=(char *)malloc(sizeof(char)*(fileLen+1));
    if (!buffer){fprintf(stderr, "Memory error!");fclose(file);}

    //Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    fclose(file);

    //Do what ever with buffer

    for (int c=0;c<fileLen;c++){
        printf("%.2X ", (int)buffer[c]);

        // put an extra space between every 4 bytes
        if (c % 4 == 3){
            printf(" ");
        }
        // Display 16 bytes per line
        if (c % 16 == 15){
            printf("\n");
        }
    }

    pFile = fopen ( h , "wb" );

    // fwrite (buffer , 1 , sizeof(buffer) , pFile );
    //    char name [100];
    for (int c=0;c<fileLen;c++){
        fprintf(pFile,"%.2X ", (int)buffer[c]);

        // put an extra space between every 4 bytes
        if (c % 4 == 3){
            fprintf(pFile," ");
        }

        // Display 16 bytes per line
        if (c % 16 == 15){
            fprintf(pFile,"\n");
        }
    }

    fclose (pFile);
    free(buffer);
    return 0;
}
*/

