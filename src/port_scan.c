/*
  TCP SYN packets sender & ACK and data receiver with linux raw sockets in c
  Compile:  $ gcc port_scan.c -lpthread -o port_scan
  Run:      $ sudo ./port_scan <IPAddress> <Port>
  Aldatuko da argumentu bezala ordez fitxategitik irakurtzeko IP eta portuak, prozesu bakoitzerako hariak sortuz
*/


#include<stdio.h>       // printf...
#include<string.h>      // memset, strtok...
#include<stdlib.h>      // for exit(0);
#include<sys/socket.h>	// linux socket
#include<errno.h>       // For errno - the error number
#include<pthread.h>	// to create threads (or fork...)
#include<netdb.h>	// hostend
#include<arpa/inet.h>	// inet_addr...
#include<netinet/tcp.h>	// Provides declarations for tcp header
#include<netinet/ip.h>	// Provides declarations for ip header
#include<unistd.h>	// close, write

#define ETH_ALEN 6	// Octets in one ethernet addr

// Declarations
void * receive_ack( void *ptr );
void process_packet(unsigned char* , int);
int start_sniffer();
unsigned short csum(unsigned short * , int );
char * hostname_to_ip(char * );
int get_local_ip (char *);
void local_ip (char *);

typedef unsigned short __u16;
typedef __u16 __le16;
typedef __u16 __be16;

// Ethernet header struct declaration
struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	__be16		h_proto;		/* packet type ID field	*/
} __attribute__((packed));

struct pseudo_header    //needed for checksum calculation
{
	unsigned int source_address;
	unsigned int dest_address;
	unsigned char placeholder;
	unsigned char protocol;
	unsigned short tcp_length;

	struct tcphdr tcp;
};

struct in_addr dest_ip;


/*
    Main program
 */
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
	char datagram[4096];

	// IP header
	struct iphdr *iph = (struct iphdr *) datagram;

	// TCP header
	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));

	struct sockaddr_in  dest;
	struct pseudo_header psh;

	char *target = argv[1];
	int port = atoi(argv[2]);

	if(argc < 3)
	{
		printf("Please specify hostname and port of destination\n");
		exit(1);
	}

	if(inet_addr(target) != -1)
	{
		dest_ip.s_addr = inet_addr( target );
	}
	else
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

	int  source_port = 43591; // edozein?
	char source_ip[20];

	// Get local machine's IP
	//local_ip( source_ip );
	get_local_ip( source_ip );

	printf("Local source IP is %s \n" , source_ip);

	memset (datagram, 0, 4096);	/* zero out the buffer */

	// Fill in the IP Header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
	iph->id = htons (54321);	// Id of this packet
	iph->frag_off = htons(16384);
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;		// Set to 0 before calculating checksum
	iph->saddr = inet_addr ( source_ip );	// Spoof the source ip address
	iph->daddr = dest_ip.s_addr;

	// Calculate checksum
	iph->check = csum ((unsigned short *) datagram, iph->tot_len >> 1);

	// TCP Header
	tcph->source = htons ( source_port );
	tcph->dest = htons (port);
	tcph->seq = htonl(11078);
	tcph->ack_seq = 0;
	tcph->doff = sizeof(struct tcphdr) / 4;	 // Size of tcp header
	tcph->fin=0;
	tcph->syn=1; // activate SYN flag
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=0;
	tcph->urg=0;
	tcph->window = htons ( 14600 );	// maximum allowed window size
	tcph->check = 0; // if you set a checksum to zero, your kernel's IP stack should fill in the correct checksum during transmission
	tcph->urg_ptr = 0;

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

	int i;
	/* Read from file IP and port *
	char const* const fileName = "ipport";
	FILE* file = fopen(fileName, "r");
	char line[256];

	char * ipport;
	int k=0;
	while (fgets(line, sizeof(line), file)) {
		//printf("%s", line);
		ipport = strtok(line, ":");
		while( ipport != NULL ) {
			printf( "%s ", ipport ); //printing each token
			ipport = strtok(NULL, ":");
			if(k%2==1) printf("\n");
			k++;
		}
	}
	fclose(file);
	/* */


	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
	for(i = 1 ; i < 2 ; i++)
	{

		tcph->dest = htons ( port );
		tcph->check = 0;	// if you set a checksum to zero, your kernel's IP stack should fill in the correct checksum during transmission

		psh.source_address = inet_addr( source_ip );
		psh.dest_address = dest.sin_addr.s_addr;
		psh.placeholder = 0;
		psh.protocol = IPPROTO_TCP;
		psh.tcp_length = htons( sizeof(struct tcphdr) );

		memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));

		tcph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));

		// Send the SYN packet
		if ( sendto (s, datagram , sizeof(struct iphdr) + sizeof(struct tcphdr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
		{
			printf ("Error sending SYN packet. Error number: %d . Error message: %s \n" , errno , strerror(errno));
			exit(0);
		}
		else
		{
			char * m = inet_ntoa(dest.sin_addr);
			printf("SYN sent to: %s\n", m);
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

	if(iph->protocol == 6) // 6 == TCP
	{
		struct iphdr *iph = (struct iphdr *)buffer;
		iphdrlen = iph->ihl*4;

		struct tcphdr *tcph=(struct tcphdr*)(buffer + iphdrlen);

		struct ethhdr *eth = (struct ethhdr *)(buffer);

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


		unsigned char * data = (buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr));
		int remaining_data = size - (iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr));

		int i;
		char * buf;
		char * buf2;
/*
		for(i=0;i<remaining_data;i++)
		{
			printf(" %.2x", data[i]);
		}
*/
		printf("%d %d %d %d %d %d %d, ", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
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
    Sometimes segmentation fault
 */
void local_ip ( char * buffer)
{

	char * comm = "ip link show | grep '^[2-9]' | awk '{print $2}' | cut -d ':' -f 1 ";
	char resp[128];
	FILE * f = popen(comm, "r");
        // read output from command
	//fscanf(f, "%s", resp);
	fgets(resp, sizeof(resp), f);
	char * re;
	char * eth;
	char * wls;
	strcpy(eth, resp);
	int len = strlen(eth);
	if(eth[len-1] == '\n') eth[len-1] = 0;
	//printf("%s",resp);
	fgets(resp, sizeof(resp), f);
	wls = resp;
	int len2 = strlen(wls);
	if(wls[len2-1] == '\n') wls[len2-1] = 0;


	//printf("%s eta %s\n", eth, wls); // enp3s0f1 and wlp2s0
	//fflush(stdout);


	// ethernet interface: enp3s0f1 & wireless interface wlp2s0
	pclose(f);

	char command[1024];

	// GET IP of ethernet and wireless interface
	sprintf(command, "ifconfig | grep -A 1 '%s' | grep 'inet ' | awk '{print $2}'  && ifconfig | grep -A 1 '%s' | grep 'inet ' | awk '{print $2}' && echo ", eth, wls);


	//char * command = "ifconfig | grep inet | awk '{print $2}' | head -n 1"; // lehen interfazearen IPa
	//char * command = "ss | grep tcp | grep ESTAB | awk '{print $5}' | cut -d ':' -f 1 | head -n 1" // Konexioak ezarrita dituen lehen IPa (socket statistics)
	//char * command = "ip address | grep -A 2 enp | grep 'inet ' | awk '{print $2}' | cut -d '/' -f 1  && ip address | grep -A 2 wl | grep 'inet ' | awk '{print $2}' | cut -d '/' -f 1 "

	FILE * fp = popen(command, "r");
        // read output from command
        fscanf(fp, "%s", buffer);

	buffer = strtok(buffer,"\n");

	pclose(fp);
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

