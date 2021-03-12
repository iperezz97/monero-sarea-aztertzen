#include "nagusia.h"
#include "bzb.h"
//#include "eskatu1001.h" // agian kendu
#include "konprob1003.h"

struct in_addr dest_ip;
int s1;

void * konprobatu_ping(void *args) {
	sleep(2);
	// log fitxategia ireki
	FILE *log;
	log = fopen("log1003", "w");
	if(log == NULL) {
		printf("Error creating log file: %s\n", strerror(errno));
		fflush(stdout);
		exit(0);
	}
	fprintf(log, "Creating socket... Destination: %s\n", (char *)args);

	// Create a raw socket to send
        s1 = socket (AF_INET, SOCK_STREAM , IPPROTO_TCP); // s: socket descriptor
        if(s1 < 0)
        {
                fprintf (log, "Error creating socket. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		fflush(log);
                exit(0);
        }
	fprintf(log, "Socket descriptor: %d\n", s1);
        fflush(log);

	// Datagram to represent the packet
        unsigned char datagram[329];  // 1003 header bidaltzeko
        unsigned char datagram1[50];  // 1003 data bidaltzeko

	// Levin header
        struct levhdr1003 *lvh = (struct levhdr1003 *) (datagram);

        // Payload
        struct data1003 *dat = (struct data1003 *) (datagram1);

	// Dest addr struct
        struct sockaddr_in  dest;

	char *token = strtok(args, " "); 	// bi argumentuak jaso
        char *target = token; 			// destination addr
        int port = atoi(strtok(NULL, " "));	// port
	//printf("%s %d \n", target, port);

	if(inet_addr(target) == -1) // is not IP
        {
		fprintf(log, "IP desegokia");
		fflush(log);
		exit(1);
        }

	if(port == 0) {
		fprintf(log, "Portu desegokia");
		fflush(log);
		exit(1);

	}

        dest_ip.s_addr = inet_addr( target );
/*	while(egoera_lortu(dest_ip) != 1) {
		;
	}
*/
        memset (datagram, 0, 329);     /* zero out the buffer */
        memset (datagram1, 0, 266);    /* zero out the buffer */

	// Fill in the Levin Header (PING request)
        lvh->sign = 0x0101010101012101;     // 8 bytes
        lvh->length = 0x0a;                 // 8 bytes
        lvh->exp_resp = 0x01;               // 1 byte request
        lvh->comm_cod = htonl(0xeb030000);  // 4 bytes
        lvh->retn_cod = htonl(0x01000000);  // 4 bytes (request!!! DOKU+)
        lvh->reserved = htonl(0x01000000);  // 4 bytes
        lvh->endchars = htonl(0x01000000);  // 4 bytes

	unsigned char datt[10] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x00};
	memcpy(dat->d, datt, 10); // honela esleitu, bestela 0x00 interpretatzean bukatzen da

	int i;

	// helburuko nodoari buruzko informazioa bete
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
        dest.sin_port = htons( port );

	// 3-way handshake (SYN-ACK)
	if(connect(s1, (struct sockaddr *)&dest, sizeof(struct sockaddr)) < 0){
		fprintf(log, "Error: %s \n", strerror(errno));
		fflush(log);
		exit(1);
	}
	// kontuz... batzuk kolgatuta beste batzuk connection refused -> ezabatu nodoa...
	fprintf(log, "Connected.\n");
	fflush(log);
        // Send the Levin handshake request header packet
        int sizesend = sizeof(struct levhdr1003);
        if ( sendto (s1, datagram, sizeof(struct levhdr1003) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
		fprintf (log, "Error sending 1003 header. Error number: %d . Error message: %s \n" , errno , strerror(errno));
        	fflush(log);
	   	exit(0);
        }

        char * m = inet_ntoa(dest.sin_addr);
        fprintf(log, "1003 request header sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
	fflush(log);

	int itr;

	sizesend = sizeof(struct data1003);
	// Send the Levin handshake request data
	if ( sendto (s1, datagram1,  sizeof(struct data1003) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
	{
		fprintf (log, "Error sending 1003 data. Error number: %d . Error message: %s \n" , errno , strerror(errno));
		 fflush(log);
		exit(0);
	}

	//m = inet_ntoa(dest.sin_addr);
	fprintf(log, "1003 request data sent to:   %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
	fflush(log);
        //printf("==========================================================================\n");


	// RECV
	//ez bada ezer jasotzen edo mezuaren tamaina 0 -> nodoa ezabatu

	unsigned char *recbuf1 = (unsigned char*)malloc(72); // (38+33=71 bytes)
	//unsigned char *recbufh = (unsigned char*)malloc(33); // levin header
	int sized = sizeof(dest);
	int b1 = recvfrom(s1, recbuf1, 72, 0, (struct sockaddr *) &dest, &sized);
	//int bh = recvfrom(s1, recbufh, 33, 0, (struct sockaddr *) &dest, &sized);

	if (b1 == -1) {
                fprintf(log, "Recvfrom failed: %s \n", strerror(errno));
		fflush(log);
                exit(1);
        }

	char leng1[7];
	sprintf(leng1, "0x%.2x%.2x", recbuf1[9],recbuf1[8]);
	long lend1 = strtol(leng1, NULL, 16);
	fprintf(log, "1003 response received \t\t\t\t Packet size: %ld = %lx \t\t Stored in '%s_1003'\n", lend1,lend1, target);
	fflush(log);

	char fileizena1[50];
	strcpy(fileizena1, target);
	strcat(fileizena1, "_1003");
	FILE *em = fopen(fileizena1, "w");
	if(em == NULL)
	{
		fprintf(log, "Error in creating the file: %s\n", strerror(errno));
		fflush(log);
                exit(1);
	}
	// mezua fitxategian idatzi
	fwrite(recbuf1, sizeof(char), (int)lend1+33, em);

	fflush(stdout);

	close(s1); // socket-a itxi
	fflush(em);
	fclose(em);
	fflush(log);
	fclose(log);
	pthread_exit(NULL);

}
