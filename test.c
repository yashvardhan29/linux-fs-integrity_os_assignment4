#include <stdio.h>

int main(){
	int levelCount = 35;
	int pCount;	
	for(pCount = 0; pCount < levelCount/2; pCount++){
		printf("%d %d\n", pCount*2, pCount*2+1);
	}
	if(levelCount%2)
		printf("%d\n", pCount*2);
    
	return 0;
}