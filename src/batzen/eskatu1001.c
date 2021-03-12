#include "nagusia.h"
#include "bzb.h"
#include "eskatu1001.h"

struct in_addr dest_ip;
struct in_addr lort;
unsigned char combuf[550];
int s;

void * eskatu_ip(void *args) {

	// log fitxategia ireki
	FILE *log;
	log = fopen("log1001", "w");
	if(log == NULL) {
		printf("Error creating log file: %s\n", strerror(errno));
		exit(0);
	}
	fprintf(log, "Creating socket... Destination: %s\n", (char *)args);

	// Create a raw socket to send
        s = socket (AF_INET, SOCK_STREAM , IPPROTO_TCP); // s: socket descriptor
        if(s < 0)
        {
                fprintf (log, "Error creating socket. Error number : %d . Error message : %s \n" , errno , strerror(errno));
                exit(0);
        }
	fprintf(log, "Socket descriptor: %d\n", s);
	fflush(log);

	// Datagram to represent the packet
        unsigned char datagram[329]; // 1001 header bidaltzeko
        unsigned char datagram1[266]; // 1001 data bidaltzeko

	// Levin header
        struct levhdr *lvh = (struct levhdr *) (datagram);

        // Payload
        struct data *dat = (struct data *) (datagram1);

	// Dest addr struct
        struct sockaddr_in  dest;

	char *token = strtok(args, " "); 	// bi argumentuak jaso
        char *target = token; 			// destination addr
        int port = atoi(strtok(NULL, " "));	// port
	//printf("%s %d \n", target, port);


	if(inet_addr(target) == -1) // is not IP
        {
		fprintf(log, "IP desegokia");
		exit(1);
        }

	if(port == 0) {
		fprintf(log, "Portu desegokia");
		exit(1);

	}

        dest_ip.s_addr = inet_addr( target );

        memset (datagram, 0, 329);     /* zero out the buffer */
        memset (datagram1, 0, 266);    /* zero out the buffer */

	// Fill in the Levin Header (handshake request)
        lvh->sign = 0x0101010101012101;     // 8 bytes
        lvh->length = 0xe2;                 // 8 bytes
        lvh->exp_resp = 0x01;               // 1 byte
        lvh->comm_cod = htonl(0xe9030000);  // 4 bytes
        lvh->retn_cod = htonl(0x00000000);  // 4 bytes
        lvh->reserved = htonl(0x01000000);  // 4 bytes
        lvh->endchars = htonl(0x01000000);  // 4 bytes

	unsigned char datt[226] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x09, 0x6e, 0x6f, 0x64, 0x65, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x10, 0x0a, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x5f, 0x74, 0x69, 0x6d, 0x65, 0x05, 0x64, 0x93, 0x16, 0x60, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6d, 0x79, 0x5f, 0x70, 0x6f, 0x72, 0x74, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x5f, 0x69, 0x64, 0x0a, 0x40, 0x12, 0x30, 0xf1, 0x71, 0x61, 0x04, 0x41, 0x61, 0x17, 0x31, 0x00, 0x82, 0x16, 0xa1, 0xa1, 0x10, 0x07, 0x70, 0x65, 0x65, 0x72, 0x5f, 0x69, 0x64, 0x05, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x0c, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x10, 0x15, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x74, 0x5f, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x74, 0x6f, 0x70, 0x5f, 0x69, 0x64, 0x0a, 0x80, 0x41, 0x80, 0x15, 0xbb, 0x9a, 0xe9, 0x82, 0xa1, 0x97, 0x5d, 0xa7, 0xd7, 0x92, 0x77, 0xc2, 0x70, 0x57, 0x27, 0xa5, 0x68, 0x94, 0xba, 0x0f, 0xb2, 0x46, 0xad, 0xaa, 0xbb, 0x1f, 0x46, 0x32, 0xe3, 0x0b, 0x74, 0x6f, 0x70, 0x5f, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x08, 0x01};

	memcpy(dat->d, datt, 226); // honela esleitu, bestela 0x00 interpretatzean bukatzen da

	int i;

	// helburuko nodoari buruzko informazioa bete
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
        dest.sin_port = htons( port );

	// 3-way handshake (SYN-ACK)
	if(connect(s, (struct sockaddr *)&dest, sizeof(struct sockaddr)) < 0){
		fprintf(log, "Error: %s \n", strerror(errno));
		exit(1);
	}
	// kontuz... batzuk kolgatuta beste batzuk connection refused..
	fprintf(log, "Connected.\n");

        // Send the Levin handshake request header packet
        int sizesend = sizeof(struct levhdr);
        if ( sendto (s, datagram, sizeof(struct levhdr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
		fprintf (log, "Error sending 1001 header. Error number: %d . Error message: %s \n" , errno , strerror(errno));
           	exit(0);
        }

        char * m = inet_ntoa(dest.sin_addr);
        fprintf(log, "1001 request header sent to: %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);

       int itr;

        sizesend = sizeof(struct data);
        // Send the Levin handshake request data
        if ( sendto (s, datagram1,  sizeof(struct data) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
                 fprintf (log, "Error sending 1001 data. Error number: %d . Error message: %s \n" , errno , strerror(errno));
                 exit(0);
        }

        //m = inet_ntoa(dest.sin_addr);
        fprintf(log, "1001 request data sent to:   %s\t Packet size: %d = %x.\n", m, sizesend, sizesend);
        //printf("==========================================================================\n");


	// RECV
	unsigned char *recbuf0 = (unsigned char*)malloc(50); // ignore 1007 request (33+10=43 bytes)
	unsigned char *recbufh = (unsigned char*)malloc(33); // 1001 header
	int sized = sizeof(dest);
	int b0 = recvfrom(s, recbuf0, 50, 0, (struct sockaddr *) &dest, &sized); // ignore
	int bh = recvfrom(s, recbufh, 33, 0, (struct sockaddr *) &dest, &sized); // 1001 header

	if (b0 == -1 || bh == -1) {
                fprintf(log, "Recvfrom failed: %s \n", strerror(errno));
                exit(1);
        }

	char leng0[7];
	sprintf(leng0, "0x%.2x%.2x", recbuf0[9],recbuf0[8]);
	long lend0 = strtol(leng0, NULL, 16);
	fprintf(log, "1007 request received (ignore) \t\t\t Packet size: %ld = %lx.\n1001 response header received \t\t\t Packet size: %d = %x.\n", lend0,lend0,bh,bh);
	char leng[7]; // mezuaren tamaina hamaseitarrez gordetzeko
	sprintf(leng, "0x%.2x%.2x", recbufh[9],recbufh[8]);
	//printf("1001 response data size: %s\n", leng);
	long lend = strtol(leng, NULL, 16); // bihurtu mezuaren tamaina hamartarrera
	//unsigned char *recbufd = (unsigned char*)malloc(65536); // allocate data buffer
	unsigned char *recbufd = (unsigned char*)malloc(lend); // allocate data buffer

	int bd=1;
	int it=0;

	char fileizena[50];
	strcpy(fileizena, target);
	FILE *em = fopen(fileizena, "w");
	if(em == NULL)
	{
		fprintf(log, "Error in creating the file: %s\n", strerror(errno));
                exit(1);
	}
	// header-a fitxategian idatzi
	fwrite(recbufh, sizeof(char), 33, em);
	fprintf(log, "1001 response data received \t\t\t Packet size: %ld = %lx \t\t Stored in '%s'\n", lend, lend, target);

	while(bd > 0) {
		bd = recvfrom(s, recbufd, lend, 0, (struct sockaddr *) &dest, &sized);
		if (bd == -1){
			fprintf(log, "Recvfrom failed: %s \n", strerror(errno));
        	        exit(1);
		}
		//for(it=0; it<bd; it++){
		//	fprintf(log, "%.2x ", recbufd[it]);
		//}
		fwrite(recbufd, sizeof(char), bd, em);
		if(bd >= lend){
			break;
		}
		lend-=bd; // jasotzeko falta diren byteak kontrolatu

        }
	fflush(stdout);

	close(s); // socket-a itxi

	// kargatu komandoa jasotakoa hamaseitarrera bihurtzeko
	sprintf(combuf, "hexdump -C %s", fileizena);
	strcat(combuf, " | cut -d \' \' -f 2-19 | tr -s \'\\n\' \' \' | grep -oE \'04 61 64 64 72 0c 08 04 6d 5f 69 70 06 ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} 06 6d 5f 70 6f 72 74 07 ([0-9a-f]){2} ([0-9a-f]){2} 04\' | sed \'s/04 61 64 64 72 0c 08 04 6d 5f 69 70 / /g\' > ");
	strcat(combuf, fileizena);
	strcat(combuf, "a"); // fitxategi berri batean gorde komandoaren irteera (fileizena+'a')
	//fprintf(log, "%s\n", combuf);
	//fflush(stdout);

	// exekutatu komandoa
	system(combuf);

	// berdina IPv6 (mapped IPv4) ateratzeko fitx berean
	sprintf(combuf, "hexdump -C %s", fileizena);
	strcat(combuf, " | cut -d \' \' -f 2-19 | tr -s \'\\n\' \' \' | grep -oE \'04 61 64 64 72 0c 08 04 61 64 64 72 0a 40 00 00 00 00 00 00 00 00 00 00 ff ff ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} 06 6d 5f 70 6f 72 74 07 ([0-9a-f]){2} ([0-9a-f]){2}\' | sed \'s/04 61 64 64 72 0c 08 04 61 64 64 72 0a 40 00 00 00 00 00 00 00 00 00 00 ff ff/006/g\' >> ");
	strcat(combuf, fileizena);
	strcat(combuf, "a"); // fitxategi berean gorde komandoaren irteera (fileizena+'a')
	//fprintf(log, "%s\n", combuf);
	//fflush(stdout);
	// exekutatu komandoa
	system(combuf);

	// irakurri komandoarekin idatzi berri den fitxategia
	FILE *fi;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	strcat(fileizena, "a");
	fi = fopen(fileizena, "r");
	if(fi == NULL) {
		fprintf(log, "Error opening the last file: %s\n", strerror(errno));
                exit(1);
	}

	const char *patt0 = "06"; // ip
	const char *patt1 = "07"; // port

	char *aurk = NULL;
	char *aurkip = NULL;
	char *aurkport = NULL;
	char *hasi, *buka, *hasip, *bukap;

	char iphx[4][6]; // ip zatiak (hex)
	char pohx[8]; 	 // port zatiak (hex)
	char ipch[16];   // ip string osoa (dec)
	long por=0;	 // portu zenbaki osoa (dec)

	int iter = 0;
	while((read = getline(&line, &len, fi)) != -1) {

		if(iter > 0) {
			// aurkitu ip-a lerroan
			if(hasi = strstr(line, patt0)) {
				hasi += strlen(patt0);
				buka = hasi;
				buka += strlen(patt0)*6;
				aurkip = (char *)malloc(buka - hasi +1);
				memcpy(aurkip, hasi, buka-hasi);
				aurkip[buka-hasi] = '\0';
			}

			// aurkitu portua lerroan
                        if(hasip = strstr(line, patt1)) {
                                hasip += strlen(patt1);
				bukap = hasip;
				bukap += strlen(patt1)*3;
                                aurkport = (char *)malloc(bukap - hasip + 1);
                                memcpy(aurkport, hasip, bukap-hasip);
                                aurkport[bukap-hasip] = '\0';

                        }

			sprintf(iphx[0], "0x%c%c\n", aurkip[1], aurkip[2]);
			sprintf(iphx[1], "0x%c%c\n", aurkip[4], aurkip[5]);
			sprintf(iphx[2], "0x%c%c\n", aurkip[7], aurkip[8]);
			sprintf(iphx[3], "0x%c%c\n", aurkip[10], aurkip[11]);
			sprintf(pohx, "0x%c%c%c%c\n", aurkport[4],aurkport[5],aurkport[1],aurkport[2]);
			sprintf(ipch, "%ld.%ld.%ld.%ld", strtol(iphx[0], NULL, 16), strtol(iphx[1], NULL, 16), strtol(iphx[2], NULL, 16), strtol(iphx[3],NULL, 16));
			por = strtol(pohx, NULL, 16);
			//printf("%s %ld\n", ipch, por);
			//fflush(stdout);

			lort.s_addr = inet_addr(ipch);
                        //printf("Port: %ld", por);

                        txertatu_elem(lort, por);

		}
		iter++;
	}
	// nodoari eskatu zaio, 0 egoeratik 1 egoerara pasa:
	egoera_aldatu(dest_ip, 1);

	//free(combuf);

	//return (void*)erantzun;
	//pthread_exit(NULL);
	fflush(log);
	fclose(log);
	pthread_exit(NULL);
}

