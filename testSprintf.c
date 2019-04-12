#include <stdio.h>

int main(){
	char buf[32];
	snprintf (buf, 5, "f\noo_%d.txt", 82);
	printf("%s", buf);


	char buffer[50]; 
    char* s = "geeksforgeeks"; 
  
    // Counting the character and storing  
    // in buffer using snprintf 
    int j = snprintf(buffer, 6, "%s\n", s); 
  
    // Print the string stored in buffer and 
    // character count 
    printf("string:\n%s\ncharacter count = %d\n", 
                                     buffer, j); 

    
	return 0;
}