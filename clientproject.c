#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT_NUMBER 6060
#define MAX_DATA_SIZE 1024

void initClient(struct sockaddr_in serv_addr);

void checkClientStatus(int clientSocket, struct sockaddr_in *serv_addr);

void clientCommandController(int clientSocket, char *buffer, const char *command);

void catClient(int clientSocket, char *buffer);

void copy(int clientSocket, char *buffer);

void createDirectoryClient(int clientSocket, char *buffer);

void terminateandKill(int clientSocket, char *buffer);

void removeClient(int clientSocket, char *buffer);

void removeDirectoryClient(int clientSocket, char *buffer);

void nanoClient(int clientSocket, char *buffer);

int main(int argc, char const *argv[])
{
    int clientSocket = 0, result;
    char str[MAX_DATA_SIZE];
    int terminateAndKill = 0;
    struct sockaddr_in serv_addr;
    
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    initClient(serv_addr);

    // Convert IPv4 address from text to binary form
    checkClientStatus(clientSocket, &serv_addr);

    char command[MAX_DATA_SIZE];
    int maxNumber = 0;
    while(maxNumber <= 10)
    {
        printf("Enter a command: \n");
        fgets(command , MAX_DATA_SIZE, stdin);
        strtok(command , '\0');
        send(clientSocket , command, strlen(command), 0);

        clientCommandController(clientSocket, str, command);
        maxNumber++;
    }
    return 0;
}

void clientCommandController(int clientSocket, char *buffer, const char *command) {
    int result;
    if(strcmp(command, "cat") == 0)
    {
        catClient(clientSocket, buffer);
    }
    else if(strcmp(command, "cp") == 0)
    {
        copy(clientSocket, buffer);
    }
    else if(strcmp(command, "createDirectory") == 0)
    {
        createDirectoryClient(clientSocket, buffer);
    }
    else if(strcmp(command, "killAndTerminate") == 0)
    {
        terminateandKill(clientSocket, buffer);
    }
    else if(strcmp(command, "rm") == 0)
    {
        removeClient(clientSocket, buffer);
    }
    else if(strcmp(command, "rmd") == 0)
    {
        removeDirectoryClient(clientSocket, buffer);
    }
    else if(strcmp(command, "ls") == 0)
    {
        result = read(clientSocket, buffer, 1024);
        printf("%s\n", buffer);
        printf("\n");
    }

    else if(strcmp(command, "printProcess") == 0)
    {

        result = read(clientSocket, buffer, 1024);
        printf("%s\n", buffer);
    }
    else if(strcmp(command, "wc") == 0){
        result = read(clientSocket, buffer, 1024);
        printf("%s\n", buffer);
    }
    else if(strcmp(command, "nano") == 0)
    {
        nanoClient(clientSocket, buffer);
    }
    else
    {
        printf("\n-bash: %s: command not found\n", buffer); //error
        exit(1);
    }
}

void nanoClient(int clientSocket, char *buffer) {
    int result;
    char data[MAX_DATA_SIZE];
    char filename[1000];
    printf("Enter the filename to create and enter text: \n");
    scanf("%s", filename);   //read file name
    send(clientSocket, filename, strlen(filename), 0);
    result = read(clientSocket, buffer, 1024);
    printf("%s\n", buffer);
}

void removeDirectoryClient(int clientSocket, char *buffer) {
    int result;
    char dirname[10];
    printf("Enter the directory name: \n");
    scanf("%s", dirname);
    send(clientSocket, dirname, strlen(dirname), 0);
    result = read(clientSocket, buffer, 1024);
    printf("%s\n", buffer);
}

void removeClient(int clientSocket, char *buffer) {
    int result;
    char name[10];
    printf("Enter file name to delete: \n");
    scanf("%s", name);
    send(clientSocket, name, strlen(name), 0);
    result = read(clientSocket, buffer, 1024);
    printf("%s\n", buffer);
}

void terminateandKill(int clientSocket, char *buffer) {
    int result;
    int *pid;
    printf("Enter pid to kill: \n");
    scanf("%d", pid);
    if((send(clientSocket, pid, 10 * sizeof(int), 0)) < 0)
    {
        printf("%s\n","Send failed");
    }
    else
    {
        result = read(clientSocket, buffer, 1024);
        printf("%s\n", buffer);
    }
}

void createDirectoryClient(int clientSocket, char *buffer) {
    int result;
    char dirname[10];
    printf("Enter the directory name: ");
    scanf("%s", dirname);
    send(clientSocket, dirname, strlen(dirname), 0);
    result = read(clientSocket, buffer, 1024);
    printf("%s\n", buffer);
}

void copy(int clientSocket, char *buffer) {
    int result;
    char fName[100];
    char sName[100];
    printf("Enter file name to open for reading: \n");
    scanf("%s", fName);
    send(clientSocket, fName, strlen(fName), 0);
    printf("Enter file name to open for writing: \n");
    scanf("%s", sName);
    send(clientSocket, sName, strlen(sName), 0);
    result = read(clientSocket, buffer, 1024);
    printf("%s\n", buffer);
}

void catClient(int clientSocket, char *buffer) {
    int result;
    char fName[100];
    printf("Enter file name:");
    scanf("%s", fName);
    send(clientSocket, fName, strlen(fName), 0);
    result = read(clientSocket, buffer, 1024);
    printf("%s\n", buffer);
}

void checkClientStatus(int clientSocket, struct sockaddr_in *serv_addr) {
    if(inet_pton(AF_INET, "127.0.0.1", &(*serv_addr).sin_addr) <= 0)
    {
        printf("\nInvalid address/Address not supported \n");
        exit(1);
    }

    if (connect(clientSocket, (struct sockaddr *) serv_addr, sizeof((*serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        exit(1);
    }
}

void initClient(struct sockaddr_in serv_addr) {
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUMBER);
}


