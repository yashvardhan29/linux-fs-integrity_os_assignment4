#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "filesys.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(){
	int fd = open ("fooin.txt", O_RDONLY, 0);
    char blk[64];
    memset (blk, 0, 64);
    
    while(read (fd, blk, 10)){
        printf("%s\n", blk);    
        memset (blk, 0, 64);
    }
    // for(int i=0; i<64; i++) printf("%d: %d\n", i, blk[i]);
	return 0;
}