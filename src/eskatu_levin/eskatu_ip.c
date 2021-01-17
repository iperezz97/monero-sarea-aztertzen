/*
 * $ gcc eskatu_ip.c -lpthread -o eskatu_ip
 * $ sudo ./eskatu_ip XXX.XXX.XXX.XXX PPPPP
 */


#include<stdio.h>        // printf...
#include<string.h>       // memset, strtok...
#include<stdlib.h>       // for exit(0);
#include<sys/socket.h>   // linux socket
#include<errno.h>        // For errno - the error number
#include<pthread.h>      // to create threads (or fork...)
#include<netdb.h>        // hostend
#include<arpa/inet.h>    // inet_addr...
#include<netinet/tcp.h>  // Provides declarations for tcp header
#include<netinet/ip.h>   // Provides declarations for ip header
#include<unistd.h>       // close, write
#include<linux/kernel.h> // types __uXX...


#define LEV_SLEN 8      // Levin signature length

// Declarations
//// Methods
void * receive_ack( void *ptr );
void process_packet(unsigned char* , int);
int start_sniffer();
unsigned short csum(unsigned short * , int );
char * hostname_to_ip(char * );
int get_local_ip (char *);
void local_ip (char *);

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
        unsigned char d[243];
} __attribute__((packed));

// Needed for checksum calculation
struct pseudo_header
{
        unsigned int source_address;
        unsigned int dest_address;
        unsigned char placeholder;
        unsigned char protocol;
        unsigned short tcp_length;

        struct tcphdr tcp;
};


struct in_addr dest_ip;


