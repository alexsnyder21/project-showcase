// Alex Snyder
// CSC 328 Project 5
// Fall 2023
// Due October 31st 11PM
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int really_read(int s, char* buf, int len) {
    int total = 0;
    int bytes_left = len;
    int n;
    while (total < len) {
        n = read(s, buf+total, bytes_left);
        if (n == -1 || n == 0) { break; }
        total += n;
        bytes_left -= n;
    }
    return n == -1 ? -1 : total;
}

/* Function: read_packets
   Description: Reads 2 bytes from the socket into the short int packet_length,
   which represents the length the incoming packet. It processes and prints the word packets
   repeatedly until there are no more left and the server closes connection
   Parameters: sockfd: client socket's file descriptor
   Return Value: N/A
*/
void read_packets(int sockfd) {
	for(;;){
	  unsigned short word_length;
	  int num_bytes = really_read(sockfd, (char *)&word_length, 2);
	  if (num_bytes == 0) { break; }
	  word_length = ntohs(word_length);
	  char buff[word_length+1];
	  num_bytes = really_read(sockfd, buff, word_length);
	  buff[word_length] = '\0';
	  printf("%s\n", buff);
	}
}	

int main(int argc, char** argv) {

  //accept two command line arguments: host and port in that order.
  if (argc != 3) {
	  fprintf(stderr, "Usage: <host> <port>\n");
	  exit(-1);
  }
  const char* host = argv[1];
  const char* port = argv[2];
  
  struct addrinfo hints, *res;
  
  //obtain addresses
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  
  if (getaddrinfo(host, port, &hints, &res) != 0){
    perror("getaddrinfo");
	exit(-1);
  }
  
  //create a socket
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1){
    perror("socket");
	exit(-1);
  }

  //connect the socket to the server
  if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
	perror("connect");
	close(sockfd);
	exit(-1);
  }

  //read word packets until the server closes the connection. 
  //for each word packet print the associated string to standard out followed by a newline.
  read_packets(sockfd);

  //close the socket file descriptor and clean up
  close(sockfd);
  freeaddrinfo(res);

  return 0;
}