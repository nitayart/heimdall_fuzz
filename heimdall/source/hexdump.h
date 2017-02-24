#include <ctype.h>

#define HEXDUMP_COLS (16)

void hexdump_log(void *mem, unsigned int len) {
    unsigned int i, j;
    char buffer[256] = {0};
    char str[80] = {0};

    if (!len || !mem) {
        printf("No data\n");
        return;
    }

    printf("Dumping %08x bytes at address %lx:\n", len, (unsigned long) mem);

    for (i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
        if(i % HEXDUMP_COLS == 0) {
            sprintf(str, "0x%lx (+0x%04x): ", (unsigned long) (i + mem), i);
            strcat(buffer, str);
        }

        if(i < len) {
            sprintf(str, "%02x ", 0xFF & ((char*)mem)[i]);
            strcat(buffer, str);
        } else {
            sprintf(str, "   ");
            strcat(buffer, str);
        }

        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if(j >= len) {
                    sprintf(str, " ");
                    strcat(buffer, str);

                } else if (isprint (((char*) mem)[j])) {
                    sprintf(str, "%c", 0xFF & ((char*)mem)[j]);
                    strcat(buffer,str);
                } else {
                    sprintf(str, ".");
                    strcat(buffer,str);
                }
            }

            printf("%s\n", buffer);
            memset(&buffer[0], 0, sizeof(buffer));
        }
    }
}
