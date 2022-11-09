#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXLINE 4096   /*max text line length*/
#define SERV_PORT 3000 /*port*/
#define LISTENQ 8	   /*maximum number of client connections*/
#define READ 0
#define WRITE 1


/*IPC pipes with forking www.tutorialspoint.com/inter_process_communication/inter_process_communication_pipes.htm*/

int main(int argc, char **argv) {
	int listenfd, connfd, n;
	pid_t childpid;
	socklen_t clilen;
	char buf1[MAXLINE];
    	char buf2[MAXLINE];
	
	// Creating two pipes for sending/recieving
   	int pipefd_1[2]; // parent write open, read close
    	int pipefd_2[2]; // parent write close, read open

	if (pipe(pipefd_1) < 0) {
        	exit(1);
	}
	if (pipe(pipefd_2) < 0) {
                exit(1);
        }


	struct sockaddr_in cliaddr, servaddr;

	// Create a socket for the server
	// If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Problem in creating the socket");
		exit(2);
	}

	// preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	// bind the socket
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	// listen to the socket by creating a connection queue, then wait for clients
	listen(listenfd,  LISTENQ);

	printf("%s\n", "Server running...waiting for connections.");

	for (;;)
	{

		clilen = sizeof(cliaddr);
		// accept a connection
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

		printf("%s\n", "Received request...");

		if ((childpid = fork()) == 0)
		{ // if it’s 0, it’s child process

			printf("%s\n", "Child created for dealing with client requests");

			// close listening socket
			close(listenfd);

			while ((n = recv(connfd, buf1, MAXLINE, 0)) > 0)
			{
				// include pipe structure to allow multiple clients to connect
				printf("%s", "String received from and resent to the client:");
				puts(buf1);
				send(connfd, buf1, n, 0);
				
				// Closing ends of pipe that will not be used in child process
				close(pipefd_1[WRITE]); // write closed
                		close(pipefd_2[READ]); // read closed

                		//printf('Writing to %s buffer\n', buf2);
                		write(pipefd_2[WRITE], buf1, sizeof(buf1));

                		//printf('Reading from %s buffer\n', buf1);
                		read(pipefd_1[READ], buf2, sizeof(buf2));
				printf("CHILD : %s", buf2);
				sleep(1);

				memset(buf1, 0, MAXLINE);
			}

			// Fix this if statement
			/*if (n < 0)
				printf("%s\n", "Read error");
			exit(0);
			*/
			
			if (n < 0) 
			{
				printf("%s\n", "Read error");
				exit(0);
			}
		}

		else // parent process
		{
			for(;;) {
				// Closing ends of pipe that will not be used in parent process
				close(pipefd_1[READ]); // read closed
        			close(pipefd_2[WRITE]); // write closed

        			read(pipefd_2[READ], buf1, sizeof(buf1));	
				printf("Parent : %s\n", buf1);	
				sleep(1);

				write(pipefd_1[WRITE], buf1, sizeof(buf1));

				// Need to get a way for parent to break out, or change for to a while loop
				//
				/*
				if((buf1) == '0'){
					break;
				}
				*/
			}
		}
		// close socket of the server
		close(connfd);
	}
}
