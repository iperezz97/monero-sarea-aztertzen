/**********  nagusia.c  **********

 * Programa hau exekutatuz, monero sarearen azterketari hasiera ematen zaio:
 * *  - 0. haria: hari honek gainerako hariak sortuko ditu eta lortutako informazioa (IP helbideak eta portuak) zuhaitz bitar ordenatu batean gordeko du.
 * *  - 1. haria: Nodo bakoitzeko bere bizilagunak (IP helbideak eta portuak) lortzen dituen haria (send/recv 1001)
 * *  - 2. haria: Nodo bakoitza moneroren sarean atzigarri jarraitzen duen aztertzen duen haria (send/recv 1003)
 * *  - 3. haria: Nodo bakoitzaren koordenatuak lortu eta mapan kokatzen dituen haria (geoip, mapan marraztu)
 * *  - 4. haria: Transakzioak jaso eta nodoekin erlazionatzen dituen haria (recv 2002, mapan marraztu, konexio kopurua?)

 * Konpilatu: gcc nagusia.c bzb.c eskatu1001.c konprob1003.c kokatu_mapan.c -lpthread -o nagusia
 * Exekutatu: sudo ./nagusia <IP> <PORT>
 * Lehenengo argumentua hasierako helburuko nodoaren IP helbidea izango da
 * Bigarren argumentua nodoaren portua izango da (zenbaki naturala, ohikoa 18080)
 * Ez da beharrezkoa monero sarera konektatuta egotea hau exekutatzeko

SEED_NODES = https://community.xmr.to/xmr-seed-nodes

*/

#include "nagusia.h"
#include "eskatu1001.h"
#include "bzb.h"
#include "konprob1003.h"
#include "kokatu_mapan.h"


// bzb_ip datu-egitura nagusia hasieratzeko metodoa //
void init(char *argv1, char *argv2) {
	root = malloc(sizeof *root);
	root->nodip.s_addr = inet_addr(argv1);
	root->port = atoi(argv2);
	root->egoe = 0;//
	root->lon = 0;
	root->lat = 0;
	root->left = NULL;//
	root->right = NULL;//

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
		exit(1);
	}

	// log fitxategia sortu
	FILE *log;
	log = fopen("log0", "w");
	if(log == NULL) {
		printf("Error creating log file: %s\n", strerror(errno));
	}

	char *h_ip = inet_ntoa(root->nodip);
	fprintf(log, "Jasotako argumentuak: %s %d\r\n", h_ip, root->port);
	//printf("STRING %d", root.nodip.s_addr);

	// Harien sorrera //
	char *message1 = (char *)malloc(50);
	sprintf(message1, "%s", h_ip);
	char sport[7];
	sprintf(sport, "%d", root->port);
	strcat(message1, " ");
	strcat(message1, sport);

	pthread_t thread1;

	if( pthread_create( &thread1 , NULL , eskatu_ip , (void*) message1) < 0)
	{
		fprintf (log, "Could not create thread1. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		exit(0);
	}

	char *message2 = (char *)malloc(50);
	sprintf(message2, "212.83.172.165 18080");
	////strcat(message2, " ");
	////strcat(message2, sport);
	pthread_t thread2;
	fflush(log);

	if( pthread_create( &thread2 , NULL , konprobatu_ping , (void*) message2) < 0)
	{
		fprintf (log, "Could not create thread2. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		exit(0);
	}

	char *message3 = (char *)malloc(50);
	sprintf(message3, "66.85.74.134 18080");
	pthread_t thread3;

	if( pthread_create( &thread3 , NULL , kokatu_mapan , (void*) message3) < 0)
	{
		fprintf (log, "Could not create thread3. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		exit(0);
	}

	char *message4 = "Thread 2002";
	pthread_t thread4;

	if( pthread_create( &thread4 , NULL , jaso_transakzioak , (void*) message4) < 0)
	{
		fprintf (log, "Could not create thread4. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		exit(0);
	}

	//sleep(5);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
//	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	//fprintf(log, "\n");
	//fflush(stdout);
	bzb_inprimatu();
	free(message1);
	free(message2);
	//free(message3);
	//free(message4);
	return 0;
}



// egiteko
void * jaso_transakzioak(void *args) {
//	printf("jaso_transakzioak %s exekutatzen...\n", (char *) args);
	pthread_exit(NULL);
}
