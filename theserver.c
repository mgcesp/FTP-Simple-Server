/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

// IN TERMINAL: ./server.out 4040 // a port number

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n = 1;
    int connectionRecv;
    pid_t pid;

    char user[256];
    char password[256];
    char root[256];
    int login;    // login flag

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    /*** CREATE SOCKET *********************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    // memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /*** BIND SOCKET *********************/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    printf("bind socket to port %d...\n", portno);

    listen(sockfd,5);

    for(;;)
    {
        /*** ACCEPT CONNECTION *********************/
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0)
        {
            error("ERROR on accept");
            exit(-1);
        }

        while(!login)
        {
            /*** CLEAR BUFFER *********************/
            bzero(buffer,256);

            /*** READ SOCKET INTO BUFFER *********************/
            n = read(newsockfd, buffer, 255);

            if (n < 0) error("ERROR reading from socket");

            /*** USER COMMAND *********************/
            if (strncmp(buffer, "user ", 5) == 0)
            {
                // get size of buffer
                int bufferSize = strlen(buffer);

                // get size of username
                int userSize = (bufferSize - 5);

                // copy the second arg to username string
                int iter;

                for (iter = 0; iter < bufferSize; iter++)
                {
                    user[iter] = buffer[(iter + 5)];
                }
                user[iter] = '\0';

                printf("Username entered: %s\n", user);
                // send username entered to client
                n = write(newsockfd,"USERNAME ENTERED", 17);
                if (n < 0) error("ERROR writing to socket");
            }

            /*** PASSWORD COMMAND *********************/
            else if (strncmp(buffer, "password ", 9) == 0)
            {
                // get size of buffer
                int bufferSize = strlen(buffer);

                // get size of password
                int passSize = (bufferSize - 9);

                // copy the second arg to password string
                int iter;

                for (iter = 0; iter < bufferSize; iter++)
                {
                    password[iter] = buffer[(iter + 9)];
                }
                password[iter] = '\0';

                printf("Password entered: %s\n", password);
                // send password entered to client
                // n = write(newsockfd,"PASSWORD ENTERED", 17);
                if (n < 0) error("ERROR writing to socket");

                /*** COMPARE USER WITH PASSWORD *********************/
                if (strlen(user) > 0 && strlen(password) > 0 && strncmp(user, password, strlen(user)) == 0)
                {
                    login = 1;

                    // chdir("/");

                    printf("User is Logged in!\n");
                    n = write(newsockfd,"LOGIN SUCCESS!", strlen("LOGIN SUCCESS!") + 1);
                    if (n < 0) error("ERROR writing to socket");
                }
                else
                {
                    // printf("User entered wrong password.\n");
                    n = write(newsockfd,"WRONG PASSWORD", strlen("WRONG PASSWORD") + 1);
                    if (n < 0) error("ERROR writing to socket");
                }
            }

            else
            {
                // printf("User must login first.\n");
                n = write(newsockfd,"LOGIN TO CONTINUE", strlen("LOGIN TO CONTINUE") + 1);
                if (n < 0) error("ERROR writing to socket");
            }
        }

        // while login
        while (login == 1)
        {
            printf("Waiting for commands!\n");

            /*** CLEAR BUFFER *********************/
            bzero(buffer,256);

            /*** READ SOCKET INTO BUFFER *********************/
            n = read(newsockfd, buffer, 255);

            if (n < 0) error("ERROR reading from socket");

            /*** MKDIR COMMAND *********************/
            if (strncmp(buffer, "mkdir ", 6) == 0)
            {
                char dir[256]; // new directory name

                int bufferSize = strlen(buffer);
                int dirSize = (bufferSize - 6);

                int i;
                for (i = 0; i < bufferSize; i++)
                {
                    dir[i] = buffer[(i + 6)];
                }
                dir[i] = '\0';

                printf("COMMAD SENT: %s\n", buffer);
                system(buffer);
                n = write(newsockfd,"DIR CREATED\n", strlen("DIR CREATED") + 1);
                if (n < 0) error("ERROR writing to socket");
            }
            /*** CD COMMAND *********************/
            else if (strncmp(buffer, "cd ", 3) == 0)
            {
                char oldpath[256];
                char newpath[256];

                int bufferSize = strlen(buffer);
                int oldpathSize = (bufferSize - 3);

                int i;
                for (i = 0; i < bufferSize; i++)
                {
                    oldpath[i] = buffer[(i + 3)];
                }
                oldpath[i] = '\0';

                realpath(oldpath, newpath);
                printf("Change path to: %s\n", newpath);
                n = write(newsockfd,"CHANGED DIR", strlen("CHANGED DIR") + 1);
                if (n < 0) error("ERROR writing to socket");
            }
            /*** PWD COMMAND *********************/
            else if (strncmp(buffer, "pwd", 3) == 0)
            {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                   printf("Current working dir: %s\n", cwd);

                   n = write(newsockfd, cwd, strlen(cwd) + 1);

                   if (n < 0) error("ERROR writing to socket");
                }
                else
                   perror("getcwd() error");
            }
            /*** LS COMMAND *********************/
            else if (strncmp(buffer, "ls", 2) == 0)
            {
                DIR *d;
                struct dirent *dir;
                d = opendir(".");
                if (d)
                {
                    while ((dir = readdir(d)) != NULL)
                    {
                      printf("%s\n", dir->d_name);
                    }
                    closedir(d);
                }
            }
            else {
                printf("Command not found.\n");
                n = write(newsockfd,"WRONG COMMAND", strlen("WRONG COMMAND") + 1);
                if (n < 0) error("ERROR writing to socket");
            }
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}