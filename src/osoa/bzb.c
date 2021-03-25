#include "nagusia.h"
#include "bzb.h"

/* Zuhaitz bitarrean elementu bat txertatzeko metodoa O(log n)
 * return 0 ondo txertatu denean
 * return 1 ez bada txertatu (adib. bazegoen IP hori zuhaitzean)
*/
int txertatu_elem(struct in_addr ip, int port) {

        struct bzb_ip *uneko = root;
        struct bzb_ip *berria = malloc (sizeof *berria);
        berria->nodip.s_addr = ip.s_addr;
        berria->port = port;
        berria->egoe = 0; // aurkituta, lehen egoera (0)
        berria->lon = 0;
	berria->lat = 0;
	berria->left = NULL;
        berria->right = NULL;
	pthread_mutex_init(&(berria->lock), NULL);
	pthread_cond_init (&(berria->cond), NULL);
//      printf("%u eta %u \n", ntohl(uneko->nodip.s_addr), ntohl(berria->nodip.s_addr));
        // ordenatu ntohl-ren emaitzaren arabera...
        while(uneko != NULL) {
                unsigned int txert = ntohl(berria->nodip.s_addr);
                unsigned int lehen = ntohl(uneko->nodip.s_addr);
                if(lehen > txert) {     // ezkerrean txertatu
                        if(uneko->left == NULL) {
                                uneko->left = berria;
                                return 0;
                        }
                        else uneko = uneko->left;
                }
                else if(lehen < txert){ // eskubian txertatu
                        if(uneko->right == NULL) {
                                uneko->right = berria;
                                return 0;
                        }
                        else uneko = uneko->right;
                }
		else { // ip berdinak diren kasuan
			break;  // bilaketa bukatu
		}
		// Nodoa IP-arekin identifikatu, (edo ip eta portua?)...... ez dut beharrezkoa ikusten
		// IP-ak berdinak diren kasuetan... ez gorde(?) (bi monero nodo IP bakarrarekin?)
/*		else if(uneko->port > berria->port) {
			if(uneko->left == NULL) {
                                uneko->left = berria;
                                return 0;
                        }
                        else uneko = uneko->left;
		}
		else if(uneko->port < berria->port) {
			if(uneko->right == NULL) {
                                uneko->right = berria;
                                return 0;
                        }
                        else uneko = uneko->right;
		}
*/
        }
        return 1;

}


/* Bilaketa zuhaitz bitarra inprimatzeko metodoa: in-order ('logbzb' fitxategian)
 * Nodoaren maila, nodoaren IP eta nodoaren portua inprimatuko dira
*/
void bzb_inprimatu() {
	FILE *fp = fopen("logbzb", "w");
	if(fp==NULL) {
		printf("bzb file error");
		exit(1);
	}
	fprintf(fp, "Sak. \t  IPv4 \t\tPort   Eg.   Lon     \t   Lat\n");
	fflush(fp);

        if(root->left != NULL ) inprimatu(root->left, 1, fp);
        fprintf(fp, "     < %15.15s  %5d  %d  %11f\t%11f  >   \n",inet_ntoa(root->nodip), root->port, root->egoe, root->lon, root->lat);
        if(root->right != NULL) inprimatu(root->right, 1, fp);
	fclose(fp);

}

void inprimatu(struct bzb_ip *uneko, int k, FILE *fp) {
        if(uneko->left != NULL ) inprimatu(uneko->left, k+1, fp);
        fprintf(fp, " %2d  %15.15s \t%5d  %d  %11f\t%11f\n", k, inet_ntoa(uneko->nodip), uneko->port, uneko->egoe, uneko->lon, uneko->lat);
        if(uneko->right != NULL) inprimatu(uneko->right, k+1, fp);
}

/* Nodoaren egoera aldatzeko metodoa
 * 0: aurkituta (Handshake batekin)
 * 1: eskatuta IP-ak (Handshake bidalita)
 * 2: ezabatzeko (ez da PING erantzunik jaso)
 * Eragiketa honek atomikotasuna mantendu dezan, pthread_mutex_lock(), pthread_cond_signal() eta pthread_mutex_unlock() metodoak erabiltzen dira
*/
int egoera_aldatu(struct in_addr ip, int egb) {
	struct bzb_ip *hel = aurkitu(root, ip);
	if(hel == NULL) {
		return 1; // ez da aurkitu nodoa
	}
	//if(pthread_mutex_lock(&(hel->lock)) != 0) return 1; // ezin...
	hel->egoe = egb; // egoera berria gorde
	//pthread_cond_signal (&(hel->cond)); // itxaroten dagoenari abisatu
	//pthread_mutex_unlock(&(hel->lock)); // desblokeatu
	return 0;
}


/* Nodoa zuhaitzean aurkitzen duen metodoa
 * return zuhaitzaren elementua
*/
struct bzb_ip * aurkitu(struct bzb_ip *unek, struct in_addr x) {
	if(unek==NULL || ntohl(unek->nodip.s_addr) == ntohl(x.s_addr)) // elementua aurkitu da edo null (ez dago)
		return unek;
	else if(ntohl(x.s_addr) > ntohl(unek->nodip.s_addr)) // aurkitu beharrekoa (x) handiagoa da, eskuinean aurkituko da
		return aurkitu(unek->right, x);
	else // aurkitu beharrekoa (x) txikiagoa da, ezkerrean aurkituko da
		return aurkitu(unek->left, x);
}

// ez da beharrezkoa ez..?
/* Nodoaren egoera itzultzen duen funtzioa
 * Egoera 0 denean 1001 bidaliko zaio (1 egoerara pasaz)
 * Egoera 1 denean mapan kokatuko da (2 egoerara pasaz) eta 1003 bidaliko da (3 egoerara erantzunik jasotzen ez bada)
 * Egoera 2 denean mapan kokatuta egongo da eta 1003 bidaltzen jarraituko da (3 egoerara erantzunik jasotzen ez bada)
 * Egoera 3 denean mapatik ezabatuko da, ez da berriro eskatuko baina zuhaitzan geratuko da gordeta
*/
int egoera_lortu(struct in_addr ip) {
	struct bzb_ip *h = aurkitu(root, ip);
	int egoera=0;
        if(h == NULL) {
                return -1; // ez da aurkitu nodoa
        }
	//if(pthread_mutex_lock(&(h->lock)) != 0) return -1;
	egoera = h->egoe; // balio konsistentea
	//pthread_mutex_unlock(&(h->lock));
	return egoera;
}