//  Main program //
int main(int argc, char *argv[])
{

        // Create a raw socket to send
        int s = socket (AF_INET, SOCK_RAW , IPPROTO_TCP); // s: socket descriptor
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
        //unsigned char datagram[4096];
        unsigned char datagram0[40]; // SYN bidaltzeko
        unsigned char datagram[329]; // Levin paketeak bidaltzeko


        // IP header
        struct iphdr *iph0 = (struct iphdr *) datagram0;
        struct iphdr *iph = (struct iphdr *) datagram;

        // TCP header
        struct tcphdr *tcph0 = (struct tcphdr *) (datagram0 + sizeof (struct ip));
        struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));

        // Levin header
        struct levhdr *lvh = (struct levhdr *) (datagram + sizeof(struct ip) + sizeof (struct tcphdr));

        // Payload
        struct data *dat = (struct data *) (datagram + sizeof(struct ip) + sizeof(struct tcphdr) + sizeof(struct levhdr));


        struct sockaddr_in  dest;
        struct pseudo_header psh;

        char *target = argv[1];   // destination addr
        int port = atoi(argv[2]); // port

        if(argc < 3)
        {
                printf("Please specify hostname and port of destination\n");
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

        int  source_port = 18080; // edozein?
        char source_ip[20];

        // Get local machine's IP
        //local_ip( source_ip );
        get_local_ip( source_ip );

        printf("Local source IP is %s \n" , source_ip);

        memset (datagram0, 0, 329);     /* zero out the buffer */
        memset (datagram, 0, 329);      /* zero out the buffer */

        // Fill in the IP Header
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
        iph->id = htons (54321);        // Id of this packet
        iph->frag_off = htons(16384);
        iph->ttl = 64;
        iph->protocol = IPPROTO_TCP;
        iph->check = 0;         // Set to 0 before calculating checksum
        iph->saddr = inet_addr ( source_ip );   // Spoof the source ip address
        iph->daddr = dest_ip.s_addr;

        // Calculate checksum
        iph->check = csum ((unsigned short *) datagram, iph->tot_len >> 1);

        // TCP Header
        tcph->source = htons ( source_port );
        tcph->dest = htons (port);
        tcph->seq = htonl(11078);
        tcph->ack_seq = 0;
        tcph->doff = sizeof(struct tcphdr) / 4;  // Size of tcp header
        tcph->fin=0;
        tcph->syn=1; // desactivate SYN flag if we want to send data (else TCP PROTOCOL VIOLATION)
        tcph->rst=0;
        tcph->psh=0;
        tcph->ack=0;
        tcph->urg=0;
        tcph->window = htons ( 14600 ); // maximum allowed window size
        tcph->check = 0; // if you set a checksum to zero, your kernel's IP stack should fill in the correct checksum during transm$
        tcph->urg_ptr = 0;

        // Fill in the Levin Header (handshake request)
        lvh->sign = 0x0101010101012101;     // 8 bytes
        lvh->length = 0xf3;                 // 8 bytes
        lvh->exp_resp = 0x01;               // 1 byte
        lvh->comm_cod = htonl(0xe9030000);  // 4 bytes
        lvh->retn_cod = htonl(0x00000000);  // 4 bytes
        lvh->reserved = htonl(0x01000000);  // 4 bytes
        lvh->endchars = htonl(0x01000000);  // 4 bytes

        //tcph0=tcph;
        //iph0=iph;

	unsigned char datt[243] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x09, 0x6e, 0x6f, 0x64, 0x65, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x0c, 0x07, 0x6d, 0x79, 0x5f, 0x70, 0x6f, 0x72, 0x74, 0x06, 0xa0, 0x46, 0x00, 0x00, 0x0a, 0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x5f, 0x69, 0x64, 0x0a, 0x40, 0x12, 0x30, 0xf1, 0x71, 0x61, 0x04, 0x41, 0x61, 0x17, 0x31, 0x00, 0x82, 0x16, 0xa1, 0xa1, 0x10, 0x07, 0x70, 0x65, 0x65, 0x72, 0x5f, 0x69, 0x64, 0x05, 0x9c, 0x85, 0x56, 0x0d, 0x0e, 0xd2, 0x09, 0xeb, 0x0c, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x14, 0x15, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x05, 0x75, 0x5c, 0x4a, 0xf7, 0xd8, 0xc4, 0x14, 0x01, 0x1b, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x5f, 0x74, 0x6f, 0x70, 0x36, 0x34, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x74, 0x5f, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x05, 0x92, 0x49, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x74, 0x6f, 0x70, 0x5f, 0x69, 0x64, 0x0a, 0x80, 0x24, 0x1d, 0x25, 0xda, 0xf6, 0x97, 0xab, 0x1c, 0x3b, 0xdf, 0x4d, 0x93, 0xc2, 0x6e, 0x6c, 0x42, 0x4b, 0x3f, 0xf0, 0x84, 0xfb, 0xcd, 0x24, 0x5a, 0xba, 0x17, 0xe6, 0xb0, 0x8a, 0x37, 0x70, 0x73, 0x0b, 0x74, 0x6f, 0x70, 0x5f, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x08, 0x0e};

	memcpy(dat->d, datt, 243); // honela esleitu, bestela 0x00 interpretatzean bukatzen da

	// IP_HDRINCL to tell the kernel that headers are included in the packet
	int one = 1;
	const int *val = &one;

	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
	{
		printf ("Error setting IP_HDRINCL. Error number: %d . Error message: %s \n" , errno , strerror(errno));
		exit(0);
	}

	printf("Starting sniffer thread...\n");
	char *message1 = "Thread 1";
	int  iret1;

	pthread_t sniffer_thread;

	/* Create thread to receive packets */
	if(pthread_create(&sniffer_thread , NULL ,  receive_ack , (void*) message1) < 0)
	{
		printf ("Could not create sniffer thread. Error number: %d . Error message: %s \n" , errno , strerror(errno));
		exit(0);
	}

	printf("Starting to send SYN TCP packets\n");
	// to change

	FILE *fr = fopen("newresponse", "w");
        FILE *fp = fopen("newrequest", "w");
       	FILE *f0 = fopen("newsyn", "w");

	int i;

	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
	for(i = 1 ; i < 4 ; i++ )
	{

		tcph->dest = htons ( port );
		tcph->check = 0;	// if you set a checksum to zero, your kernel's IP stack should fill in the correct checksum during transmission

		psh.source_address = inet_addr( source_ip );
		psh.dest_address = dest.sin_addr.s_addr;
		psh.placeholder = 0;
		psh.protocol = IPPROTO_TCP;
		psh.tcp_length = htons( sizeof(struct tcphdr) );

		// Send 1001 req
		if(i==2) {

			tcph->syn=0; // data bidaltzeko syn=0 izan behar da
			memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));

                        tcph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));
                        int sizesend = sizeof(struct iphdr) + sizeof(struct tcphdr) + sizeof(struct levhdr) + sizeof(struct data);
                        // Send the Levin handshake request packet
                        if ( sendto (s, datagram , sizeof(struct iphdr) + sizeof(struct tcphdr) + sizeof(struct levhdr) + sizeof(struct data) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
                        {
				printf ("Error sending 1001 packet. Error number: %d . Error message: %s \n" , errno , strerror(errno));

                                exit(0);
                        }
                        else
                        {
                                char * m = inet_ntoa(dest.sin_addr);
                                printf("\n1001 request sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
                                int itr;
                                //  gorde fitxategian bidalitako levin eskaera
                                //FILE *fp = fopen("newrequest", "w");
                                if(fp == NULL)
                                {
                                        printf("Error in creating the file\n");
                                        exit(1);
                                }

                                fwrite(datagram+sizeof(struct ip) + sizeof(struct tcphdr), sizeof(char), 243+33, fp);
                        	fputs("\n", fp);
			        fclose(fp);
                        }
			sleep(2);
		}
		// Send 1007 resp
		else if(i==3){
			lvh->length = 0x1d;
			lvh->exp_resp = 0x00;
			lvh->comm_cod = htonl(0xef030000);
			lvh->retn_cod = htonl(0x01000000);
			lvh->reserved = htonl(0x02000000);
			unsigned char dat7[29] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x04, 0x0d, 0x73, 0x75, 0x70, 0x70, 0x6f, 0x72, 0x74, 0x5f, 0x66, 0x6c, 0x61, 0x67, 0x73, 0x06, 0x01, 0x00, 0x00, 0x00};
			memcpy(dat->d, dat7, 29);

                        int sizesend = sizeof(struct iphdr) + sizeof(struct tcphdr) + sizeof(struct levhdr) + 29;
                        // Send the Levin 1007 response
                        if ( sendto (s, datagram , sizeof(struct iphdr) + sizeof(struct tcphdr) + sizeof(struct levhdr) + 29, 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
                        {
                                printf ("Error sending 1007 packet. Error number: %d . Error message: %s \n" , errno , strerror(errno));

                                exit(0);
                        }
                        else
                        {
                                char * m = inet_ntoa(dest.sin_addr);
                                printf("\n1007 response sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
                                int itr;
                                //  gorde fitxategian bidalitako levin eskaera
                                if(fr == NULL)
                                {
                                        printf("Error in creating the file\n");
                                        exit(1);
                                }

                                fwrite(datagram+sizeof(struct ip) + sizeof(struct tcphdr), sizeof(char), 29+33, fr);
				fputs("\n", fr);
                                fclose(fr);
                        }

		}
		else {
                        memcpy(iph0 , iph , sizeof (struct iphdr));
                        memcpy(tcph0 , tcph , sizeof (struct tcphdr));

                	memcpy(&psh.tcp , tcph0 , sizeof (struct tcphdr));

                	tcph0->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));
                	int sizesend = sizeof(struct iphdr) + sizeof(struct tcphdr);
                	// Send the TCP SYN packet
                	if ( sendto (s, datagram0 , sizeof(struct iphdr) + sizeof(struct tcphdr), 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
                	{
				printf ("Error sending SYN packet. Error number: %d . Error message: %s \n" , errno , strerror(errno));
                	        exit(0);
                	}
                	else
                	{
                        	char * m = inet_ntoa(dest.sin_addr);
                     		printf("TCP SYN sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
                        	int itr;
                        	//  gorde fitxategian bidalitako levin eskaera
                        	//FILE *fp = fopen("newrequest", "w");
                        	if(fp == NULL)
                        	{
                              		printf("Error in creating the file\n");
                               		exit(1);
                        	}
				//printf("TCP SYN: >> ");
				//fwrite(datagram0+0, sizeof(char), 40, stdout);
	                        fwrite(datagram0+0, sizeof(char), 40, f0);
				//printf("<< \n ");
				fputs("\n", f0);
				//fflush(stdout);
				fflush(f0);
        	                fclose(f0);

	                }

			sleep(3);

		}

	}

	pthread_join( sniffer_thread , NULL);
	printf("Buk.: %d\n" , iret1);
	//close(s);
	return 0;
}


/*
    Method to sniff incoming packets and look for ACK replies
 */
void * receive_ack( void *ptr )
{
	// Start the sniffer process
	start_sniffer();
}

int start_sniffer()
{
	int sock_raw;

	int saddr_size , data_size;
	struct sockaddr saddr;

	unsigned char *buffer = (unsigned char *)malloc(65536); //Its Big!

	printf("Sniffer initialising...\n");
	fflush(stdout);

	// Create a raw socket that shall sniff
	sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);

	if(sock_raw < 0)
	{
		printf("Socket Error\n");
		fflush(stdout);
		return 1;
	}

	saddr_size = sizeof saddr;

	while(1)
	{
		// agian lehenengo header 33 byte jaso eta hortik length atributua atera eta jaso gainerako informazioa? hasieran 1007 request jasoko dugu gero 1001 (eta hor IPak)
		// Receive packets until Ctrl C ... to change
		data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &saddr , &saddr_size);
		int err = errno;
		if(data_size <0 )
		{
			printf("Recvfrom error , failed to get packets: %s\n", strerror(err));
			fflush(stdout);
			return 1;
		}
		else
		{
			//printf("Received\n");
		}

		//Now process the packet
		process_packet(buffer , data_size);
	}
	close(sock_raw);
	printf("Sniffer finished.");
	fflush(stdout);
	return 0;
}

/*
    Print port of the ACK sender and more information ... to change
 */
void process_packet(unsigned char* buffer, int size)
{
	// Get the IP Header part of this packet
	struct iphdr *iph = (struct iphdr*)buffer;
	struct sockaddr_in source,dest;
	unsigned short iphdrlen;

	unsigned char * ipak;
	long ip[4];
	long ips;

/*
	int i;
	for(i=0;i<size;i++)
	{
		printf(" %.2x", *buffer);
		buffer++;
	}
*/
	if(iph->protocol == 6) // 6 == TCP
	{
		struct iphdr *iph = (struct iphdr *)buffer;
		iphdrlen = iph->ihl*4;

		struct tcphdr *tcph=(struct tcphdr*)(buffer + iphdrlen);

		//struct levhdr *levh=(struct levhdr*)(buffer); //?

		//struct ethhdr *eth = (struct ethhdr *)(buffer);

		memset(&source, 0, sizeof(source));
		source.sin_addr.s_addr = iph->saddr;

		memset(&dest, 0, sizeof(dest));
		dest.sin_addr.s_addr = iph->daddr;

		if(tcph->syn == 1 && tcph->ack == 1 && source.sin_addr.s_addr == dest_ip.s_addr )
		{
			printf("ACK received. Port %d open \n", ntohs(tcph->source));
			//printf("My host: %s and port %d \n", inet_ntoa(dest.sin_addr), ntohs(tcph->dest));
			fflush(stdout);
		}


		unsigned char * data = (buffer + iphdrlen /*+ sizeof(struct ethhdr)*/ + sizeof(struct tcphdr));
		int remaining_data = size - (iphdrlen + sizeof(struct tcphdr));

		int i;
		char * buf;
		char * buf2;

		for(i=0;i<remaining_data;i++)
		{
			printf(" %.2x", data[i]);
		}

//		printf("%d %d %d %d %d %d %d, ", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
//		printf("%s", buffer[4]);
//		printf("%s %s %s %s %s ", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);


/*
		char  c1 = ',';
		char  c2 = ' ';
		char result[INET_ADDRSTRLEN];

		if(strcmp(&buffer[3], "0x01"))
		{
			write(2, (inet_ntop(AF_INET, (void*)(&buffer[4]), result, sizeof result)), sizeof(result));
			write(2, c1, sizeof c1);
		}
		else
		{
			char result[INET6_ADDRSTRLEN];
			write(3, (inet_ntop(AF_INET, (void*)(&buffer[4]), result, sizeof result)), sizeof(result));
			write(3, c1, sizeof c1);
		}
*/
//		printf("%s, " , inet_ntop(AF_INET, (void*)(&buffer[4]), result, sizeof result));

	}
	//buffer=NULL;
}


/*
    Checksums - IP and TCP
 */
unsigned short csum(unsigned short *ptr,int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;

	return(answer);
}


/*
    Get ip from domain name
 */
char* hostname_to_ip(char * hostname)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ( (he = gethostbyname( hostname ) ) == NULL)
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



/*
    Get source IP of system , like 192.168.0.6 or 192.168.1.2
 */
int get_local_ip ( char * buffer)
{
	int sock = socket ( AF_INET, SOCK_DGRAM, 0);

	const char* kGoogleDnsIp = "8.8.8.8";
	int dns_port = 53;

	struct sockaddr_in serv;

	memset( &serv, 0, sizeof(serv) );
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
	serv.sin_port = htons( dns_port );

	int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );

	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	err = getsockname(sock, (struct sockaddr*) &name, &namelen);

	const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);

	close(sock);
}

