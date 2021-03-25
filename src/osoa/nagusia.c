/**********  nagusia.c  **********

 * Programa hau exekutatuz, monero sarearen azterketari hasiera ematen zaio:
 * *  - 0. haria: hari honek gainerako hariak sortuko ditu eta lortutako informazioa (IP helbideak eta portuak) zuhaitz bitar ordenatu batean gordeko du.
 * *  - 1. haria: Nodo bakoitzeko bere bizilagunak (IP helbideak eta portuak) lortzen dituen haria (send/recv 1001)
 * *  - 2. haria: Nodo bakoitza moneroren sarean atzigarri jarraitzen duen aztertzen duen haria (send/recv 1003)
 * *  - 3. haria: Nodo bakoitzaren koordenatuak lortu eta mapan kokatzen dituen haria (geoip, mapan marraztu)
 * * (+ 4. haria:)Transakzioak jaso eta nodoekin erlazionatzen dituen haria (recv 2002, mapan marraztu, konexio kopurua?)

 * Konpilatu: gcc nagusia.c bzb.c eskatu1001.c konprob1003.c kokatu_mapan.c -lpthread -o nagusia
 * Exekutatu: sudo ./nagusia <IP> <PORT>
 * Lehenengo argumentua hasierako helburuko nodoaren IP helbidea izango da
 * Bigarren argumentua nodoaren portua izango da (zenbaki naturala, ohikoa 18080)
 * Ez da beharrezkoa monero sarera konektatuta egotea hau exekutatzeko

SEED_NODES = https://community.xmr.to/xmr-seed-nodes

*/

#include "nagusia.h"
#include "bzb.h"
#include "eskatu1001.h"
#include "konprob1003.h"
#include "kokatu_mapan.h"


// bzb_ip datu-egitura nagusia hasieratzeko metodoa //
void init(char *argv1, char *argv2) {
	root = malloc(sizeof *root);
	root->nodip.s_addr = inet_addr(argv1);
	root->port = atoi(argv2);
	root->egoe = 0;
	root->lon = 0;
	root->lat = 0;
	//root->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	//root->cond = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
	root->left = NULL;
	root->right = NULL;

}

