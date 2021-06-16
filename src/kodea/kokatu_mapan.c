#include "nagusia.h"
#include "bzb.h"
#include "kokatu_mapan.h"

char bufc[BSIZE];
char coords[CSIZE];
struct in_addr uneko;
int nodomap = 0;

/* Nodo (IP) bakoitzaren kokapena hasieratzeko metodoa
 * Hasteko zuhaitzaren lehen elementua behar du soilik, bzb-n modulu nagusiak txertatzen du hari hauek sortu aurretik
 * Egoera 0: Aurkitua izan da beste nodo bati 1001 bidaliz (egituran txertatuta)
 * Egoera 1: pareen zerrenda eskatu zaio (1001: Handshake request) -> PING bidaliko zaio konexioa aztertzeko
 * Egoera 2: nodoak ez du erantzun (1001 edo 1003) -> mapatik ezabatuko da
 * Egoera 3: mapatik ezabatu da (egituratik ez)
 * Azken hau ere gerta daiteke geoip komandoarekin erantzunik lortu ez bada, kasu horretan nodoaren sarraila itxi eta 3 egoerara pasako da nodoa
*/
void * hasieratu_kokapena(void *args) {
	// log fitxategia ireki
	FILE *log;
	log = fopen("logmap", "w");
	if(log == NULL) {
		printf("Errorea logmap fitxategia sortzean: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	while(1) {
		hautatu_mnodoa(root, log);
	}

	// ez da iristen
	fclose(log);
	pthread_exit(NULL);
}

/* Metodo errekurtsibo honekin mapan kokatuko den nodoa aukeratuko da (pre-order)
 * Nodoa 0 edo 1 egoeran dagoenean mapan gehituko da ('a <LONG> <LAT>' argumentuak pipe-ra pasaz) -> koordenatuak gehitu
 * Aldiz 2 egoeran dagoenean, mapatik nodoa ezabatuko da ('r <LONG> <LAT>' argumentuak pipe-ra pasaz) -> egoera = 3 izatera pasako da errepikapena sahiestuz
 * Gainera, nodoaren koordenatuak gehituko dira egitura nagusian, nodoa mapan kokatua izan dela jakiteko
*/
void * hautatu_mnodoa(struct bzb_ip *un, FILE *log) {
	long err = 0;
	int nodokop;
    if(un == NULL) {
        //sleep(1);
        ;
    }
    else {
        if(un->egoe == 2) { // ezabatu beharra
			// ez du inork erabiliko nodoa egoera honetan (ez blokeatu)
			if(un->lon != 0 || un->lat != 0) { // puntua mapan gehitu ez bada, ez bidali remove eragiketa
				printf("r %f %f %s 0\n", un->lon, un->lat, inet_ntoa(un->nodip)); // city ez da behar
				fflush(stdout);
				nodomap--;
			}
			pthread_mutex_lock(&(un->lock));   // blokeatu aldaketak eragina duelako besteen azterketan
			un->egoe = 3; // mapatik ezabatuta (1001 edo 1003 erantzunik gabe)
			pthread_mutex_unlock(&(un->lock)); // askatu
		}
		else if(un->lon == 0 && un->lat == 0 && un->egoe != 3) {
            //pthread_mutex_lock(&(un->lock)); // blokeakorra (wait-ekin kontrolatu daiteke)
            //pthread_mutex_trylock(&(un->lock)); // ez blokeakorra
            err = ( long ) kokatu_mapan(un, log);
            if(err > 0) { // ez da ohikoa
				nodomap--;
				fprintf(log, B_RED"Errorea: %ld\n"RESET, err);
				fflush(log);
				pthread_mutex_lock(&(un->lock));   // blokeakorra (wait-ekin kontrolatu daiteke)
				un->egoe = 3;  // ezin koordenatuak aurkitu (geoip-ren erantzuna hutsa)
				pthread_mutex_unlock(&(un->lock)); // askatu nodoaren sarraila
			}
			else {
				nodomap++;
			}
            //pthread_cond_wait(&(un->cond), &(un->lock)); // signal-aren zain geratzeko
            //pthread_mutex_unlock(&(un->lock));
        }
		char tmp[1024];
		FILE *es = fopen("log1001", "r"); // 1. hariaren fitxategia irakurtzeko ireki
		if(es != NULL) {
			while(!feof(es)) {
				fgets(tmp, 1024, es);
			} // bukatu behar duen aztertu
			if(strcmp(tmp, "BUKATU") == 0){ // 1. hariak bukatu du (eta 2.ak)
				nodokop = kontatu_atzigarri(root);
				if(nodokop == nodomap) { // mapan kokatu dira nodo atzigarri guztiak
					fprintf(log, "BUKATU\n");
					fflush(log);
					fclose(log);
					fclose(es);
					pthread_exit(NULL); // Bukatu 3. haria (asinkronoa)
				}
			}
			fclose(es);
		}

        if(un->left != NULL) {
                hautatu_mnodoa(un->left, log);
        }
        if(un->right != NULL) {
                hautatu_mnodoa(un->right, log);
        }
    }
}


/* Maxmind-en geoip datu-base lokala erabiliz lortuko dira nodo bakoitzaren koordenatuak
 * Pipe-a ireki, komandoa exekutatu, informazio esanguratsua lortu eta pantailan idatzi (pipe-aren bidez irakurri ahal izateko)
 * logmap fitxategian nodo bakoitzaren informazioa idatziko da
*/
void * kokatu_mapan(struct bzb_ip *nodo, FILE *log) {

	char *ip = inet_ntoa(nodo->nodip);
	int port = nodo->port;

    fprintf(log, "Koordenatuak lortzen... Helburuko nodoa: %s %d\n", ip, port);
	fflush(log);

	float lon, lat;
	char *city;
	uneko.s_addr = inet_addr(ip);

	// Koordenatuak atera geoip-rekin
	sprintf(bufc, "geoiplookup -f ./geoip/maxmind4.dat %s | cut -d ',' -f 5,7-8 | awk -F ',' '{print $3, $2, $1}' | sed s/,//g ", ip); // <longitudea> <latitudea> <hiria>
    fprintf(log, "IP helbideen datuak 'geoip' karpetan deskargatzeko exekutatu 'sh get_maxmind.sh'\nPipe-an exekutatuko den komandoa: %s\n", bufc);
	fflush(log);
	// ireki pipe-a komandoa exekutatzeko
	FILE *fp;

	if ((fp = popen(bufc, "r")) == NULL) {
		fprintf(log, "Errorea pipe-a irekitzean.\n");
		fflush(log);
		return (void *) 1;
	}

    fprintf(log, "Pipe-a ongi ireki da.\n");
	fflush(log);

	while (fgets(coords, CSIZE, fp) != NULL) {
		;
	}
	//fprintf(log, "Coordinates: %s\n", coords);
	//fflush(log);

	// erantzun hutsa lortu den aztertu
	if(!coords || (coords[0] == ' ' && coords[1] == ' '  && coords[2] =='\n')) {
		return (void *) 3;
	}

	if(pclose(fp))  {
		fprintf(log, "Komando ezezaguna edo pipe-a ustekabean itxi da.\n");
		fflush(log);
		return (void *) 2;
	}


	lon = atof(strtok(coords, " "));
	lat = atof(strtok(NULL, " "));
	city = strtok(NULL, "");
	int ct=1;
	int new_char;

	while (city[ct])
	{
		//printf("%s %d",city, (int)new_char);
		new_char=city[ct];
		if(new_char == ' ')	city[ct]='_';
		if(new_char == '\n')	city[ct]='\0';

		ct++;
	}

	// koordenatuak ez dituzte beste hariek erabiltzen, ez blokeatu
	nodo->lon = lon;
	nodo->lat = lat;
	printf("a %f %f %s %s\n", lon, lat, inet_ntoa(nodo->nodip), city); // eragiketa: add point (mapan)
	fflush(stdout);

    fprintf(log, "Lon: %f\tLat: %f\tCity: %s\n----\n", lon, lat, city);
	fflush(log);

	return (void *) 0;

}

