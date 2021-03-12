#include <sys/socket.h>
#include <linux/kernel.h>


// Levin header struct declaration
struct levhdr1003 {
        __be64          sign;           // Signature of msg (8 bytes)   /  0x0121010101010101 (beti)
        __le64          length;         // Length of msg (8 bytes)      /  0xf3=243
        unsigned char   exp_resp;       // Expected response (1 byte)   /  0x01 (eskaera)
        __le32          comm_cod;       // Command code (4 bytes)       /  0xe903=1001 (handshake), 0xef03=1007 (support flags)
        __le32          retn_cod;       // Return code (4 bytes)        /  0x00000000
        __le32          reserved;       // Reserved (4 bytes)           /  0x01000000
        __be32          endchars;       // Ending chars (4 bytes)       /  0x01000000 (beti)
} __attribute__((packed)); //33 byte-ak enpaketatuta

// Levin payload struct
struct data1003 {
	unsigned char d[10];
} __attribute__((packed));


// Methods
void * konprobatu_ping(void *args);

