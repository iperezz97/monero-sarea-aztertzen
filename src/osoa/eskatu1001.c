#include "nagusia.h"
#include "bzb.h"
#include "eskatu1001.h"

struct in_addr dest_ip;
struct in_addr lort;
unsigned char combuf[ESIZE];
unsigned char ler[LSIZE];
int s;
struct timeval read_timeout;

/* Hasieratu iterazioa zuhaitzaren erroarekin eta egoeraren arabera, 1001 exekutatu edo jarraitu egitura osoa iteratzen behin eta berriro
 * Egoera 0 denean exekutatuko da, 1001 komandoa bidaliz (Handshake request)
 * Hasteko zuhaitzaren lehen elementua soilik behar du, zuhaitzan txertatua izana (programa nagusiak sortzen du hariak sortu aurretik)
 * Egoera 0: Aurkitua izan da beste nodo bati 1001 bidaliz (egituran txertatuta)
 * Egoera 1: pareen zerrenda eskatu zaio (1001: Handshake request) -> PING bidaliko zaio konexioa aztertzeko
 * Egoera 2: nodoak ez du erantzun (1001 edo 1003) -> mapatik ezabatuko da
 * Egoera 3: mapatik ezabatu da (egituratik ez)
*/
void * hasieratu1001(void *args) {
        // log fitxategia ireki
        FILE *log;
        log = fopen("log1001", "w");
        if(log == NULL) {
                printf("Errorea log1001 fitxategia sortzean: %s\n", strerror(errno));
                fflush(stdout);
                pthread_exit(NULL);
        }
        //fprintf(log,"Fitxategia sortuta\n");
        //fflush(log);
        while(1) {
                hautatu_enodoa(root, log); // iteratu zuhaitza errekurtsiboki
        }
	fclose(log);
	pthread_exit(NULL);
}

/* Metodo errekurtsibo honekin PING bidaliko zaion nodoa aukeratuko da (pre-order)
 * Egoera 0 denean exekutatuko da, 1001 komandoa bidaliz (Hanshake request) nodoaren pareen zerrendatik 250 IP helbide lortzeko
 * Lortutako IP-ak egituran txertatuko dira eta nodoaren egoera 1 izango da
 * Ez bada espero zen erantzuna lortzen, 2 egoerara pasa mapatik ezabatzeko
*/
void * hautatu_enodoa(struct bzb_ip *un, FILE *log) {
	long err=0;
	void * stat;
        if(un == NULL) {
                //sleep(1);
                ;
        }
        else {
                if(un->egoe == 0) {
                        //pthread_mutex_lock(&(une->lock)); // blokeakorra (wait-ekin kontrolatu daiteke)
                        //pthread_mutex_trylock(&(une->lock)); // ez blokeakorra
                        stat = eskatu_ip(inet_ntoa(un->nodip), un->port, log);
			err = ( long ) stat;
			if(err > 0) { // erroreren bat gertatu da
				fprintf(log, B_RED"Errorea: %ld\n"RESET, err);
				fflush(log);
				pthread_mutex_lock(&(un->lock));   // blokeakorra (wait-ekin kontrolatu daiteke)
				if(un->egoe < 3) { // ezabatuta ez badago
					un->egoe = 2; // ezin nodoarekin konektatu -> mapatik ezabatu
				}
				pthread_mutex_unlock(&(un->lock)); // askatu
			}
			else { // exekuzioa ondo joan da
				pthread_mutex_lock(&(un->lock));   // blokeakorra (wait-ekin kontrolatu daiteke)
                                if(un->egoe < 3) { // ezabatuta ez badago
					un->egoe = 1; // lortu dira bere bizilagunak
				}
				pthread_mutex_unlock(&(un->lock)); // askatu
			}
                        //pthread_cond_wait(&(une->cond), &(une->lock)); // signal-aren zain geratzeko
                        //pthread_mutex_unlock(&(une->lock));
                }
                if(un->left != NULL) {
                        hautatu_enodoa(un->left, log);
                }
                if(un->right != NULL) {
                        hautatu_enodoa(un->right, log);
                }
        }
}

