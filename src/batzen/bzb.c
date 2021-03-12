#include "nagusia.h"
#include "eskatu1001.h"
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
	fprintf(fp, "\n");
	fflush(fp);
        if(root->left != NULL ) inprimatu(root->left, 1, fp);
        fprintf(fp, "     < %s  %d %d  >   \n",inet_ntoa(root->nodip), root->port, root->egoe);
        if(root->right != NULL) inprimatu(root->right, 1, fp);
	fclose(fp);
}

void inprimatu(struct bzb_ip *uneko, int k, FILE *fp) {
        if(uneko->left != NULL ) inprimatu(uneko->left, k+1, fp);
        fprintf(fp, " %d  %s  %d \n", k, inet_ntoa(uneko->nodip), uneko->port);
        if(uneko->right != NULL) inprimatu(uneko->right, k+1, fp);
}

/* Elementua aurkitu eta zuhaitzetik nodoa ezabatu (egoera 3 denean)
 * return 0 ondo ezabatu bada
 * return 1 elementua ezabatu ez bada (ez zegoen, adibidez)
*/
int ezabatu_elem(struct in_addr ip) {

	struct bzb_ip *hel = aurkitu(root, ip);

	if(hel == NULL) {
		return 1; // ez da aurkitu nodoa
	}
	if(pthread_mutex_lock(&(hel->lock)) != 0) return 1; // ezin...
	// ezab.
	pthread_mutex_unlock(&(hel->lock));
	return 0;
}

/* Nodoaren egoera aldatzeko metodoa
 * 0: aurkituta
 * 1: eskatuta
 * 2: kokatuta
 * 3: ezabatzeko
 * Eragiketa honek atomikotasuna mantendu dezan, pthread_lock() metodoa erabiltzen da
*/
int egoera_aldatu(struct in_addr ip, int egb) {
	struct bzb_ip *hel = aurkitu(root, ip);
	if(hel == NULL) {
		return 1; // ez da aurkitu nodoa
	}
	if(pthread_mutex_lock(&(hel->lock)) != 0) return 1; // ezin...
	hel->egoe = egb; // egoera berria gorde
	pthread_mutex_unlock(&(hel->lock));
	return 0;
}

/* Nodoaren koordenatuak gordetzeko metodoa
 *
*/
int koordenatuak_gehitu(struct in_addr ip, float x, float y) {
	struct bzb_ip *hel = aurkitu(root, ip);
        if(hel == NULL) {
                return 1; // ez da aurkitu nodoa
        }
        if(pthread_mutex_lock(&(hel->lock)) != 0) return 1; // ezin...
        hel->lon = x; // longitudea gorde
        hel->lat = y; // latitudea gorde
        pthread_mutex_unlock(&(hel->lock));
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

/* 
 * 
*/
int egoera_lortu(struct in_addr ip) {
	struct bzb_ip *h = aurkitu(root, ip);
        if(h == NULL) {
                return -1; // ez da aurkitu nodoa
        }
	return h->egoe;
}



