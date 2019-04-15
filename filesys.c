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

struct merkleNode{
	char hash[20];
	struct merkleNode *leftChild;
	struct merkleNode *rightChild;
};

static struct merkleNode* root[100]; //assuming fd lies in [0,99]
static char* fnames[100];
static int filesys_inited = 0;

/* returns 20 bytes unique hash of the buffer (buf) of length (len)
 * in input array sha1.
 */
void get_sha1_hash (const void *buf, int len, const void *sha1)
{
	SHA1 ((unsigned char*)buf, len, (unsigned char*)sha1);
}

struct merkleNode* createMerkleTree(int fd){
	char blk[64];
	memset (blk, 0, 64);

	// printf("tryint to open: %s\n", fnames[fd]);
	fd = open (fnames[fd], O_RDONLY, 0);
	//Handling empty file case, and case when read is returning -1
	int numCan = read (fd, blk, 64);
	// printf("Able to read %d bytes (max 64)\n", numCan);
	assert (numCan >=0 );
	if(numCan == 0){
		struct merkleNode* ret = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
		memset (ret->hash, 0, 64);
		ret -> leftChild = NULL;
		ret -> rightChild = NULL;
		return ret;
	}
	lseek(fd, 0, SEEK_SET);
	
	
	struct merkleNode* level[3000];
	int levelCount = 0;

	//Creating all the leaf nodes
	while(read (fd, blk, 64) > 0){
		assert(levelCount<3000);

		level[levelCount] = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
		level[levelCount] -> leftChild = NULL;
		level[levelCount] -> rightChild = NULL;
		get_sha1_hash(blk, 64, level[levelCount++]->hash);

		memset (blk, 0, 64);
	}
	close(fd);

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
	}
	return level[0];
}

int hashSame(char* h1, char* h2){
	for(int i=0; i<20; i++)
		if( h1[i]!=h2[i] )
			return 0;
	return 1;
}

void merkleTreeTraverse(int fd){
	struct merkleNode* rootNode = root[fd];
	// printf("%x\n", rootNode->hash);
	for(int i=0; i<20; i++)
		printf("%d", rootNode->hash[i]);
	printf("\n");
}

void printHash(char* has){
	for(int i=0; i<20; i++)
		printf("%d", has[i]);
	printf("\n");
}

int updateSecure(int fd){
	char* fName = fnames[fd];
	char* updatedHash = root[fd]->hash;
	int secureFD = open("secure.txt", O_RDWR, S_IRUSR|S_IWUSR);
	int n;
	char secureBlock[52]; // 32 for fileName + 20 for Root Hash

	while((n = read(secureFD, secureBlock, sizeof(secureBlock))) > 0){
		assert(n == 52);
		
		char filename[32];
		char hash[20];
		for(int i = 0; i < 32; i++) filename[i] = secureBlock[i];
		for(int j = 0; j < 20; j++) hash[j] = secureBlock[j+32];

		if( !strcmp(filename,fName) ){
			lseek(secureFD, -20, SEEK_CUR);

			//Update the root hash in secure.txt
			write(secureFD, updatedHash, 20);
			return 1;
		}
	}
	return 0;
}

char* checkSecure(int fd){
	char* fName = fnames[fd];
	int secureFD = open("secure.txt", O_RDWR, S_IRUSR|S_IWUSR);
	int n;
	char secureBlock[52]; // 32 for fileName + 20 for Root Hash
	char *hash = (char*)malloc(20);

	while((n = read(secureFD, secureBlock, sizeof(secureBlock))) > 0){
		assert(n == 52);
		
		char filename[32];
		
		for(int i = 0; i < 32; i++) filename[i] = secureBlock[i];
		for(int j = 0; j < 20; j++) hash[j] = secureBlock[j+32];

		if( !strcmp(filename,fName) )
			return hash;
	}
	return NULL;
}

/* Build an in-memory Merkle tree for the file.
 * Compare the integrity of file with respect to
 * root hash stored in secure.txt. If the file
 * doesn't exist, create an entry in secure.txt.
 * If an existing file is going to be truncated
 * update the hash in secure.txt.
 * returns -1 on failing the integrity check.
 */