/* Levin protokoloaren 1001 komandoa bidaltzeko eta erantzuna jasotzeko metodoa (agian 1007 baztertu bitartean)
 * Itzulera kodeak zein errore gertatu den jakinaraziko du
 * Errorerik gertatzen bada nodoa mapatik ezabatuko da (2 egoerara pasaz)
*/
void * eskatu_ip(char * target, int port, FILE *log) {

	fprintf(log, "\nSocket-a sortzen... \tHelburuko nodoa: %s %d\n", target, port);
	fflush(log);

	// Create a raw socket to send
        s = socket (AF_INET, SOCK_STREAM , IPPROTO_TCP); // s: socket descriptor
        if(s < 0)
        {
                fprintf (log, "Errorea socket-a sortzean. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
		fflush(log);
                return (void *) 10;
        }
	fprintf(log, "Socket deskriptorea: %d\n", s); // desberdinak direla konprobatzeko
	fflush(log);

	read_timeout.tv_sec = 1;
	//read_timeout.tv_usec = 500000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);


	// Datagram to represent the packet
        unsigned char datagram[329]; // 1001 header bidaltzeko
        unsigned char datagram1[266]; // 1001 data bidaltzeko

	// Levin header
        struct levhdr *lvh = (struct levhdr *) (datagram);

        // Payload
        struct data *dat = (struct data *) (datagram1);

	// Dest addr struct
        struct sockaddr_in  dest;

	//char *token = strtok(args, " "); 	// bi argumentuak jaso
        //char *target = token; 		// destination addr
        //int port = atoi(strtok(NULL, " "));	// port
	//printf("%s %d \n", target, port);

	if(inet_addr(target) == -1) // is not IP
        {
		fprintf(log, "IP desegokia");
                fflush(log);
                return (void *) 1;
        }

	if(port == 0) {
		fprintf(log, "Portu desegokia");
                fflush(log);
                return (void *) 2;
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

	// segundu 1 eman konektatzeko
	int ko = connect_with_timeout(s, (struct sockaddr *)&dest, sizeof(struct sockaddr), 1000);

	if(ko >= 0){
		fprintf(log, "Ongi konektatu da: %d\n", ko);
		fflush(log);
	}
	else {
		fprintf(log, "Ezin konektatu: %d\n", ko);
		fflush(log);
		return (void *) 3;
	}

/*	// 3-way handshake (SYN-ACK)
	if(connect(s, (struct sockaddr *)&dest, sizeof(struct sockaddr)) < 0){
		fprintf(log, "Errorea helburuko nodoarekin konektatzean: %s \n", strerror(errno));
		fflush(log);
		return (void *) 3;
	}
*/
	// kontuz... batzuk kolgatuta beste batzuk connection refused..
	//fprintf(log, "Ongi konektatu da.\n");
	//fflush(log);

        // Send the Levin handshake request header packet
        int sizesend = sizeof(struct levhdr);
        if ( sendto (s, datagram, sizeof(struct levhdr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
		fprintf (log, "Errorea 1001 goiburukoa bidaltzean. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
           	fflush(log);
		return (void *) 4;
        }

        char * m = inet_ntoa(dest.sin_addr);
        fprintf(log, "1001 eskaeraren goiburukoa bidalita \t\t Pakete tamaina: %d (10),    %x (16).\n", sizesend, sizesend);
	fflush(log);

        sizesend = sizeof(struct data);
        // Send the Levin handshake request data
        if ( sendto (s, datagram1,  sizeof(struct data) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
        {
                fprintf(log, "Errorea 1001 mezuaren datuak bidaltzean. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
		fflush(log);
		return (void *) 5;
        }

        //m = inet_ntoa(dest.sin_addr);
        fprintf(log, "1001 eskaeraren datuak bidalita \t\t Pakete tamaina: %d (10),   %x (16).\n", sizesend, sizesend);
        //printf("==========================================================================\n");
	fflush(log);


	// RECV
	unsigned char *recbuf0 = (unsigned char*)malloc(33); // ignore 1007 request (33+10=43 bytes)
	unsigned char *recbufi = (unsigned char*)malloc(10); // 1007 data
	unsigned char *recbufh = (unsigned char*)malloc(33); // 1001 header
	int sized = sizeof(dest);
	int b0 = recvfrom(s, recbuf0, 33, 0, (struct sockaddr *) &dest, &sized); // ignore

	if (b0 < 33) {
       		fprintf(log, "Erantzuna 1007 ezin jaso (recvfrom pakete tamaina: %d): %s \n", b0, strerror(errno));
		fflush(log);
		return (void *) 6;
       	}

	char fileizena[50];
	strcpy(fileizena, target);
	FILE *em = fopen(fileizena, "w");
	if(em == NULL)
	{
		fprintf(log, "Errorea irteera fitxategia sortzean: %s\n", strerror(errno));
		fflush(log);
		return (void *) 7;
	}

	unsigned char *recbufd;
	char com0[7];
	sprintf(com0, "0x%.2x%.2x", recbuf0[18],recbuf0[17]);
	long como = strtol(com0, NULL, 16);
	char leng[7]; // mezuaren tamaina hamaseitarrez gordetzeko
	long lend;

	if(como == 1007) { // support flags mezua
		char leng0[7];
		sprintf(leng0, "0x%.2x%.2x", recbuf0[9],recbuf0[8]); // jaso tamaina
		long lend0 = strtol(leng0, NULL, 16); // 1007 datuak 10 byte (lend0)
		if(recvfrom(s, recbufi, lend0, 0, (struct sockaddr *) &dest, &sized) <= 0) return (void *) 6; // ignore
		fprintf(log, "1007 eskaera jasota (baztertu) \t\t\t Pakete tamaina: %ld (10),    %02lx (16).\n", lend0, lend0);

		// jaso 1001 goiburukoa (1007 ondoren)
		int bh = recvfrom(s, recbufh, 33, 0, (struct sockaddr *) &dest, &sized); // 1001 header

		if (bh <= 0) {
               		fprintf(log, "Erantzuna 1001 ezin jaso (recvfrom pakete tamaina: %d): %s \n", bh, strerror(errno));
			fflush(log);
			return (void *) 6;
        	}

		// datuen tamaina bihurtu eta espazio hori erreserbatu
		fprintf(log, "1001 erantzunaren goiburukoa jasota \t\t Pakete tamaina: %d (10),    %x (16).\n", bh, bh);

		sprintf(leng, "0x%.2x%.2x", recbufh[9],recbufh[8]);
		lend = strtol(leng, NULL, 16); // bihurtu mezuaren tamaina hamartarrera
		recbufd = (unsigned char*)malloc(lend); // allocate data buffer
		// header-a fitxategian idatzi
		fwrite(recbufh, sizeof(char), 33, em);


	}
	else if(como == 1001) { // agian lehenengo 1001 jaso da 1007 ordez...

		// datuen tamaina bihurtu eta espazio hori erreserbatu
		fprintf(log, "1001 erantzunaren goiburukoa jasota \t\t Pakete tamaina: %d (10),    %x (16).\n", b0, b0);

		sprintf(leng, "0x%.2x%.2x", recbuf0[9],recbuf0[8]);
		lend = strtol(leng, NULL, 16); // bihurtu mezuaren tamaina hamartarrera
		recbufd = (unsigned char*)malloc(lend); // allocate data buffer
		// header-a fitxategian idatzi
		fwrite(recbuf0, sizeof(char), 33, em);


	}
	else { // besterik jaso bada baztertu
		fprintf(log, "Baztertutako mezua: %ld", como);
		fflush(log);
		return (void *) 6;
	}

	// jaso 1001 datuak (memoria erreserbatu ondoren)
	int bd=1;
	int it=0;

	fprintf(log, "1001 erantzunaren datuak jasotzen... \t\t Pakete tamaina: %ld (10), %lx (16)\t\t '%s' fitxategian\n", lend, lend, target);

	while(bd > 0) {
		bd = recvfrom(s, recbufd, lend, 0, (struct sockaddr *) &dest, &sized);
		if (bd <= 0){
			fprintf(log, "1001 datuak jasotzean errorea (recvfrom, tamaina %d): %s \n", bd, strerror(errno));
			return (void *) 8;
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
	fflush(log);

	close(s); // socket-a itxi

	// system ordezkatu popen pipe bat irekitzen eta fget-ekin erantzuna jasoz (ez fitx berri)
	// kargatu komandoa jasotakoa hamaseitarrera bihurtzeko
	sprintf(combuf, "hexdump -C %s", fileizena);
	strcat(combuf, " | cut -d \' \' -f 2-19 | tr -s \'\\n\' \' \' | grep -oE \'04 61 64 64 72 0c 08 04 6d 5f 69 70 06 ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} 06 6d 5f 70 6f 72 74 07 ([0-9a-f]){2} ([0-9a-f]){2} 04\' | sed \'s/04 61 64 64 72 0c 08 04 6d 5f 69 70 / /g\' > ");
	strcat(combuf, fileizena);
	strcat(combuf, "a"); // fitxategi berri batean gorde komandoaren irteera (fileizena+'a')
	//fprintf(log, "%s\n", combuf);
	//fflush(stdout);

	// exekutatu komandoa
        // ireki pipe-a komandoa exekutatzeko
        FILE *pp1;

        if ((pp1 = popen(combuf, "r")) == NULL) {
                fprintf(log, "Errorea pipe1 irekitzean.\n");
                fflush(log);
                return (void *) 11;
        }

        //fprintf(log, "Pipe1 ongi ireki da.\n");
        //fflush(log);

        while(fgets(ler, LSIZE, pp1) != NULL) {
                ;//fwrite();
        }

        if(pclose(pp1))  {
                fprintf(log, "Komando ezezaguna edo pipe1 ustekabean itxi da.\n");
                fflush(log);
                return (void *) 12;
        }


//	system(combuf);

	// berdina IPv6 (mapped IPv4) ateratzeko fitx berean
	sprintf(combuf, "hexdump -C %s", fileizena);
	strcat(combuf, " | cut -d \' \' -f 2-19 | tr -s \'\\n\' \' \' | grep -oE \'04 61 64 64 72 0c 08 04 61 64 64 72 0a 40 00 00 00 00 00 00 00 00 00 00 ff ff ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} ([0-9a-f]){2} 06 6d 5f 70 6f 72 74 07 ([0-9a-f]){2} ([0-9a-f]){2}\' | sed \'s/04 61 64 64 72 0c 08 04 61 64 64 72 0a 40 00 00 00 00 00 00 00 00 00 00 ff ff/006/g\' >> ");
	strcat(combuf, fileizena);
	strcat(combuf, "a"); // fitxategi berean gorde komandoaren irteera (fileizena+'a')
	//fprintf(log, "%s\n", combuf);
	//fflush(stdout);
	// exekutatu komandoa

	FILE *pp2;
        if ((pp2 = popen(combuf, "r")) == NULL) {
                fprintf(log, "Errorea pipe2 irekitzean.\n");
                fflush(log);
                return (void *) 11;
        }

        //fprintf(log, "Pipe2 ongi ireki da.\n");
        //fflush(log);

        while(fgets(ler, LSIZE, pp2) != NULL) {
                ;//fwrite();
        }

        if(pclose(pp2))  {
                fprintf(log, "Komando ezezaguna edo pipe2 ustekabean itxi da.\n");
                fflush(log);
                return (void *) 12;
        }

//	system(combuf);

	// irakurri komandoarekin idatzi berri den fitxategia
	FILE *fi;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	strcat(fileizena, "a");
	fi = fopen(fileizena, "r");
	if(fi == NULL) {
		fprintf(log, "Errorea irteera fitxategia irekitzean: %s\n", strerror(errno));
		fflush(log);
		return (void *) 9;
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
	// nodoari 1001 eskatu zaio, 0 egoeratik 1 egoerara pasa:
	//while(egoera_lortu(dest_ip) != 0) {
	//	sleep(1);
	//}

	// egoera aldatu return 0 ondoren
/*	if(egoera_aldatu(dest_ip, 1) == 0) fprintf(log, "Ondo.\n");
	else {
		fprintf(log, "Ezin egoera aldatu\n");
		return (void *) 11;
	}
*/
	//free(combuf);
	free(recbufd);
	free(recbufh);
	free(recbuf0);
	fflush(log);
	//fclose(log);
	//pthread_exit(NULL);
	return (void *) 0;
}

/*
 *
*
int jaso1001(unsigned char * buff, long tam, FILE *log) {

}
*/




