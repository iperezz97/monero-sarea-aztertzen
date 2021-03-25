#include <sys/socket.h>

#define ESIZE 550
#define LSIZE 48

// Levin 1001 data struct
struct data {
        unsigned char d[226];
} __attribute__((packed));

// Methods
void * hasieratu1001(void *args);
void * hautatu_enodoa(struct bzb_ip *un, FILE *log);
void * eskatu_ip(char * target, int port, FILE *log);
