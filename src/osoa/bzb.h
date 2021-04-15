// Bilaketa zuhaitz bitarra: datu-egitura nagusia lortutako nodoen informazioa gordetzeko
struct bzb_ip {
        struct in_addr    nodip; // barnean uint32_t s_addr (nodoaren IP helbidea)
        int               port;  // nodoaren portu zenbakia
        int               egoe;  // nodoaren egoera: aurkituta (0), eskatuta(1), mapan kokatuta (2), ezabatu beharra (3)
        float             lon;   // nodoaren longitudea (mapan kokatzeko)
        float             lat;   // nodoaren longitudea (mapan kokatzeko)
        pthread_mutex_t   lock;  // atomikotasuna bermatzeko mutex-a (ezabatu_elem eta egoera_aldatu metodoetan beharrezkoa)
//	pthread_cond_t	  cond;  // baldintza bat bete arte itxaroteko (wait/signal)
        struct bzb_ip   * left;  // ezkerreko umea (IP txikiagoa duen nodoa)
        struct bzb_ip   * right; // eskuineko umea (IP handiagoa duen nodoa)
};

// Datu-egitura nagusia (globala)
struct bzb_ip *root;

// Methods
int txertatu_elem(struct in_addr ip, int port);
void bzb_inprimatu();
void inprimatu(struct bzb_ip *uneko, int k, FILE *fp);
int egoera_aldatu(struct in_addr ip, int egb);
struct bzb_ip * aurkitu(struct bzb_ip *unek, struct in_addr ip);
int egoera_lortu(struct in_addr ip);

