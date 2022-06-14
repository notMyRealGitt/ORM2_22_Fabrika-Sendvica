// Subscriber
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>         // Needed for strtok()
#include <sys/types.h>    // Needed for system defined identifiers.
#include <netinet/in.h>   // Needed for internet address structure.
#include <sys/socket.h>   // Needed for socket(), bind(), etc...
#include <arpa/inet.h>    // Needed for inet_ntoa()
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>

//----- Defines -------------------------------------------------------------
#define PORT_NUM         4448             // Port number used
#define GROUP_ADDR "224.0.0.0"            // Address of the multicast group

using namespace std;

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc == 0) {
		printf("Subcribing to topic -t factory/prilog/+ \n ");
		mosquitto_subscribe(mosq, NULL, "factory/prilog/+", 0);
		printf("Subcribing to topic -t factory/pice/+ \n ");
		mosquitto_subscribe(mosq, NULL, "factory/pice/+", 0);
		mosquitto_subscribe(mosq, NULL, "factory/newline", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	printf("Topic:   %s\n", msg->topic);
	printf("payload: %s\n", msg->payload);
}

int main()
{
	//first get brokers ip address
	unsigned int         multi_server_sock; // Multicast socket descriptor
	struct ip_mreq       mreq;              // Multicast group structure
	struct sockaddr_in   client_addr;       // Client Internet address
	unsigned int         addr_len;          // Internet address length
	char        		 buffer[256];       // Datagram buffer
	int 				 retcode;           // Return code

		// Create a multicast socket and fill-in multicast address information
	multi_server_sock=socket(AF_INET, SOCK_DGRAM,0);
	mreq.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);
	mreq.imr_interface.s_addr = INADDR_ANY;

	// Create client address information and bind the multicast socket
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(PORT_NUM);

	retcode = bind(multi_server_sock,(struct sockaddr *)&client_addr,
					sizeof(struct sockaddr));
	if (retcode < 0)
	{
		printf("*** ERROR - bind() failed with retcode = %d \n", retcode);
		return 1;
	}

	// Have the multicast socket join the multicast group
	retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				(char*)&mreq, sizeof(mreq)) ;
	if (retcode < 0)
	{
		printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
		return 1;
	}
	addr_len = sizeof(client_addr);

	char * recv_type;
	char * recv_id;
	char * recv_address;
	char * recv_msg;
	char * type;
	
	 while(1)
	{
		// Receive a datagram from the multicast server
		if( (retcode = recvfrom(multi_server_sock, buffer, sizeof(buffer), 0,
		(struct sockaddr *)&client_addr, &addr_len)) < 0){
			printf("*** ERROR - recvfrom() failed \n");
			return 1;
		}
		if(sizeof(buffer)>2){
			printf("%s",buffer);
			break;
		}

	}
	close(multi_server_sock);



	struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL) {
		printf("Failed to create client instance.\n");
		return 1;
	}
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, buffer, 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();

}
