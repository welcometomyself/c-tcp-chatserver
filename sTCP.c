/*
 * File:   sUDP.c
 * Author: theconquerer
 *
 * Created on May 5, 2009, 12:53 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#define MAXLEN_MESSAGE  1024
#define MAX_CONNECTIONS 20
#define PORT_NUMBER  50000

static void * threading_socket(void *);
void process(int);

pthread_t th;
int currentconnections = 0;
int running = 1;
char** connectedClientsIP;
char** connectedClientsName;

static void *threading_socket(void *arg) {
  pthread_detach(pthread_self());
  currentconnections++;
  printf ("Connection #%d opened (%d of %d currently open)\n",(int) arg, currentconnections, MAX_CONNECTIONS);
  process((int) arg);
  close((int) arg);
  currentconnections--;
  printf ("Connection #%d closed (%d of %d currently open)\n",(int) arg, currentconnections, MAX_CONNECTIONS);
  return NULL;
}

void process(int connfd) {
  ssize_t j;
  char buffer[MAXLEN_MESSAGE] = {0};
  char clientsList[1024] = {0};
  char * p_clientsList = (char *) & clientsList;
  char * clr;
  int infocut;
  struct sockaddr_in in_addr;
  socklen_t in_addr_len = sizeof (struct sockaddr_in);
  FILE * in = fdopen (connfd, "r+");
  int i;
  int clientCount;

  while (running == 1) {
      j = 0;
      
      if (!fgets(buffer, 26, in)) {
        fprintf(stdout, "EOF on connection #%d\n", connfd);
        connectedClientsIP[connfd] = (char*) malloc (15*sizeof (char));
        connectedClientsName[connfd] = (char*) malloc (15*sizeof (char));
        break;
      }
      //printf("IN: %s", buffer);
      
      //clr = strchr(tempbuffer, '\r');
      //*clr = 0;

      printf ("IN(%d):%s", connfd, buffer);

      if (strncasecmp(buffer, "BYE", 3) == 0) {
        if (strlen(connectedClientsName[connfd]) > 0) {
            connectedClientsIP[connfd] = (char*) malloc (15*sizeof (char));
            connectedClientsName[connfd] = (char*) malloc (15*sizeof (char));
            fputs("BYE\n", in);
            fprintf(stdout, "OUT(%d):BYE\n", connfd);
        } else {
            fputs("ERROR Not registered\n", in);
            fprintf(stdout, "OUT(%d):ERROR Not registered\n", connfd);
        }
        break;
      } else if (strncasecmp(buffer, "NEW", 3) == 0) {
        if (strlen(connectedClientsName[connfd]) == 0) {
            getpeername (connfd, (struct sockaddr *) &in_addr, (socklen_t *)&in_addr_len);
            fputs("New Client: ", stdout);
            connectedClientsIP[connfd] = inet_ntoa(in_addr.sin_addr);
            clr = (char *) 0;

            // Workaround for telnet -.-
            clr = strchr(buffer, '\r');
            infocut = 4;

            if (clr == (char *)0) {
                clr = strchr(buffer, '\0');
                infocut = 5;
            }

            strncpy(connectedClientsName[connfd], buffer+4, (int)(clr-(char *)&buffer)-infocut);

            printf("%s (%s)\n", connectedClientsName[connfd], connectedClientsIP[connfd]);

            fputs("OK\n", in);
            fprintf(stdout, "OUT(%d):OK\n", connfd);
        } else {
            fprintf(in, "ERROR Already connected as %s\n", connectedClientsName[connfd]);
            fprintf(stdout, "OUT(%d):ERROR Already connected as %s\n", connfd, connectedClientsName[connfd]);
            connectedClientsIP[connfd] = (char*) malloc (15*sizeof (char));
            connectedClientsName[connfd] = (char*) malloc (15*sizeof (char));
            break;
        }
      } else if (strncasecmp(buffer, "INFO", 4) == 0) {
        if (strlen(connectedClientsName[connfd]) > 0) {
            clientCount = 0;
            * clientsList = '\0';
            for (i=0;i<MAX_CONNECTIONS;i++) {
              if (strlen(connectedClientsName[i]) > 0) {
                clientCount++;
                sprintf(clientsList, "%s %s %s", clientsList, connectedClientsIP[i], connectedClientsName[i]);
              }
            }
            fprintf(in ,"LIST %d%s\n", clientCount, clientsList);
            fprintf(stdout ,"OUT(%d):LIST %d %s\n", connfd, clientCount, clientsList);
        } else {
            fputs("ERROR Not registered\n", in);
            fprintf(stdout, "OUT(%d):ERROR Not registered\n", connfd);
            connectedClientsIP[connfd] = (char*) malloc (15*sizeof (char));
            connectedClientsName[connfd] = (char*) malloc (15*sizeof (char));
            break;
        }
      } else if (strncasecmp(buffer, "\n", 1) != 0) {
        connectedClientsIP[connfd] = (char*) malloc (15*sizeof (char));
        connectedClientsName[connfd] = (char*) malloc (15*sizeof (char));
        fputs("ERROR Unknown command\n", in);
        fprintf(stdout, "OUT(%d):ERROR Unknown command\n", connfd);
        break;
      }

/*
      for (i=0;i<MAXLEN_MESSAGE;i++) {
        buffer[i] = 0;
      }
*/
  }
  fclose(in);
  close(connfd);
  return;
}

int main(void) {
  int sockfd, connfd;
  struct sockaddr_in address;
  socklen_t address_len = sizeof (struct sockaddr_in);
  const int y = 1;
  int i;

  connectedClientsIP = (char**) malloc (MAX_CONNECTIONS*sizeof (char*));
  connectedClientsName = (char**) malloc (MAX_CONNECTIONS*sizeof (char*));
  for (i=0;i<MAX_CONNECTIONS;i++) {
    connectedClientsIP[i] = (char*) malloc (15*sizeof (char));
    connectedClientsName[i] = (char*) malloc (15*sizeof (char));
  }

  printf ("Creating socket...\t");
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf ("[FAIL]\nError: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf ("[ OK ]\n");

  address.sin_family = AF_INET;
  address.sin_port = htons(PORT_NUMBER);
  memset(&address.sin_addr, 0, sizeof (address.sin_addr));

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof (int));

  printf("Binding Address...\t");
  if (bind(sockfd,
      (struct sockaddr *) & address,
      sizeof (address))) {
    printf ("[FAIL]\nError: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf ("[ OK ]\nServer ready and waiting\n");

  if (listen(sockfd, 5)) {
    printf ("Can't listen() ...(%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  while (1) {
    if (currentconnections <= MAX_CONNECTIONS) {
        connfd = accept(sockfd,
                        (struct sockaddr *) & address,
                        &address_len);
        if (connfd < 0) {
          if (errno == EINTR)
            continue;
          else {
            printf ("Failed to accept connection ...\n");
            exit(EXIT_FAILURE);
          }
        }

        pthread_create(&th,
            NULL,
            &threading_socket,
            (void *) connfd);
    }
  }
  exit(EXIT_SUCCESS);
}
