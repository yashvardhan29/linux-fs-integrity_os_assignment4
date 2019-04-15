#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

struct merkleNode{
	char hash[20];
	struct merkleNode *leftChild;
	struct merkleNode *rightChild;
};

static struct merkleNode* root[100]; //assuming fd lies in [0,99]
static char* fnames[100];
static int filesys_inited = 0;

void get_sha1_hash (const void *buf, int len, const void *sha1)
{
	SHA1 ((unsigned char*)buf, len, (unsigned char*)sha1);
}

struct merkleNode* createMerkleTree(int fd){
	char blk[64];
	memset (blk, 0, 64);
	
	//Earlier we had 1024*1024 = 4 MB, but that can lead to 
	// a segfault, because there might not be enough space
	// on the stack. Hence, we will reduce the pointers, 
	// as in our case, we generally dont need files above
	// the size of 128 KB
	struct merkleNode* level[3000];
	int levelCount = 0;

	// Creating all the leaf nodes
	while(read (fd, blk, 64) > 0){
		assert(levelCount<1024*1024);

		level[levelCount] = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
		get_sha1_hash(blk, 64, level[levelCount++]->hash);

		memset (blk, 0, 64);
	}
	for(int i=0; i<levelCount; i++)
		printf("%d: %s\n", i, level[i]->hash);

	while(levelCount>1){
		int pCount;
		char blk[40];

		for(pCount = 0; pCount < levelCount/2; pCount++){
			for(int i=0; i<20; i++) blk[i] = level[pCount*2]->hash[i];
			for(int i=0; i<20; i++) blk[20+i] = level[pCount*2+1]->hash[i];

			struct merkleNode *node = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
			get_sha1_hash(blk, 64, node->hash);
			node->leftChild = level[pCount*2];
			node->rightChild = level[pCount*2+1];
			
			level[pCount] = node;
		}

		// if there was a node left, pull it in the level up
		// TODO: Duplicate the last node and add a combined hash
		// this will ensure that we always have a complete BT
		if(levelCount%2){
			level[pCount] = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
			for(int i=0; i<20; i++) level[pCount]->hash[i] = level[pCount*2]->hash[i];
			level[pCount*2]->leftChild = NULL;
			level[pCount*2]->rightChild = NULL;

			pCount++;
		}
		levelCount = pCount;
		printf("levelCount at end of iteration: %d\n", levelCount);
	}
	return level[0];
}

void merkleTreeTraverse(int fd){
	struct merkleNode* rootNode = root[fd];
	printf("%x\n", rootNode->hash);
	for(int i=0; i<20; i++)
		printf("%d", rootNode->hash[i]);
	printf("\n");
}

int main(){
	int fd = open ("merkleTest.txt", O_RDONLY, 0);
	printf("Got FD: %d\n", fd);
    root[fd] = createMerkleTree(fd);
    merkleTreeTraverse(fd);

	return 0;
}