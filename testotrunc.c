#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(){
	int x = O_WRONLY | O_CREAT | O_TRUNC;
	printf("%d\n", x&O_TRUNC);
}