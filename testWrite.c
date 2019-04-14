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
	int fd = open ("fooin.txt", O_RDWR, S_IRUSR|S_IWUSR);
	int ret = lseek (fd, 5, SEEK_SET);
	printf("lseek returned: %d\n", ret);
	char val = 4;
	write(fd, &val, 1);
	return 0;
}