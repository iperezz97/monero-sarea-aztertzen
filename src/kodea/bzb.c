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
	berria->tkop = -1;
	berria->left = NULL;
        berria->right = NULL;
	pthread_mutex_init(&(berria->lock), NULL);
//	pthread_cond_init (&(berria->cond), NULL);
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
		// Nodoa IP-arekin eta portuarekin identifikatu
		// IP-ak berdinak diren kasuetan...  gorde (bi monero nodo IP bakarrarekin portu desberdinarekin)
		else if(uneko->port > berria->port) {
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
		else 	break; // ip eta portu bera aurkitzean bukatu txertaketa..
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
	fprintf(fp, "Sak. \t  IPv4 \t\tPort   Eg.   Lon     \t   Lat    \t  TrKop.\n");
	fflush(fp);

        if(root->left != NULL ) inprimatu(root->left, 1, fp);
        fprintf(fp, "     < %15.15s  %5d  %d  %11f\t%11f\t  %2d  >   \n",inet_ntoa(root->nodip), root->port, root->egoe, root->lon, root->lat, root->tkop);
        if(root->right != NULL) inprimatu(root->right, 1, fp);
	fclose(fp);

}

void inprimatu(struct bzb_ip *uneko, int k, FILE *fp) {
        if(uneko->left != NULL ) inprimatu(uneko->left, k+1, fp);
        fprintf(fp, " %2d  %15.15s \t%5d  %d  %11f\t%11f\t  %2d\n", k, inet_ntoa(uneko->nodip), uneko->port, uneko->egoe, uneko->lon, uneko->lat, uneko->tkop);
        if(uneko->right != NULL) inprimatu(uneko->right, k+1, fp);
}


/* Nodoa zuhaitzean aurkitzen duen metodoa
 * return zuhaitzaren elementua
 * edo azpi-zuhaitzan bilatzen jarraitu
 * aurkitzen ez bada return NULL
*/
struct bzb_ip * aurkitu(struct bzb_ip *unek, struct in_addr x) {
	if(unek==NULL || ntohl(unek->nodip.s_addr) == ntohl(x.s_addr)) // elementua aurkitu da edo null (ez dago)
		return unek;
	else if(ntohl(x.s_addr) > ntohl(unek->nodip.s_addr)) // aurkitu beharrekoa (x) handiagoa da, eskuinean aurkituko da
		return aurkitu(unek->right, x);
	else // aurkitu beharrekoa (x) txikiagoa da, ezkerrean aurkituko da
		return aurkitu(unek->left, x);
}

/* Atzigarria izateko baldintza mapan kokatuta egotea da, ezabatzeko eragiketa ez bidali izana, zehazki
 * Maparen hariak (3. haria) exekuzioa era asinkronoan amaitzeko egingo da deia funtzio honi
 * 1,2 hariek erantzuna ez badute jaso edo mapan kokatu ezin izan bada nodoa ez da kontatuko
 * Transakzioak jasotzeko (4. haria) ere mapan dauden nodo atzigarriekin ezarriko da konexioa
*/
int kontatu_atzigarri(struct bzb_ip *uneko) {

	if(uneko == NULL) 				return 0;
	else if(uneko->egoe < 2 || uneko->egoe == 4)	return 1 + kontatu_atzigarri(uneko->left) + kontatu_atzigarri(uneko->right);
	else 						return 0 + kontatu_atzigarri(uneko->left) + kontatu_atzigarri(uneko->right);

}

