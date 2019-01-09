#include <stdio.h>	//printf
#include <stdint.h>	//int types
#include <netinet/in.h>	//sockaddr_in structure
#include <stdlib.h>	//EXIT_FAILURE and EXIT_SUCCESS 
#include <errno.h>	//strerror and errno
#include <string.h>	//bzero

int main(void){
	char buffer[512];
	bzero(buffer,512);
	int socket_fd = -1,read_bytes,written_bytes;
	struct sockaddr_in serv_addr;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0 ){
		fprintf(stderr,"%s:%s\n","socket creation error",strerror(errno));
		exit(EXIT_FAILURE);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1") ;
	serv_addr.sin_port = htons(5000);
	if(connect(socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		fprintf(stderr,"%s:%s\n","socket connection failed",strerror(errno));
		exit(EXIT_FAILURE);
	}
	read_bytes = read(socket_fd,buffer,512);
	if(read_bytes < 0 ){
		fprintf(stderr,"%s:%s\n","socket read failed",strerror(errno));
		exit(EXIT_FAILURE);
	}	
	fprintf(stdout,"%s:%s\n","socket read",buffer);
	if(close(socket_fd) < 0){
		fprintf(stderr,"%s:%s\n","socket close failed",strerror(errno));
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
