/*
 * $ gcc eskatu_ipp.c -o eskatu_ipp
 * $ sudo ./eskatu_ipv4 XXX.XXX.XXX.XXX PPPPP
 */


#include<stdio.h>        // printf...
#include<string.h>       // memset, strtok...
#include<stdlib.h>       // for exit(0);
#include<sys/socket.h>   // linux socket
#include<errno.h>        // For errno - the error number
#include<netdb.h>        // hostend
#include<arpa/inet.h>    // inet_addr...
#include<netinet/tcp.h>  // Provides declarations for tcp header
#include<netinet/ip.h>   // Provides declarations for ip header
#include<unistd.h>       // close, write
#include<linux/kernel.h> // types __uXX...


#define LEV_SLEN 8      // Levin signature length

// Declarations
//// Methods
char * hostname_to_ip(char * );

//// Structs
// Levin header struct declaration
struct levhdr {
        __be64          sign;           // Signature of msg (8 bytes)   /  0x0121010101010101 (beti)
        //unsigned char sign[LEV_SLEN]; // Signature of msg (8 bytes)   /  0x01 0x21 0x01 0x01 0x01 0x01 0x01 0x01
        __le64          length;         // Length of msg (8 bytes)      /  0xf3=243
        unsigned char   exp_resp;       // Expected response (1 byte)   /  0x01 (eskaera)
        //uint8_t       exp_resp;       // Expected response (1 byte)   /  0x01 (eskaera)
        __le32          comm_cod;       // Command code (4 bytes)       /  0xe903=1001 (handshake), 0xef03=1007 (support flags)
        __le32          retn_cod;       // Return code (4 bytes)        /  0x00000000
        __le32          reserved;       // Reserved (4 bytes)           /  0x01000000
        __be32          endchars;       // Ending chars (4 bytes)       /  0x01000000 (beti)
} __attribute__((packed)); //33 byte-ak enpaketatuta

// Levin payload struct
struct data {
        unsigned char d[226];
} __attribute__((packed));

struct in_addr dest_ip;


