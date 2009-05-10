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
#define MAXLEN_MESSAGE  122
#define MAX_CONNECTIONS 20
#define PORT_NUMBER  50000

static void * threading_socket( void *);
static void process (int);

pthread_t th;
int currentconnections = 0;
int running = 1;
char** connectedClientsIP, connectedClientsName;

static void *threading_socket (void *arg) {
  pthread_detach (pthread_self ());
  process ((int) arg);
  close ((int) arg);
  printf("Connection closed");
  //currencconnections--;
  return NULL;
}

static void process (int connfd) {
  ssize_t j, n;
  char buffer[MAXLEN_MESSAGE];

  printf ("Data received on connection %d\n", connfd);

  //while (running == 1) {
    j = 0;
    while ((n = read (connfd, &buffer[j], 1)) > 0) {
    if (buffer[j] == '\n') {
        buffer[j] = 0;
        break;
    }
    j++;
    }

    printf("\t [DONE]\n");
    if (n < 0) {
      printf ("Error while read() ...\n");
      exit (EXIT_FAILURE);
    }

/*
      regex_t test;
      test.re_magic = 0;
      test.re_nsub = 1;
      test.re_endp = '\0';

      if (puffer == "BYE") {
        // send BYE to client
        // remove client and ip from client list
        printf("%s requested disconnect.\n");
        break;
      }
      else if (regcomp(&test, puffer, 1)) {
        regexec(&test, puffer, strlen(&puffer), regmatch_t[] _match, 1);
        // add name and ip to client list
        // send OK to client
        printf("%s connected as %s with ip %s.\n");
        continue;
      }
      else if (//INFO) {
        // if client is in clients list
            // send LIST COUNT <IP NAME>,... to client
        printf("Sent list of clients to %s.\n");
        continue;
      }
      else {
        // send ERROR WRONG_FORMAT to client
        // remove client from clients list if present
        printf("Error in clients %s message, closing connection ...\n");
        break;
      }
*/
      printf ("Got \"%s\"", buffer);
  //}
/*
  strcpy (path_file, getenv ("HOME"));
  strcat (path_file, "/tmp/");
  strcat (path_file, puffer);
  printf ("%s\n", path_file);

  // Datei zum lesen öffnen
  if ((fd=open(path_file,O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
    printf ("... kann %s nicht öffnen (%s)\n",
       puffer, strerror(errno));
    close (connfd);
  }
  // Datei aus dem Socket lesen und in lokale Kopie schreiben
  ngesamt = 0;
  while ((n = read (connfd, puffer, sizeof (puffer))) > 0) {
    if (write (fd, puffer, n) != n) {
        printf ("Fehler bei write() ...(%s)\n", strerror(errno));
        exit (EXIT_FAILURE);
    }
    ngesamt += n;
  }
  if (n < 0) {
      printf ("Fehler bei read() ...\n");
  }
  printf ("... beendet (%d Bytes)\n", ngesamt);
  close (fd);
 */
  close (connfd);
  return;
}

int main (void) {
  int sockfd, connfd;
  struct sockaddr_in address;
  socklen_t address_len = sizeof (struct sockaddr_in);
  const int y = 1;

  printf("Creating socket...\t");
  if ((sockfd = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
     printf ("[FAIL]\nError: %s\n", strerror(errno));
     exit (EXIT_FAILURE);
  }
  printf ("[ OK ]\n");

  address.sin_family = AF_INET;
  address.sin_port = htons (PORT_NUMBER);
  memset (&address.sin_addr, 0, sizeof (address.sin_addr));

  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));

  printf("Binding Address...\t");
  if (bind ( sockfd,
             (struct sockaddr *) &address,
             sizeof (address) ) ) {
     printf ("[FAIL]\nError: %s\n", strerror(errno));
     exit (EXIT_FAILURE);
  }
  printf ("[ OK ]\nServer ready and waiting\n");

  if (listen (sockfd, 5)) {
     printf ("Can't listen() ...(%s)\n", strerror(errno));
     exit (EXIT_FAILURE);
  }
  while (1) {
    //if (currentconnections <= MAX_CONNECTIONS) {
        connfd = accept ( sockfd,
                          (struct sockaddr *) &address,
                          &address_len );
        if (connfd < 0) {
          if (errno == EINTR)
             continue;
          else {
             printf ("Failed to accept connection ...\n");
             exit (EXIT_FAILURE);
          }
        }
        
        pthread_create(&th,
                        NULL,
                        &threading_socket,
                        connfd);
    //}
  }
  exit (EXIT_SUCCESS);
}