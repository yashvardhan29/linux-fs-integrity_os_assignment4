#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "filesys.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct merkleNode{
	char hash[20];
	struct merkleNode *leftChild;
	struct merkleNode *rightChild;
};

static struct merkleNode* root;
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
	
	struct merkleNode* level[1024*1024];//4 MB
	int levelCount = 0;

	//Creating all the leaf nodes
	while(read (fd, blk, 64)){
		assert(levelCount<1024*1024);

		level[levelCount++] = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
		get_sha1_hash(blk, 64, level[levelCount++]->hash);

		memset (blk, 0, 64);
	}

	while(levelCount!=1){
		int pCount;
		for(pCount = 0; pCount < levelCount/2; pCount++){
			
			char blk[40];
			for(int i=0; i<20; i++) blk[i] = level[pCount*2]->hash[i];
			for(int i=0; i<20; i++) blk[20+i] = level[pCount*2+1]->hash[i];

			struct merkleNode *node = (struct merkleNode*) malloc( sizeof(struct merkleNode) );
			get_sha1_hash(blk, 64, node->hash);
			node->leftChild = level[pCount*2];
			node->rightChild = level[pCount*2+1];
			
			level[pCount] = node;
		}

		// if there was a node left, pull it in the level up
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
	assert (filesys_inited);
	int fd; //file descriptor for file
	int fd1; //file descriptor for secure.txt
	char buffer[52];
	struct merkleNode *root;
	fd=open(pathname,flags,mode);

	/*
	If the file exists and secure.txt is there; then check for consistency
	If tampered, then return -1 

	otrunk -> truncate the file, make file size 0 (something like that)
	if not tampered..., then you are going to make the merkle
	if file size is 0, then consider the hash to be 0.

	if truncate flag is not given, then open call will not change the file
	so would not need to do anything with merkle, in that case
	*/

	//Step 1: Build in-memory merkle tree
	if( access( pathname, F_OK ) != -1 ) {
		root=createMerkleTree(fd);
		fd1=open("secure.txt",O_RDWR, S_IRUSR|S_IWUSR);
		while((n = read(fd1, buffer, sizeof(buffer))) > 0){
			char filename[32];
        	char hash[20];
        	int i,j;
        	for(i=0;i<32;i++){
        		filename[i]=buffer[i];
        	}
        	filename[i] = '\0';
        	for(j=0;j<20;j++){
        		hash[j]=buffer[j+32];
        	}
        	hash[20] = '\0';
        	if(strcmp(filename,pathname)==0){
        		
        	}
		}
    } 
    else {
    	// file doesn't exist
    }
	//Step 2: Compare Root Hash (if fail return -1)
	
	//Step 3: if file DNE, create entry in secure.txt
	
	//Step 4: Handle Truncating

	return open (pathname, flags, mode);
}

/* SEEK_END should always return the file size 
 * updated through the secure file system APIs.
 */
int s_lseek (int fd, long offset, int whence)
{
	assert (filesys_inited);
	return lseek (fd, offset, SEEK_SET);
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
	assert (filesys_inited);
	return write (fd, buf, count);
}

/* check the integrity of blocks containing the 
 * requested data.
 * returns -1 on failing the integrity check.
 */
ssize_t s_read (int fd, void *buf, size_t count)
{
	//Step 1: Compute the BLOCKS of the file that need to be read

	//Step 2: Read the blocks

	//Step 3: Check Integrity (if fail return -1)


	assert (filesys_inited);
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
	assert (filesys_inited);
	destroyTree(root);
	return close (fd);
}

/* Check the integrity of all files in secure.txt
 * remove the non-existent files from secure.txt
 * returns 1, if an existing file is tampered
 * return 0 on successful initialization
 */
int filesys_init (void)
{
	//if secure.txt does not exist, CREATE
	int fd = open("secure.txt", O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
	assert(fd != -1);

	// Check the integrity of all the files whos hashes exist in secure.txt

	// if a file DNE, just throw away corresponding entry in secure.txt, (if entry exists)

	// if Integrity of an existing file is compromised, return 1

	filesys_inited = 1;
	return 0; //on success
}