//  Main program //
int main(int argc, char *argv[])
{

        // Create a raw socket to send
        int s = socket (AF_INET, SOCK_STREAM , IPPROTO_TCP); // s: socket descriptor
        if(s < 0)
        {
                printf ("Error creating socket. Error number : %d . Error message : %s \n" , errno , strerror(errno));
                exit(0);
        }
        else
        {
                printf("Socket created.\n");
        }

        // Datagram to represent the packet
        unsigned char datagram1[266]; // 1001 payload bidaltzeko
        unsigned char datagram0[40];  // SYN bidaltzeko
        unsigned char datagram[329];  // Levin goiburukoak bidaltzeko

        // Levin header
        struct levhdr *lvh = (struct levhdr *) (datagram);

        // Payload
        struct data *dat = (struct data *) (datagram1);


        struct sockaddr_in  dest;

        char *target = argv[1];   // destination addr
        int port = atoi(argv[2]); // port

        if(argc < 3)
        {
                printf("Please specify hostname and port of destination.\n");
                exit(1);
        }

        if(inet_addr(target) != -1) // is IP
        {
                dest_ip.s_addr = inet_addr( target );
        }
        else // convert hostname
        {
                char *ip = hostname_to_ip(target);
                if(ip != NULL)
                {
                        printf("%s resolved to %s \n" , target , ip);
                        // Convert domain name to IP
                        dest_ip.s_addr = inet_addr( hostname_to_ip(target) );
                }
                else
                {
                        printf("Unable to resolve hostname : %s" , target);
                        exit(1);
                }
        }

        memset (datagram0, 0, 40);      /* zero out the buffer */
        memset (datagram1, 0, 266);     /* zero out the buffer */
        memset (datagram, 0, 329);      /* zero out the buffer */


        // Fill in the Levin Header (handshake request)
        lvh->sign = 0x0101010101012101;     // 8 bytes
        lvh->length = 0xe2;                 // 8 bytes
        lvh->exp_resp = 0x01;               // 1 byte
        lvh->comm_cod = htonl(0xe9030000);  // 4 bytes
        lvh->retn_cod = htonl(0x00000000);  // 4 bytes
        lvh->reserved = htonl(0x01000000);  // 4 bytes
        lvh->endchars = htonl(0x01000000);  // 4 bytes


	//unsigned char datt[243] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x09, 0x6e, 0x6f, 0x64, 0x65, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x0c, 0x07, 0x6d, 0x79, 0x5f, 0x70, 0x6f, 0x72, 0x74, 0x06, 0xa0, 0x46, 0x00, 0x00, 0x0a, 0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x5f, 0x69, 0x64, 0x0a, 0x40, 0x12, 0x30, 0xf1, 0x71, 0x61, 0x04, 0x41, 0x61, 0x17, 0x31, 0x00, 0x82, 0x16, 0xa1, 0xa1, 0x10, 0x07, 0x70, 0x65, 0x65, 0x72, 0x5f, 0x69, 0x64, 0x05, 0x9c, 0x85, 0x56, 0x0d, 0x0e, 0xd2, 0x09, 0xeb, 0x0c, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x14, 0x15, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x05, 0x75, 0x5c, 0x4a, 0xf7, 0xd8, 0xc4, 0x14, 0x01, 0x1b, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x5f, 0x74, 0x6f, 0x70, 0x36, 0x34, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x74, 0x5f, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x05, 0x92, 0x49, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x74, 0x6f, 0x70, 0x5f, 0x69, 0x64, 0x0a, 0x80, 0x24, 0x1d, 0x25, 0xda, 0xf6, 0x97, 0xab, 0x1c, 0x3b, 0xdf, 0x4d, 0x93, 0xc2, 0x6e, 0x6c, 0x42, 0x4b, 0x3f, 0xf0, 0x84, 0xfb, 0xcd, 0x24, 0x5a, 0xba, 0x17, 0xe6, 0xb0, 0x8a, 0x37, 0x70, 0x73, 0x0b, 0x74, 0x6f, 0x70, 0x5f, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x08, 0x0e};

	//memcpy(dat->d, datt, 243); // honela esleitu, bestela 0x00 interpretatzean bukatzen da

	// BIDALI LEHENIK GOIBURUKOA ETA ONDOREN DATA HAU:
	unsigned char datt[226] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x09, 0x6e, 0x6f, 0x64, 0x65, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x10, 0x0a, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x5f, 0x74, 0x69, 0x6d, 0x65, 0x05, 0x64, 0x93, 0x16, 0x60, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6d, 0x79, 0x5f, 0x70, 0x6f, 0x72, 0x74, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x5f, 0x69, 0x64, 0x0a, 0x40, 0x12, 0x30, 0xf1, 0x71, 0x61, 0x04, 0x41, 0x61, 0x17, 0x31, 0x00, 0x82, 0x16, 0xa1, 0xa1, 0x10, 0x07, 0x70, 0x65, 0x65, 0x72, 0x5f, 0x69, 0x64, 0x05, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x0c, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x10, 0x15, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x74, 0x5f, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x74, 0x6f, 0x70, 0x5f, 0x69, 0x64, 0x0a, 0x80, 0x41, 0x80, 0x15, 0xbb, 0x9a, 0xe9, 0x82, 0xa1, 0x97, 0x5d, 0xa7, 0xd7, 0x92, 0x77, 0xc2, 0x70, 0x57, 0x27, 0xa5, 0x68, 0x94, 0xba, 0x0f, 0xb2, 0x46, 0xad, 0xaa, 0xbb, 0x1f, 0x46, 0x32, 0xe3, 0x0b, 0x74, 0x6f, 0x70, 0x5f, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x08, 0x01};

	memcpy(dat->d, datt, 226); // honela esleitu, bestela 0x00 interpretatzean bukatzen da


        FILE *fp = fopen("request1001", "w");
       	FILE *f0 = fopen("newsynp", "w");

	int i;

	// helburuko nodoari buruzko informazioa bete
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
        dest.sin_port = htons( port );

	// 3-way handshake
	if(connect(s, (struct sockaddr *)&dest, sizeof(struct sockaddr)) < 0){
		printf("Error: %s", strerror(errno));
		exit(1);
	}

	printf("Connected.\n");

	// Send 1001 req

        int sizesend = sizeof(struct levhdr);
        // Send the Levin handshake request header packet
        if ( sendto (s, datagram, sizeof(struct levhdr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
		printf ("Error sending 1001 header. Error number: %d . Error message: %s \n" , errno , strerror(errno));
           	exit(0);
        }

        char * m = inet_ntoa(dest.sin_addr);
        printf("1001 request header sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
        int itr;
        //  gorde fitxategian bidalitako levin eskaera
        //FILE *fp = fopen("newrequest", "w");
        if(fp == NULL)
        {
                printf("Error in creating the file\n");
                exit(1);
        }

        fwrite(datagram, sizeof(char), 33, fp);
        //fputs("\n", fp);
	fclose(fp);
	//sleep(1);


	// bidali 1001 mezuaren gorputza
        sizesend = sizeof(struct data);
        // Send the Levin handshake request data
        if ( sendto (s, datagram1,  sizeof(struct data) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
                 printf ("Error sending 1001 data. Error number: %d . Error message: %s \n" , errno , strerror(errno));
                 exit(0);
        }

        m = inet_ntoa(dest.sin_addr);
        printf("1001 request data sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
        //int itr;
        //  gorde fitxategian bidalitako levin eskaera
        FILE *fp2 = fopen("1001body", "w");
        if(fp2 == NULL)
        {
                 printf("Error in creating the file\n");
                 exit(1);
        }
        printf("==========================================================================\n");

        fwrite(datagram1, sizeof(char), 226, fp2);
        //fputs("\n", fp);
        fclose(fp2);
	//sleep(3);

	// RECV

	unsigned char *recbuf0 = (unsigned char*)malloc(43); // ignore 1007 request
	unsigned char *recbufh = (unsigned char*)malloc(33); // 1001 header
	unsigned char *recbufd = (unsigned char*)malloc(65536); // too big
	int sized = sizeof(dest);
	int b0 = recvfrom(s, recbuf0, 43, 0, (struct sockaddr *) &dest, &sized); // ignore
	int bh = recvfrom(s, recbufh, 33, 0, (struct sockaddr *) &dest, &sized); // header

	//int tam = 65536;

        if (b0 == -1 || bh == -1) {
                perror("Recvfrom failed");
                exit(1);
        }
	printf("1007 request received (ignore) \t\t\t Packet size: %d = %x.\n1001 response header received \t\t\t Packet size: %d = %x.\n", b0,b0,bh,bh);
	char leng[7];
	sprintf(leng, "0x%.2x%.2x", recbufh[9],recbufh[8]);
	//printf("1001 response data size: %s\n", leng);
	long lend = strtol(leng, NULL, 16);
	printf("1001 response data received \t\t\t Packet size: %ld = %lx.\n", lend, lend);
	//sprintf(lend, "%s", leng);
	//printf("1001 response data size: %d\n", lend);
	int bd=1;
	int it=0;

	// inprimatu header-a
	for(it=0; it<bh; it++){
		printf("%.2x ", recbufh[it]);
	}

	FILE *em = fopen("emaitz1001", "w");
	if(em == NULL)
	{
		printf("Error in creating the file\n");
                exit(1);
	}

	fwrite(recbufh, sizeof(char), 33, em);

	while(bd > 0) {
		bd = recvfrom(s, recbufd, lend, 0, (struct sockaddr *) &dest, &sized);
		if (bd == -1){
			perror("Recvfrom failed");
        	        exit(1);
		}
		for(it=0; it<bd; it++){
			printf("%.2x ", recbufd[it]);
		}
		fwrite(recbufd, sizeof(char), bd, em);
		if(bd >= lend){
			break;
		}
		lend-=bd; // jasotzeko falta diren byteak kontrolatu

        }
	//printf("\n");
	fflush(stdout);



	printf("\n");
	close(s);
	return 0;
}



/*
    Get ip from domain name
 */
char* hostname_to_ip(char * hostname)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ((he = gethostbyname(hostname)) == NULL)
	{
		// get the host info
		herror("gethostbyname");
		return NULL;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for(i = 0; addr_list[i] != NULL; i++)
	{
		// Return the first one;
		return inet_ntoa(*addr_list[i]) ;
	}

	return NULL;
}
