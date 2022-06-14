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
#include <fstream>

//----- Defines -------------------------------------------------------------
#define PORT_NUM         4448             // Port number used
#define GROUP_ADDR "224.0.0.0"            // Address of the multicast group

using namespace std;

#define DECREMENT_SIR      (amtSir < 4 ? (amtSir = 100, printf("Sir zamenjen\n")) : amtSir-=4)
#define DECREMENT_KECAP    (amtKecap < 3 ? (amtKecap = 100, printf("Kecap zamenjen\n")) : amtKecap-=3)
#define DECREMENT_SALAMA   (amtSalama < 3 ? (amtSalama = 100, printf("Salama zamenjena\n")) : amtSalama-=3)
#define DECREMENT_KOBASICA (amtKobasica < 2 ? (amtKobasica = 100, printf("Kobasica zamenjena\n")) : amtKobasica-=2)
#define DECREMENT_SENF     (amtSenf < 1 ? (amtSenf = 100, printf("Senf zamenjen\n")) : amtSenf-=1)
#define DECREMENT_MAJONEZ  (amtMajonez < 5 ? (amtMajonez = 100, printf("Majonez zamenjen\n")) : amtMajonez-=5)

#define DECREMENT_VODA (amtVoda < 2 ? (amtVoda = 100, printf("Voda zamenjena\n")) : amtVoda-=2)
#define DECREMENT_KAFA (amtKafa < 1 ? (amtKafa = 100, printf("Kafa zamenjena\n")) : amtKafa-=1)
#define DECREMENT_PIVO (amtPivo < 2 ? (amtPivo = 100, printf("Pivo zamenjeno\n")) : amtPivo-=2)

int amtSir = 99;
int amtKecap = 98;
int amtSalama = 100;
int amtKobasica = 100;
int amtSenf = 77;
int amtMajonez = 6;

int amtVoda = 100;
int amtKafa = 50;
int amtPivo = 99;

volatile int sandwichDone = 0;  // 0 = nije gotov, 1 = gotov, -1 = greska
volatile int drinkDone = 0;      // 0 = nije gotov, 1 = gotov, -1 = greska

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc == 0) {
		printf("Subcribing to topic -t factory/recept \n ");
		mosquitto_subscribe(mosq, NULL, "factory/recept", 0);
	} else {
		mosquitto_disconnect(mosq);
	}
}
/*
-> (1) kecap, sir, salama
-> (2) senf, salama
-> (3) senf, salama, kobasica
-> (4) majonez, sir, salama
-> (5) sir, salama, kobasica

-> (1) voda
-> (2) kafa
-> (3) pivo
*/
void make_order(char s, char d)
{
	int si = s - '0';
	switch(si)
	{
		case 1:
		//cout << "DEBUG SWITCH s (1) " << amtKecap << " " << amtSir << " " << amtSalama << endl;
		DECREMENT_KECAP;
		DECREMENT_SIR;
		DECREMENT_SALAMA;
		//cout << "DEBUG SWITCH s (2)" << amtKecap << " " << amtSir << " " << amtSalama << endl;
		sandwichDone = 1;
		break;
		
		case 2:
		DECREMENT_SENF;
		DECREMENT_SALAMA;
		sandwichDone = 1;	
		break;
		
		case 3:		
		DECREMENT_SENF;
		DECREMENT_SALAMA;
		DECREMENT_KOBASICA;
		sandwichDone = 1;
		break;
		
		case 4:		
		DECREMENT_MAJONEZ;
		DECREMENT_SIR;
		DECREMENT_SALAMA;
		sandwichDone = 1;
		break;
		
		case 5:
		DECREMENT_SIR;
		DECREMENT_SALAMA;
		DECREMENT_KOBASICA;
		sandwichDone = 1;
		break;
		
		default:
			cout << "Nepostojeci izbor priloga: " << si << endl;
			sandwichDone = -1;
	}
	int di = d - '0';
	switch(di)
	{
		case 1:
		DECREMENT_VODA;
		drinkDone = 1;
		break;
		
		case 2:
		DECREMENT_KAFA;
		drinkDone = 1;
		break;
		
		case 3:
		DECREMENT_PIVO;
		drinkDone = 1;
		break;
		
		default:
			cout << "Nepostojeci izbor soka: " << di << endl;
			drinkDone = 0;
	}
	
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	printf("Topic:   %s\n", msg->topic);
	printf("payload: %s\n", msg->payload);
	
	char *s = (char*)msg->payload;
	
	//char** pld;
	//pld = (char **)&(msg->payload);
	cout << "DEBUG pld" << msg->payload << " " << *s << " " << s[0] << " " << s[1] << endl;
	make_order(s[0], s[1]);
}

void writeFunc(string filename, int val)
{
	//cout << flush;
	//cout << filename << " ";
	ofstream outfile;
	outfile.open(filename);
	outfile << val;
	outfile.close();	
}

void writeF()
{
	writeFunc("Sir.txt", amtSir);
	writeFunc("Kecap.txt", amtKecap);
	writeFunc("Salama.txt", amtSalama);
	writeFunc("Kobasica.txt", amtKobasica);
	writeFunc("Senf.txt", amtSenf);
	writeFunc("Majonez.txt", amtMajonez);
	
	writeFunc("Voda.txt", amtVoda);
	writeFunc("Kafa.txt", amtKafa);
	writeFunc("Pivo.txt", amtPivo);
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

	//mosquitto_loop_forever(mosq, -1, 1);
	
	mosquitto_loop_start(mosq);
	
	while(1)
	{
		while(sandwichDone != 1 && drinkDone != 1);
		sandwichDone = 0;
		drinkDone = 0;
		writeF();
		mosquitto_publish(mosq, NULL, "factory/gotov", 6, "gotov", 0, false);
	}
	
	printf("Press Enter to quit...\n");
	getchar();
	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();

}
