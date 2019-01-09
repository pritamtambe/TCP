//"This is a chnage"
#include <stdio.h>
#include <stdint.h>		
#include <netinet/in.h>		//structure sockaddr_in
#include <string.h> 		//bzero
#include <stdlib.h>		//EXIT_SUCCESS EXIT_FAILURE
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#define UNIT_ID			0x02
//Address of registers
#define NO_OF_READABLE_COILS	100
#define READ_COILS_START_ADD	((UNIT_ID*1000) + (001))
#define READ_COILS_END_ADD	(READ_COILS_START_ADD+NO_OF_READABLE_COILS)
//Max counts
#define MAX_NO_OF_READ_COILS	NO_OF_READABLE_COILS

#define LEN_OF_MBAP_HEADER	0x06
//Indexes
#define INDEX_OF_MODBUS_CMD_LEN		0x05
#define INDEX_OF_UNIT_ID		0x06
#define INDEX_OF_FUNCTION		0x07
#define INDEX_OF_START_ADDRESS_MSB	0x08
#define INDEX_OF_START_ADDRESS_LSB	0x09
#define INDEX_OF_NO_OF_COILS_MSB	0x0A
#define INDEX_OF_NO_OF_COILS_LSB	0x0B	
//Functions
#define READ_COILS			0x01

