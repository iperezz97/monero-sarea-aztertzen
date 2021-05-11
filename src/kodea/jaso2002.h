#include <sys/socket.h>

struct datatr {
	unsigned char d[226];
} __attribute__((packed));

// Methods
void * jaso_transakzioak(void *args);
void * hasieratu_trnodoa(void *args);
void * hautatu_tnodoa(struct bzb_ip *un, int id, FILE *log);
void * jaso2002(struct bzb_ip *un, FILE *log, int id);
