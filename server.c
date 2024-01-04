// Alex Snyder
// CSC 328 Project 6
// Fall 2023
// Due November 7th 11PM
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>

//list of 20 random words
char* word_list[] = { 
  "hello", "apple", "bat", "bird", "bed", "strawberry",
  "thirsty", "water", "window", "sword", "biology", "tree", 
  "house", "bear", "strong", "cheeseburger", "math", "game", "computer", "python" };
  
/* Function: send_all
   Description: Sends all of the needed bytes to the client by calling the send function multiple times 
   in a while loop and keeping track of the total bytes sent and how many are left to send
   Parameters: new_fd: client socket's file descriptor
			   word: pointer the the word array that is being sent to the client
			   len: the length of the word in bytes
   Return Value: -1 on failurem, 0 on success
*/
int send_all(int new_fd, char *word, unsigned short len) {
  int total = 0;        // how many bytes we've sent
  int bytesleft = len; // how many we have left to send
  int n;
  while(total < len) {
    if ((n = send(new_fd, word+total, bytesleft, 0)) == -1) {
	  perror("send");
	  break;
	}
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
  len = total; // return number actually sent here
  return n==-1?-1:0; // return -1 on failure, 0 on success
}

/* Function: create_packets
   Description: generates a random amount of packets to create and randomly selects from an array of words
   to put into the packet. It finds the length of the chosen word and stores it as an unsigned short into an array
   that holds both the length of the word and the word itself. The array is then passed into the send_all function and sent to the client
   Parameters: new_fd: client socket's file descriptor
   Return Value: N/A
*/
void create_packets(int new_fd) {
  srand(time(NULL));
  //generate the amount of packets to send
  int num_packets = rand() % 10 + 1;
  for (int i = 0; i < num_packets; i++) {
    int idx = rand() % (sizeof(word_list) / sizeof(word_list[0])); //get random index from the word list
	char *word = word_list[idx];
	uint16_t len = strlen(word);
	uint16_t new_len = len;
	char word_packet[len+2];
	len = htons(len); //convert to network byte order
	memcpy(word_packet, &len, sizeof(len)); //copy word length into word packet array
	memcpy(word_packet+2, word, strlen(word)); //copy word into word packet array
	//send length and word
	if (send_all(new_fd, word_packet, new_len+2) == -1) {
	  perror("send_all");
	  break;
	}
  }
}

int main(int argc, char** argv) {
  //accept one command line argument: the port number
  if (argc != 2) {
	  fprintf(stderr, "Usage: <port>\n");
	  exit(-1);
  }
  const char* port = argv[1];

  //create a socket
  int sockfd, new_fd;
  struct addrinfo hints, *res;
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  char client_ip[INET_ADDRSTRLEN];
 
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo("127.0.0.1", port, &hints, &res) != 0){
    perror("getaddrinfo");
	exit(-1);
  }
  if ((sockfd = socket(res->ai_family, res->ai_socktype, 0)) == -1){
    perror("socket");
	exit(-1);
  }
  //bind the socket
  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1){
	perror("bind");
	exit(-1);
  }
  //make the socket passive with listen
  if (listen(sockfd, 10) == -1){
	perror("listen");
	exit(-1);
  }
  
  printf("Server started on 127.0.0.1\n");
  //loop forever
  while (1){
	printf("Waiting for connections...\n");
    //get a client connection file descriptor with accept
	addr_size = sizeof their_addr;
	if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1){
		perror("accept");
		break;
	}
	printf("after connect\n");
	//get client ip address and port number
	if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&their_addr)->sin_addr), client_ip, sizeof(client_ip)) == NULL){
		perror("inet_ntop");
		break;
	}
	unsigned int client_port = ntohs(((struct sockaddr_in *)&their_addr)->sin_port);
	printf("Got a connection from %s:%d\n", client_ip, client_port);
    //generate 1 to 10 random word packets
	//send the word packets to the client
	create_packets(new_fd);
	//close the socket file descriptor that was created from accept
	if (close(new_fd) == -1){
		perror("close");
		break;
	}
  }
  return 0;
}
