#include "nagusia.h"
#include "bzb.h"
#include "kokatu_mapan.h"
#include <time.h>

char bufc[128];

void * kokatu_mapan(void *args) {

/*	const char* pipeName = "./fifoKanala";       // uneko direktorioan
	mkfifo(pipeName, 0666);                      // baimenak: irak + idatz. erabilt/talde/beste.-entzat
	int fd = open(pipeName, O_CREAT | O_WRONLY); // idazteko bakarrik ireki
	if (fd < 0) {
		printf("Ezin izan da fifo kanala eraiki.\n");  // errorea
		exit(0); // thread exit...
	}
*/
	sleep(5);

	char *ip = strtok((char *)args, " ");
	sprintf(bufc, "geoiplookup -f ./geoip/maxmind4.dat %s | cut -d ',' -f 7-8 | sed s/,//g | awk '{print $2, $1}' >1", ip);
        //printf("%s\n", bufc);
	// Koordenatuak atera geoip-rekin
	printf("a ");
	system(bufc);

	//close(fd);           // itxi deskriptorea (pipe-a)
	//unlink(pipeName);
	pthread_exit(NULL);


}
