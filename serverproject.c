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
#include <pthread.h>

#define MAX_DATA_SIZE 1024
#define PORT_NUMBER 6060

void ls();

void printProcess();

int wc();

void cat(FILE *fptr, void *);

void killAndTerminate(void *new_socket);

void nano(FILE *filePointer, void *new_socket);

void rm(FILE *fptr, void *new_socket);

void createDirectory(char *dirname, void *new_socket);

void rmd(char *dirname, void *new_socket);

void cp(FILE *fptr1, FILE *fptr2, void *new_socket);

void *connection_handler(void *);

void *client(void *new_socket);

typedef struct pthread_arg_t {
    int new_socket_fd;
    struct sockaddr_in client_address;
} pthread_arg_t;

struct sockaddr_in init(struct sockaddr_in *address);


void *thread(void *arg);

int setSocket(int socket_fd, struct sockaddr_in *address);

pthread_attr_t getThreadAttr(pthread_attr_t *pthread_attr);

int getCommand(int *new_socket_fd, FILE *filePointer, FILE *pIobuf, FILE *pIoBuf2, const char *data, char *directoryName);

void pipeprocess(const int *fd);

void closePipe(const int *fd);

struct sockaddr_in init(struct sockaddr_in *address) {
    memset(address, 0, sizeof((*address)));
    (*address).sin_family = AF_INET;
    (*address).sin_port = htons(PORT_NUMBER);
    (*address).sin_addr.s_addr = INADDR_ANY;
    return (*address);
}

int main(int argc, char *argv[]) {

    int socket_fd, new_socket_fd;
    struct sockaddr_in address;

    pthread_t pthread;
    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;

    socklen_t client_address_len;
    address = init(&address);


    socket_fd = setSocket(socket_fd, &address);


    pthread_attr = getThreadAttr(&pthread_attr);

    while (1) {

        pthread_arg = (pthread_arg_t *) malloc(sizeof *pthread_arg);
        if (!pthread_arg) {
            perror("malloc");
            continue;
        }


        client_address_len = sizeof pthread_arg->client_address;
        new_socket_fd = accept(socket_fd, (struct sockaddr *) &pthread_arg->client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            free(pthread_arg);
            continue;
        }


        pthread_arg->new_socket_fd = new_socket_fd;


        if (pthread_create(&pthread, &pthread_attr, thread, (void *) pthread_arg) != 0) {
            perror("pthread_create");
            free(pthread_arg);
            continue;
        }
    }
    return 0;
}

pthread_attr_t getThreadAttr(pthread_attr_t *pthread_attr) {
    if (pthread_attr_init(pthread_attr) != 0) {
        perror("thread initialization failed");
        exit(1);
    }

    if (pthread_attr_setdetachstate(pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("thread failure");
        exit(1);
    }
    return (*pthread_attr);
}

int setSocket(int socket_fd, struct sockaddr_in *address) {
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }


    if (bind(socket_fd, (struct sockaddr *) address, sizeof(*address)) == -1) {
        perror("binding error");
        exit(1);
    }


    if (listen(socket_fd, 10) < 0) {
        perror("error in listening");
        exit(1);
    }
    return socket_fd;
}


void *thread(void *arg) {
    pthread_arg_t *pthread_arg = (pthread_arg_t *) arg;
    int new_socket_fd = pthread_arg->new_socket_fd;
    struct sockaddr_in client_address = pthread_arg->client_address;

    free(arg);

    FILE *filePointer, *pointerToFirstFile, *pointerToSecondFIle;
    char command[MAX_DATA_SIZE];
    char *directoryName;
    int bashInput, max = 0;
    while (max <= 10) {
	
        bashInput = read(new_socket_fd, command, MAX_DATA_SIZE);
        new_socket_fd = getCommand(&new_socket_fd, filePointer, pointerToFirstFile, pointerToSecondFIle, command, directoryName);
    }

    close(new_socket_fd);
    return NULL;
}

//list of commands
int getCommand(int *new_socket_fd, FILE *filePointer, FILE *pIobuf, FILE *pIoBuf2, const char *data, char *directoryName) {
    if (strcmp(data, "cat") == 0) {
        cat(filePointer, (void *) new_socket_fd);

    } else if (strcmp(data, "cp") == 0) {
        cp(pIobuf, pIoBuf2, (void *) new_socket_fd);

    } else if (strcmp(data, "wc") == 0) {
        wc();

    } else if (strcmp(data, "ls") == 0) {
        ls((void *) new_socket_fd);

    } else if (strcmp(data, "rm") == 0) {

        rm(filePointer, (void *) new_socket_fd);

    } else if (strcmp(data, "killAndTerminate") == 0) {
        killAndTerminate((void *) new_socket_fd);

    } else if (strcmp(data, "rmd") == 0) {
        rmd(directoryName, (void *) new_socket_fd);

    }else if (strcmp(data, "createDirectory") == 0) {
        createDirectory(directoryName, (void *) new_socket_fd);

    }  else if (strcmp(data, "nano") == 0) {
        nano(filePointer, (void *) new_socket_fd);

    } else if (strcmp(data, "printProcess") == 0) {
        printProcess((void *) new_socket_fd);

    } else {
        printf("\n-bash: %s: command not found. Please try again!!!!!\n", data);
        exit(1);
    }
    return (*new_socket_fd);
}

void cat(FILE *fptr, void *new_socket) {
    int input;
    int socket = *(int *) new_socket;
    char fileName[100], c, buffer[100];
    int i = 0;

    input = read(socket, fileName, 100);

    fptr = fopen(fileName, "r");
    if (fptr == NULL) {
        char empty[15] = "File is empty\n";
        send(socket, empty, strlen(empty), 0);
        exit(0);
    }

    c = fgetc(fptr);
    printf("%c", c);
    while (c != EOF) {
        buffer[i++] = c;
        c = fgetc(fptr);
        printf("%c", c);
    }
    send(socket, buffer, strlen(buffer), 0);
    fclose(fptr);
}


