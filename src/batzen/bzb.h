
// Methods
int txertatu_elem(struct in_addr ip, int port);
void bzb_inprimatu();
void inprimatu(struct bzb_ip *uneko, int k, FILE *fp);
int ezabatu_elem(struct in_addr ip); // egoera 3 denean nodoa ezabatu!!!
int egoera_aldatu(struct in_addr ip, int egb);
int koordenatuak_gehitu(struct in_addr ip, float x, float y);
struct bzb_ip * aurkitu(struct bzb_ip *unek, struct in_addr ip);
int egoera_lortu(struct in_addr ip);

