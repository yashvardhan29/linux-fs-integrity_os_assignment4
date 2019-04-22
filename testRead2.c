#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void readFile(char* fName){
	int fd = open (fName, O_RDONLY, 0);
	char* buf = (char *)malloc(500);
	int x = read(fd, buf, 500);
	printf("Number of Bytes Read: %d\n", x);
	
	assert(lseek(fd,0,SEEK_SET)==0);
	int end = lseek(fd, 0, SEEK_END);
	printf("Number of Bytes in the File: %d\n", end);

	close(fd);
}

int main(){
	int fd1 = open ("foo_1.txt", O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	char buf[30];
	memset (buf, 1, 30);
	write (fd1, buf, 30);
	close(fd1);

	readFile("foo_1.txt");
	return 0;
}