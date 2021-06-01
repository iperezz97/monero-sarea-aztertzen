// Bilaketa zuhaitz bitarra: datu-egitura nagusia lortutako nodoen informazioa gordetzeko
struct bzb_ip {
        struct in_addr    nodip; // barnean uint32_t s_addr (nodoaren IP helbidea)
        int               port;  // nodoaren portu zenbakia
        int               egoe;  // nodoaren egoera: aurkituta (0), erantzuna jasota (1), mapatik ezabatzeko (2), ezabatuta (3), transakzioak jasotzen (4).
        float             lon;   // nodoaren longitudea (mapan kokatzeko)
        float             lat;   // nodoaren latitudea (mapan kokatzeko)
	int 		  tkop;  // nodotik jaso diren 2002 mezuak (horien transakzio kopurua)
        pthread_mutex_t   lock;  // atomikotasuna bermatzeko mutex-a
//	pthread_cond_t	  cond;  // baldintza bat bete arte itxaroteko (wait/signal) ez da erabili
        struct bzb_ip   * left;  // ezkerreko azpi-zuhaitza (IP txikiagoa)
        struct bzb_ip   * right; // eskuineko azpi-zuhaitza (IP handiagoa)
};

// Datu-egitura nagusia (globala erroaren erakuslea)
struct bzb_ip *root;

// Methods
int txertatu_elem(struct in_addr ip, int port);
void bzb_inprimatu();
void inprimatu(struct bzb_ip *uneko, int k, FILE *fp);
struct bzb_ip * aurkitu(struct bzb_ip *unek, struct in_addr ip);
int kontatu_atzigarri(struct bzb_ip * uneko);