// main function //
int main(int argc, char *argv[]) {

	// Argumentuen balidazioa //
	if(argc != 3 || inet_addr(argv[1]) == -1 || atoi(argv[2]) == 0) {
		printf(B_RED"Helburuko IP helbidea eta portua beharrezkoak dira exekuzioa hasteko!\n"RESET
		       "Zerrenda honetatik aukeratu dezakezu: \n"
   		       "  - 66.85.74.134 18080\n"
	   	       "  - 88.198.163.90 18080\n"
	               "  - 95.217.25.101 18080\n"
	   	       "  - 104.238.221.81 18080\n"
	   	       "  - 192.110.160.146 18080\n"
		       "  - 209.250.243.248 18080\n"
		       "  - 212.83.172.165 18080\n"
		       "  - 212.83.175.67 18080\n" );
		exit(1);
	}

	// Hasieratu datu-egitura nagusia //
	init(argv[1], argv[2]);

	// Hasieratu mutex-a //
	if(pthread_mutex_init(&(root->lock), NULL) != 0) {
		printf("mutex init failed\n");
		fflush(stdout);
		exit(1);
	}
	if(pthread_cond_init (&(root->cond), NULL) != 0) {
		printf("cond init failed\n");
		fflush(stdout);
		exit(1);
	}

	// log fitxategia sortu
	FILE *log;
	log = fopen("log0", "w");
	if(log == NULL) {
		printf("Errorea log0 fitxategia sortzean: %s\n", strerror(errno));
		fflush(stdout);
	}

	char *h_ip = inet_ntoa(root->nodip);
	fprintf(log, "Jasotako argumentuak: %s %d\r\n", h_ip, root->port);

	// Harien sorrera //
	char *message1 = (char *)malloc(50);
	sprintf(message1, "%s", h_ip);
	char sport[7];
	sprintf(sport, "%d", root->port);
	strcat(message1, " ");
	strcat(message1, sport);

	pthread_t thread1;

	if( pthread_create( &thread1 , NULL , hasieratu1001 , (void*) message1) < 0)
	{
		fprintf (log, "Ezin izan da thread1 haria sortu. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	char *message2 = (char *)malloc(50);
	sprintf(message2, "%s", h_ip);
	strcat(message2, " ");
	strcat(message2, sport);
	pthread_t thread2;

	if( pthread_create( &thread2 , NULL , hasieratu1003 , (void*) message2) < 0)
	{
		fprintf (log, "Ezin izan da thread2 haria sortu. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	char *message3 = (char *)malloc(50);
	sprintf(message3, "%s", h_ip);
	strcat(message3, " ");
	strcat(message3, sport);
	pthread_t thread3;

	if( pthread_create( &thread3 , NULL , hasieratu_kokapena , (void*) message3) < 0)
	{
		fprintf (log, "Ezin izan da thread3 haria sortu. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	// inplementatu gabea
	char *message4 = "Thread 2002";
	pthread_t thread4;

	if( pthread_create( &thread4 , NULL , jaso_transakzioak , (void*) message4) < 0)
	{
		fprintf (log, "Ezin izan da thread4 haria sortu. Errore zenbakia: %d   Errore mezua: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	// harien exekuzioa bukatu denean horiek elkartu
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	//fprintf(log, "\n");
	//fflush(stdout);
	bzb_inprimatu(); // logbzb fitxategian idatzi egitura nagusia
	free(message1);
	free(message2);
	free(message3);
	//free(message4);
	return 0;
}



// egiteko
void * jaso_transakzioak(void *args) {
//	printf("jaso_transakzioak %s exekutatzen...\n", (char *) args);
	pthread_exit(NULL);
}




/*
 *
*/
int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, unsigned int timeout_ms) {
        int rc = 0;
        // Set O_NONBLOCK
        int sockfd_flags_before;
        if((sockfd_flags_before=fcntl(sockfd,F_GETFL,0)<0)) return -1;
        if(fcntl(sockfd,F_SETFL,sockfd_flags_before | O_NONBLOCK)<0) return -1;
        // Start connecting (asynchronously)
        do {
                if (connect(sockfd, addr, addrlen)<0) {
                        // Did connect return an error? If so, we'll fail.
                        if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
                                rc = -1;
                        }
                        // Otherwise, we'll wait for it to complete.
                        else {
                                // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be interrupted)
                                struct timespec now;
                                if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-2; break; }
                                struct timespec deadline = { .tv_sec = now.tv_sec,
                                                             .tv_nsec = now.tv_nsec + timeout_ms*1000000l};
                                // Wait for the connection to complete.
                                do {
                                        // Calculate how long until the deadline
                                        if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-3; break; }
                                        int ms_until_deadline = (int)(    (deadline.tv_sec  - now.tv_sec)*1000l
                                                                        + (deadline.tv_nsec - now.tv_nsec)/1000000l);
                                        if(ms_until_deadline<0) { rc=0; break; }
                                        // Wait for connect to complete (or for the timeout deadline)
                                        struct pollfd pfds[] = { { .fd = sockfd, .events = POLLOUT } };
                                        rc = poll(pfds, 1, ms_until_deadline);
                                        // If poll 'succeeded', make sure it *really* succeeded
                                        if(rc>0) {
                                                int error = 0; socklen_t len = sizeof(error);
                                                int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                                                if(retval==0) errno = error;
                                                if(error!=0) rc=-4;
                                        }
                                }
                                // If poll was interrupted, try again.
                                while(rc<=-1 && errno==EINTR);
                                // Did poll timeout? If so, fail.
                                if(rc==0) {
                                        errno = ETIMEDOUT;
                                        rc=-5;
                                }
                        }
                }
        } while(0);
        // Restore original O_NONBLOCK state
        if(fcntl(sockfd,F_SETFL,sockfd_flags_before)<0) return -1;
        // Success rc=0
        return rc;
}

