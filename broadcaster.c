//===================================================== file = mserver.c =====
//=  A multicast server to send multicast datagrams                          =
//============================================================================
//=  Notes:                                                                  =
//=    1) This program sends datagrams one per second to a multicast group.  =
//=    2) Conditionally compiles for Winsock and BSD sockets by setting the  =
//=       initial #define to WIN or BSD as appropriate.                      =
//=    3) The multicast group address is GROUP_ADDR.                         =
//=--------------------------------------------------------------------------=
//=  Build: bcc32 mserver.c or cl mserver.c wsock32.lib for Winsock          =
//=         gcc mserver.c -lsocket -lnsl for BSD                             =
//=--------------------------------------------------------------------------=
//=  History:  JNS (07/11/02) - Genesis                                      =
//============================================================================
#define  BSD                // WIN for Winsock and BSD for BSD sockets

//----- Include files -------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <stdlib.h>         // Needed for memcpy() and itoa()
#ifdef WIN
  #include <winsock.h>      // Needed for all Windows stuff
#endif
#ifdef BSD
  #include <sys/types.h>    // Needed for system defined identifiers.
  #include <netinet/in.h>   // Needed for internet address structure.
  #include <sys/socket.h>   // Needed for socket(), bind(), etc...
  #include <arpa/inet.h>    // Needed for inet_ntoa()
  #include <fcntl.h>
  #include <netdb.h>
#endif

//----- Defines -------------------------------------------------------------
#define PORT_NUM         4448             // Port number used
#define GROUP_ADDR "224.0.0.0"            // Address of the multicast group

//===== Main program ========================================================
void main(void)
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  unsigned int         server_s;                // Server socket descriptor
  unsigned int         multi_server_sock;       // Multicast socket descriptor
 // struct sockaddr_in   client_addr;       // this. Internet address
  struct sockaddr_in   addr_dest;               // Multicast group address
  struct ip_mreq       mreq;                    // Multicast group descriptor
  unsigned char        TTL;                     // TTL for multicast packets
  struct in_addr       recv_ip_addr;            // Receive IP address
  unsigned int         addr_len;                // Internet address length
  unsigned char        buffer[256];             // Datagram buffer
  unsigned char        address_buffer[256];     // Address_Buffer
  int                  count;                   // Loop counter
  int                  retcode;                 // Return code
  int                  TYPE=1;
  int                  ID=1;
  unsigned char        MSG[128];

#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // Create a multicast socket
  multi_server_sock=socket(AF_INET, SOCK_DGRAM,0);

  // Create multicast group address information
  addr_dest.sin_family = AF_INET;
  addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
  addr_dest.sin_port = htons(PORT_NUM);

  // Create client address information and bind the multicast socket
  /*client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
  client_addr.sin_port = htons(PORT_NUM);*/

  // Set the TTL for the sends using a setsockopt()
  TTL = 1;
  retcode = setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL,
                       (char *)&TTL, sizeof(TTL));
  if (retcode < 0)
  {
    printf("*** ERROR - setsockopt() failed with retcode = %d \n", retcode);
    return;
  }

  // Set addr_len
  addr_len = sizeof(addr_dest);

  // Multicast the message forever with a period of 1 second
  count = 0;
  printf("*** Sending multicast datagrams to '%s' (port = %d) \n",
    GROUP_ADDR, PORT_NUM);

    system("rm adresa.txt");
    system("hostname -I >> adresa.txt");
    FILE *fp;
    char host_address[20];
    fp=fopen("adresa.txt","r");
    if(fp==NULL){
        printf("couldn't open file"); 
    }
    else{
        fgets(host_address,20,fp);
    }

  sprintf(MSG,"/factory/line/stopped");
  while(1)
  {
    // Increment loop count
    count++;

    // Build the message in the buffer
    //sprintf(address_buffer,"%s",inet_ntoa(addr_dest.sin_addr));
    sprintf(buffer,"%s", host_address);
    printf("%s",host_address);
    
    // Send buffer as a datagram to the multicast group
    sendto(multi_server_sock, buffer, sizeof(buffer), 0,
           (struct sockaddr*)&addr_dest, addr_len);
#ifdef WIN
    Sleep(1000);
#endif
#ifdef BSD
    sleep(1);
#endif
  }

  // Close and clean-up
#ifdef WIN
  closesocket(multi_server_sock);
  WSACleanup();
#endif
#ifdef BSD
  close(multi_server_sock);
#endif
}