static char coil_registers[MAX_NO_OF_READ_COILS]={0};
static int read_coils(char * pc_data,int length_of_data,int client_socket_fd){
	int start_address_from_modbus_cmd = (((unsigned char)pc_data[INDEX_OF_START_ADDRESS_MSB]<<8)|(unsigned char)pc_data[INDEX_OF_START_ADDRESS_LSB])&0xFFFF;
	int number_of_coils_to_read_from_modbus_cmd = (((unsigned char)pc_data[INDEX_OF_NO_OF_COILS_MSB]<<8)|(unsigned char)pc_data[INDEX_OF_NO_OF_COILS_LSB])&0xFFFF;
	int end_addres_from_modbus_cmd = start_address_from_modbus_cmd + number_of_coils_to_read_from_modbus_cmd;
	int initial_addr_of_coil_register = 0;
	char pc_response_from_slave[LEN_OF_MBAP_HEADER + MAX_NO_OF_READ_COILS];
	//fprintf(stdout,"pc_data[INDEX_OF_START_ADDRESS_MSB] : %d\n",(unsigned char)pc_data[INDEX_OF_START_ADDRESS_MSB]);
	//fprintf(stdout,"pc_data[INDEX_OF_START_ADDRESS_LSB] : %d\n",(unsigned char)pc_data[INDEX_OF_START_ADDRESS_LSB]);

	if(MAX_NO_OF_READ_COILS < number_of_coils_to_read_from_modbus_cmd){
		fprintf(stderr,"%s\n","requested number coils are more");
	}

	//printf("start_address_from_modbus_cmd : %d\n",start_address_from_modbus_cmd);
	//printf("READ_COILS_START_ADD : %d\n",READ_COILS_START_ADD);
	//printf("READ_COILS_END_ADD : %d\n",READ_COILS_END_ADD);
	//printf("end_addres_from_modbus_cmd : %d\n",end_addres_from_modbus_cmd);
	char test[] = {0x00,0x00,0x00,0x00,0x00,0x01,0x00};
	if(start_address_from_modbus_cmd >= READ_COILS_START_ADD && end_addres_from_modbus_cmd <= READ_COILS_END_ADD){
		initial_addr_of_coil_register = READ_COILS_START_ADD - start_address_from_modbus_cmd;
		int i = 0,j = 0;
		for( i = 0 ; i < LEN_OF_MBAP_HEADER+2 ; i++ ){
			pc_response_from_slave[j++] = pc_data[i];
			test[i] = pc_data[i];
			fprintf(stdout,"%02X ",pc_response_from_slave[i]);
		}
	/*	//pc_response_from_slave[j++] = 3;
		for( i = 0 ; i < number_of_coils_to_read_from_modbus_cmd ; i++ ){
			if(coil_registers[i] == 1){
				pc_response_from_slave[j] |= (1<<i);  
			}else{
				pc_response_from_slave[j] &= ~(1<<i);
			}
			if((i%7) == 0 && i < number_of_coils_to_read_from_modbus_cmd){
				j++;
			}
		}  */
		for( i = initial_addr_of_coil_register ; i < number_of_coils_to_read_from_modbus_cmd ; i++ ){
			pc_response_from_slave[j++] = coil_registers[i];
		}
		fprintf(stdout,"j = %d\n",j);
		pc_response_from_slave[INDEX_OF_MODBUS_CMD_LEN] = j - LEN_OF_MBAP_HEADER;  		

		int n = write(client_socket_fd,pc_response_from_slave,11);
		//int n = write(client_socket_fd,test,6);
		fprintf(stdout,"%s : ","data sent to server");
		for( i = 0 ; i <= n ; i++ ){
			fprintf(stdout,"%02X ",(unsigned char)pc_response_from_slave[i]);
		}
	}else{
		fprintf(stderr,"%s\n","address is invalid");
	}
	
}
void process_data(char * pc_data,int length_of_data,int client_fd){
	int i = 0 ;
	fprintf(stdout,"\nrequest string : ");
	for(i = 0 ; i < length_of_data ; i++){
		fprintf(stdout,"%02X ",(unsigned char)pc_data[i]);
	}
	fprintf(stdout,"\n");
	if(UNIT_ID != pc_data[INDEX_OF_UNIT_ID]){
		fprintf(stderr,"%s\n","unit id mismatch\n");
	}
	int length_of_function_data_in_modbus_cmd = pc_data[INDEX_OF_MODBUS_CMD_LEN];
	int total_len_of_modbus_cmd = LEN_OF_MBAP_HEADER + length_of_function_data_in_modbus_cmd;

	if(total_len_of_modbus_cmd != length_of_data){
		fprintf(stderr,"%s\n","packet length is appropriate");
	}	
	int function_code = pc_data[INDEX_OF_FUNCTION];
	switch(function_code){
		case READ_COILS :
			fprintf(stdout,"%s\n","read coils\n");
			read_coils(pc_data,length_of_data,client_fd);
			break;
		default:
			break;
	}

}
int main (void){
	char buffer[512];
	bzero(buffer,512);
	int socket_fd = -1,client_socket_fd,n = 0;
	struct sockaddr_in serv_addr,cli_addr;
	bzero((char *)&serv_addr,sizeof(serv_addr));
	bzero((char *)&cli_addr,sizeof(cli_addr));
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd < 0){
		fprintf(stderr,"%s:%s\n","server socket creation failed",strerror(errno));
		exit(EXIT_FAILURE);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(502);
	if(bind(socket_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
		fprintf(stderr,"%s:%s\n","server socket bind failed",strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(listen(socket_fd,100) < 0){
		fprintf(stderr,"%s:%s\n","server socket failed to listen",strerror(errno));
		exit(EXIT_FAILURE);
	}
	int client_addr_len = sizeof(cli_addr);

	fprintf(stdout,"%d\n",__LINE__);
	while(1){
		client_socket_fd = accept(socket_fd,(struct sockaddr *)&cli_addr,&client_addr_len);
		if(client_socket_fd < 0 ){
			fprintf(stderr,"%s:%s\n","server socket failed to accept",strerror(errno));
			exit(EXIT_FAILURE);
		}
		while(n >= 0){
			n = read(client_socket_fd,buffer,512);
			process_data(buffer,n,client_socket_fd);
			/*if(close(client_socket_fd) < 0){
			  fprintf(stderr,"%s:%s\n","client close failed",strerror(errno));

			  }*/
		}
		/*
		   char * hello = "Hello form server";
		   n = write(client_socket_fd,hello,strlen(hello));
		   if(n < 0){
		   fprintf(stderr,"%s:%s\n","server socket failed  to write on client",strerror(errno));
		   exit(EXIT_FAILURE);
		   }*/

	}
	exit(EXIT_SUCCESS);
}