int s_open (const char *pathname, int flags, mode_t mode)
{
	printf("s_open(%s, %d)\n", pathname, flags);
	assert (filesys_inited);
	int n;
	int fd1; //file descriptor for file
	int fd2; //file descriptor for secure.txt
	char buffer[52]; // 32 for fileName + 20 for Root Hash
	int existsInSecure = 0;

	/*
	If the file exists and secure.txt is there; then check for consistency
	If tampered, then return -1 

	otrunk -> truncate the file, make file size 0 (something like that)
	if not tampered..., then you are going to make the merkle
	if file size is 0, then consider the hash to be 0.

	if truncate flag is not given, then open call will not change the file
	so would not need to do anything with merkle, in that case
	*/
	int existsInFS = access( pathname, F_OK ) != -1;
	fd1 = open(pathname, flags, mode);
	fd2 = open("secure.txt", O_RDWR, S_IRUSR|S_IWUSR);

	fnames[fd1] = (char *)malloc(32);//IS THIS THE RIGHT PLAce
	int i;
	for(i=0; pathname[i]!='\0'; i++) fnames[fd1][i] = pathname[i];
	fnames[fd1][i] = '\0';

	//Step 1: Build in-memory merkle tree
	if( existsInFS ) {
		printf("existsInFS\n");
		root[fd1] = createMerkleTree(fd1);
		// merkleTreeTraverse(fd1);

		while((n = read(fd2, buffer, sizeof(buffer))) > 0){
			char filename[32];
			char hash[20];

			for(int i = 0; i < 32; i++) 
				filename[i]=buffer[i];
			for(int j = 0; j < 20; j++) 
				hash[j]=buffer[j+32];

			if( !strcmp(filename,pathname) ){
				existsInSecure = 1;
				if(!(flags & O_TRUNC)){

					//Step 2: Compare Root Hash (if fail return -1)
					if(!hashSame(hash, root[fd1]->hash)){
						// printf("%s\n", root[fd1]->hash);
						printHash(root[fd1]->hash);
						// printf("%s\n", hash);
						printHash(hash);
						printf("hash ki dikkat hai\n");
						return -1;
					}
					return fd1;
				} else{
					lseek(fd2, -20, SEEK_CUR);

					//Update the root hash in secure.txt
					write(fd2, root[fd1]->hash, 20);

					return fd1;
				}
			}
		}

		//If the file exists in FS, secure.txt must have an entry for it
		assert(existsInSecure);

	} else {
		printf("DNE InFS\n");
		root[fd1] = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
		memset(root[fd1]->hash, 0, 64);
		root[fd1] -> leftChild = NULL;
		root[fd1] -> rightChild = NULL;

		//Step 3: if file DNE, create entry in secure.txt
		char fn[32];
		memset(fn, 0, 32);
		int i;
		for(i=0; pathname[i]!='\0'; i++) fn[i] = pathname[i];
		assert(i<31);
		assert(pathname[i]=='\0');
		fn[i]=pathname[i];

		lseek(fd2, 0, SEEK_END);
		write(fd2, fn, 32);
		write(fd2, root[fd1]->hash, 20);
		return fd1;
	}
	assert(1==0);
	return -1;//SHOULD NOT REACH HERE
}

/* SEEK_END should always return the file size 
 * updated through the secure file system APIs.

 * from unistd.h; whence values for lseek(2)
	#define	SEEK_SET	0	set file offset to offset
	#define	SEEK_CUR	1	set file offset to current plus offset
	#define	SEEK_END	2	set file offset to EOF plus offset
 */
int s_lseek (int fd, long offset, int whence)
{
	assert(1==0);
	assert(fd<100);
	assert (filesys_inited);
	int ret = lseek (fd, offset, whence);
	printf("ret: %d\n", ret);
	return ret;
}

/* read the blocks that needs to be updated
 * check the integrity of the blocks
 * modify the blocks
 * update the in-memory Merkle tree and root in secure.txt
 * returns -1 on failing the integrity check.
 * Finally, write modified blocks of the file
 */
ssize_t s_write (int fd, const void *buf, size_t count)
{
	assert(fd<100);
	assert (filesys_inited);

	struct merkleNode* blah = createMerkleTree(fd);
	if(!hashSame(blah->hash, root[fd]->hash)){
		printHash(blah->hash);
		printHash(root[fd]->hash);
		// printHash(checkSecure(fd));
		return -1;
	}

	int ret = write (fd, buf, count);//CHECK OUTPUT MAYBE
	root[fd] = createMerkleTree(fd);
	// merkleTreeTraverse(fd);

	// CHANGE SECURE.TXT
	assert(updateSecure(fd)==1);

	return ret;
}

/* check the integrity of blocks containing the 
 * requested data.
 * returns -1 on failing the integrity check.
 */
ssize_t s_read (int fd, void *buf, size_t count)
{
	assert (filesys_inited);
	assert(fd<100);
	//Step 1: Compute the BLOCKS of the file that need to be read
	//Step 2: Read the blocks
	//Step 3: Check Integrity (if fail return -1)

	struct merkleNode* blah = createMerkleTree(fd);
	if(!hashSame(blah->hash, root[fd]->hash)) 
		return -1;

	return read (fd, buf, count);
}

void destroyTree(struct merkleNode* x){
	if(x == NULL)
		return;
	destroyTree(x->leftChild);
	destroyTree(x->rightChild);
	free(x);
}

/* destroy the in-memory Merkle tree */
int s_close (int fd)
{
	assert(fd<100);
	assert (filesys_inited);
	destroyTree(root[fd]);
	return close (fd);
}

/* Check the integrity of all files in secure.txt
 * remove the non-existent files from secure.txt
 * returns 1, if an existing file is tampered
 * return 0 on successful initialization
 */
int filesys_init (void)
{
	assert(filesys_inited == 0);

	//if secure.txt does not exist, CREATE
	int fd = open("secure.txt", O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	assert(fd != -1);

	// Check the integrity of all the files whos hashes exist in secure.txt - no more

	// if a file DNE, just throw away corresponding entry in secure.txt, (if entry exists)

	// if Integrity of an existing file is compromised, return 1

	filesys_inited = 1;
	return 0; //on success
}
