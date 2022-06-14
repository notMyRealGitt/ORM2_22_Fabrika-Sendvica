// Publisher
#include <stdio.h>
#include <mosquitto.h>
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
		printf("Subcribing to topic -t factory/gotov \n ");
		mosquitto_subscribe(mosq, NULL, "factory/gotov", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}

volatile int gotov = 0;
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	//printf("Topic:   %s\n", msg->topic);
	//printf("payload: %s\n", msg->payload);
	//string pld = msg->payload;
	//if( (pld).compare("gotov") == 0)
		gotov = 1;
	//else
	//	gotov = 0;
}


int main(int argc, int** argv) {
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


	int rc;
	struct mosquitto * mosq;

	mosquitto_lib_init();

	mosq = mosquitto_new("musterija", true, NULL);

	rc = mosquitto_connect(mosq, buffer, 1883, 60);
	if (rc != 0) {
		printf("Client could not connect to broker! Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	printf("We are now connected to the broker!\n");

	
	mosquitto_loop_start(mosq);
	
	while(1)
	{
		cout << flush;
		cout << "Biraj sendvic:"  				<< endl;
		cout << " -> (1) kecap, sir, salama" 	<< endl;
		cout << " -> (2) senf, salama" 			<< endl;
		cout << " -> (3) senf, salama, kobasica"<< endl;
		cout << " -> (4) majonez, sir, salama" 	<< endl;
		cout << " -> (5) sir, salama, kobasica" << endl;
		char sendv;
		cin >> sendv;
		
		cout << flush;
		cout << "Biraj pice:" 	<< endl;
		cout << " -> (1) voda" 	<< endl;
		cout << " -> (2) kafa" 	<< endl;
		cout << " -> (3) pivo" 	<< endl;
		char sok;
		cin >> sok;
		cout << flush;
		
		string porudz = "";
		porudz += sendv;
		porudz += sok;
		cout << "DEBUG" << " " << sendv << " " << sok << " " << porudz << endl;
		
		mosquitto_publish(mosq, NULL, "factory/recept", 4, porudz.c_str(), 0, false);
		
		while(gotov != 1); // cekamo da se zavrsi sendvic
		cout << "Nardzubina je gotova! Naruci opet?" << endl;
		gotov = 0;
	}
	
	printf("Press Enter to quit...\n");
	getchar();
	mosquitto_loop_stop(mosq, true);

	//mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}