void cp(FILE *fptr1, FILE *fptr2, void *new_socket) {

    char fileName[100], toBeCopiedFileName[100], c;
    int socket = *(int *) new_socket;
    int input1, input2;

    input1 = read(socket, fileName, 100);
    fptr1 = fopen(fileName, "r");

    if (fptr1 == NULL) {
        char empty[30] = "Cannot open file.";
        send(socket, empty, strlen(empty), 0);
        exit(0);
    }


    input2 = read(socket, toBeCopiedFileName, 100);
    fptr2 = fopen(toBeCopiedFileName, "w");
    if (fptr2 == NULL) {
        char empty1[30] = "Cannot open file.";
        send(socket, empty1, strlen(empty1), 0);
        exit(0);
    }

    c = fgetc(fptr1);
    while (c != EOF) {
        fputc(c, fptr2);
        c = fgetc(fptr1);
    }
    char copied[30] = "Contents are copied";
    send(socket, copied, strlen(copied), 0);

    fclose(fptr1);
    fclose(fptr2);
}


void ls() {
    char* exp [] = {"ls",NULL};
    pid_t pid;
    pid = fork();
    if(pid == 0){
    execvp("ls",exp);
    perror("Execvp failed from child");
    exit(1);
    
    }
}


void rm(FILE *fptr, void *new_socket) {

    char dstBuf[100];
    int input, toRemove;
    int socket = *(int *) new_socket;

    input = read(socket, dstBuf, 100);


    toRemove = remove(dstBuf);

    if (toRemove == 0) {
        char statusSuccess[50] = "File deleted successfully. ";
        send(socket, statusSuccess, strlen(statusSuccess), 0);
    } else {
        char statusError[30] = "Unable to toRemove the dstBuf.";
        send(socket, statusError, strlen(statusError), 0);
    }
}


void nano(FILE *filePointer, void *new_socket) {
    char file[MAX_DATA_SIZE];
    char fileName[100];
    int input;
    int sock = *(int *) new_socket;
    input = read(sock, fileName, 100);

    if (filePointer == NULL) {
        char statusError[100] = "File creation failed";
        send(sock, statusError, strlen(statusError), 0);
    }

    filePointer = fopen(fileName, "w");
    while ((sizeof(filePointer)) <= 10) {
        fgets(file, MAX_DATA_SIZE, stdin);
        fputs(file, filePointer);
        fclose(filePointer);
    }
    char saved[50] = "Filed saved successfully.";
    send(sock, saved, strlen(saved), 0);
}

void printProcess() {
    pid_t pid = getpid();
    printf("pid: %d", pid);
    time_t start = time(NULL);
    time_t final = time(NULL);
    printf("\nTime it takes to execute is %ld seconds", (final - start));
}

int wc() {
    FILE *filePointer;

    char filename[100], c;

    printf("Enter filename in order to open \n");
    scanf("%s", filename);


    filePointer = fopen(filename, "r");
    if (filePointer == NULL) {
        printf("Cannot open file \n");
        exit(0);
    }

    pid_t pid;
    int fd[2];
    pipeprocess(fd);

    pid = fork();

    if (pid == 0) {

        pipeprocess(fd);

        char *exp[] = {"ls", ".", NULL};
        if ((execvp("ls", exp)) < 0) {
            perror("Execvp failed from child");
        }

        exit(EXIT_FAILURE);

    } else if (pid > 0) {

        wait(NULL);
        closePipe(fd);

        char *exp[] = {"wc", "-l", NULL};

        if ((execvp("wc", exp)) < 0) {
            perror("Execvp failed from parent");
        }
        exit(EXIT_FAILURE);
    } else {
        printf("Error in forking");
        exit(EXIT_FAILURE);

    }

    close(fd[0]);
    close(fd[1]);
}

void closePipe(const int *fd) {
    close(0);
    dup(fd[0]);
    close(fd[1]);
    close(fd[0]);
}

void pipeprocess(const int *fd) {
    close(1);
    dup(fd[1]);
    close(fd[0]);
    close(fd[1]);
}

void createDirectory(char *dirname, void *new_socket) {
    int bool;
    int socket = *(int *) new_socket;
    int input;
    char dir[30];
    input = read(socket, dir, 100);
    bool = mkdir(dir, 0);

    if (!bool) {
        char status[30] = "Directory Created\n";
        send(socket, status, strlen(status), 0);
    } else {
        char error[100] = "Unable to create directory\n";
        send(socket, error, strlen(error), 0);
    }
}


void killAndTerminate(void *new_socket) {

    int socket = *(int *) new_socket;
    int toll = 0;
    int input, pid[MAX_DATA_SIZE];
    input = read(socket, pid, MAX_DATA_SIZE);

    if ((kill(pid[toll], SIGKILL)) == 0) {
        char status[30] = "Process Killed\n";
        send(socket, status, strlen(status), 0);
        toll++;
    } else {
        char status[30] = "Process cannot be terminated";
        send(socket, status, strlen(status), 0);
    }
}


void rmd(char *dirname, void *new_socket) {

    int socket = *(int *) new_socket;
    char dstBuf[30];
    int bool;
    int value;

    value = read(socket, dstBuf, 100);
    bool = rmdir(dstBuf);
    if (!bool) {
        char status[30] = "Directory is removed\n";
        send(socket, status, strlen(status), 0);
    } else {
        char error[100] = "Not able to remove directory\n";
        send(socket, error, strlen(error), 0);
    }
}







