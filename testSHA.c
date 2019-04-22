#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "filesys.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void printHash(char* has){
	for(int i=0; i<20; i++)
		printf("%d", has[i]);
	printf("\n");
}

/* returns 20 bytes unique hash of the buffer (buf) of length (len)
 * in input array sha1.
 */
void get_sha1_hash (const void *buf, int len, const void *sha1)
{
	SHA1 ((unsigned char*)buf, len, (unsigned char*)sha1);
}

char* sha1File(char* fName){
	char* ret = (char *) malloc( 20 );

	int fd = open (fName, O_RDONLY, 0);

	//Seek to the end of the file, in order to get File size
	int end = lseek(fd, 0, SEEK_END);
	//Create a buffer of file size length
	char* buf = (char *)malloc(end);

	//go to the beginning of the file
	assert(lseek(fd,0,SEEK_SET)==0);
	//read the file in buf
	int x = read(fd, buf, end);
	printf("x: %d\n", x);

	get_sha1_hash(buf,end,ret);
	printf("Generating SHA1 of file: %s with size: %d\n", fName,end);

	close(fd);

	return ret;
}

int main(){
	char* t1 = sha1File("foo_1.txt");
	char* t2 = sha1File("foo_1.txt");
	printHash(t1);
	printHash(t2);
	return 0;
}