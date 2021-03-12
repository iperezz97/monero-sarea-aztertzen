#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define B_RED "\033[1m\033[31m"
#define RESET "\033[0m"

// Bilaketa zuhaitz bitarra: datu-egitura nagusia lortutako informazioa gordetzeko
struct bzb_ip {
        struct in_addr    nodip; // uint32_t s_addr (nodoaren IP helbidea)
        int               port;  // nodoaren portu zenbakia
        int               egoe;  // nodoaren egoera: aurkituta (0), eskatuta(1), mapan kokatuta (2), ezabatu beharra (3)
	float		  lon;   // nodoaren longitudea (mapan kokatzeko)
	float		  lat;   // nodoaren longitudea (mapan kokatzeko)
	pthread_mutex_t   lock;  // atomikotasuna bermatzeko mutex-a (ezabatu_elem eta egoera_aldatu metodoetan beharrezkoa)
        struct bzb_ip   * left;  // ezkerreko umea (IP txikiagoa duen nodoa)
        struct bzb_ip   * right; // eskuineko umea (IP handiagoa duen nodoa)
};

// Datu-egitura nagusia (globala)
struct bzb_ip *root;

// Erantzuna gordetzeko datu-egitura
struct ipport {
        struct in_addr  ip; // 1001 komandoarekin lortuko diren IP helbideak gordetzeko
        int             port; // portuak gordetzeko
};
struct h_erantzuna {
        struct ipport ip_port[250];
};

// Methods + int main()
void init(char *argv1, char *argv2);
//void * konprobatu_ping(void *);//mugitu
//void * kokatu_mapan(void *);//
void * jaso_transakzioak(void *);//


