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
#include <fstream>

//----- Defines -------------------------------------------------------------
#define PORT_NUM         4448             // Port number used
#define GROUP_ADDR "224.0.0.0"            // Address of the multicast group

using namespace std;

string readFunc(string filename)
{
	string val;
	cout << flush;
	cout << filename << " ";
	
	ifstream infile;
	infile.open(filename);
	
	getline(infile, val);
	//int val = stoi(tmp);
	cout << val << endl;
	
	infile.close();
	
	return val;
}

string oldSir = "";
string oldKecap = "";
string oldSalama = "";
string oldKobasica = "";
string oldSenf = "";
string oldMajonez = "";

string oldVoda =  "";
string oldKafa =  "";
string oldPivo =  "";
void readF(mosquitto * mosq)
{
	cout << flush;
	cout << "pocinje" << endl;
	
	string amtSir = readFunc("Sir.txt");
	string amtKecap = readFunc("Kecap.txt");
	string amtSalama = readFunc("Salama.txt");
	string amtKobasica = readFunc("Kobasica.txt");
	string amtSenf = readFunc("Senf.txt");
	string amtMajonez = readFunc("Majonez.txt");
	
	string amtVoda = readFunc("Voda.txt");
	string amtKafa = readFunc("Kafa.txt");
	string amtPivo = readFunc("Pivo.txt");
	
	mosquitto_publish(mosq, NULL, "factory/newline", 4, "\n\n", 0, false);
	
	mosquitto_publish(mosq, NULL, "factory/prilog/sir", 12, (oldSir + " -> " + amtSir).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/prilog/kecap", 12, (oldKecap + " -> " + amtKecap).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/prilog/salama", 12, (oldSalama + " -> " + amtSalama).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/prilog/kobasica", 12, (oldKobasica + " -> " + amtKobasica).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/prilog/senf", 12, (oldSenf + " -> " + amtSenf).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/prilog/majonez", 12, (oldMajonez + " -> " + amtMajonez).c_str(), 0, false);
	
	mosquitto_publish(mosq, NULL, "factory/pice/voda", 12, (oldVoda + " -> " + amtVoda).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/pice/kafa", 12, (oldKafa + " -> " + amtKafa).c_str(), 0, false);
	mosquitto_publish(mosq, NULL, "factory/pice/pivo", 12, (oldPivo + " -> " + amtPivo).c_str(), 0, false);
	
	
	oldSir = amtSir;
	oldKecap = amtKecap;
	oldSalama = amtSalama;
	oldKobasica = amtKobasica;
	oldSenf = amtSenf;
	oldMajonez = amtMajonez;
	
	oldVoda = amtVoda;
	oldKafa = amtKafa;
	oldPivo = amtPivo;
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

	mosq = mosquitto_new("senzori", true, NULL);

	rc = mosquitto_connect(mosq, buffer, 1883, 60);
	if (rc != 0) {
		printf("Client could not connect to broker! Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}

	printf("We are now connected to the broker!\n");

	
	
	cout << flush;
	cout << "Sacekajte par sekundi . . ." << endl;
	sleep(6);
	mosquitto_loop_start(mosq);
	while(1){
		readF(mosq);
		sleep(10);
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
