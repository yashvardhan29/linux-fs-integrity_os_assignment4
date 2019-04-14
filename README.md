# File System Integrity using Merkle Tree

1. Would need to install libraries for ssh. Run `sudo apt-get install libssl-dev
`
2. This design would not work with crash, if you are able to make the tests pass. Then think about it.
3. Assuming only one file will be open at a time => We have capacity of having only one in-memory merkle tree
4. <https://www.ibm.com/developerworks/community/blogs/58e72888-6340-46ac-b488-d31aa4058e9c/entry/understanding_linux_open_system_call?lang=en> gives a better explanation of open syscall, than it's man page
5. Max File Name in our FS can be of 32 Bytes (including NULL Character). As is, assumed in base.c

## To make this work on MacOS
1. `brew install openssl`
2. `cd /usr/local/include `
3. `ln -s ../opt/openssl/include/openssl .`
4. `export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/opt/openssl/lib/`
Refer: [1](https://github.com/mongodb/mongo-php-driver/issues/523), [2](https://github.com/brianmario/mysql2/issues/795#issuecomment-337006164)

## Run
> `make && make run`

## Test Cases
1. `testcase1.c`: basically all it does, is pick a random file, change its contents (in corrupt_file function) at some random offset. Then tries to open all the files again, if all files open; it fails. else it passes
2. `testcase2.c`: Corrupt some file at a given offset, and then try to write in the file at that offset
3. `testcase3.c`: Opens `foo_0.txt` twice, first using `open` and then using `s_open`. After this, it does `ret = write (fd1, &val, 1);` at the end of the file, then it expects that `s_lseek (fd2, 0, SEEK_END);` should return 128000 instead of 128001(which it is currently returning). Our program gave 128001, which was updated due to a `write`. We should have updated it, only if it was a `s_write`, currently no clue how to solve this, as no where do we manually change the SEEK_SET/SEEK_END. (they are just some #defs for numbers. Actually it would be something related to the linux file system)
4. `testcase4.c`: corrupts one file and then tries to do a `filesys_init`, it should fail, as there exists a corrupt file. If it passes, then the test case fails.

## Assumptions
1. That the file size is 64 bit aligned. If not, we add '\0' to the rest of the block
2. At a time only one file is open (we store merkle tree of only one file): this might be problematic

## Potential Problems
1. The merkle tree might not be complete. So change the odd case handling
2. 

## References
1. [Using SHA in C](https://stackoverflow.com/a/919375/2806163)
2. [Allocating structures dynamically](https://www.quora.com/How-to-create-a-dynamic-structures-in-C)
3. <https://www.quora.com/Cryptography-How-does-a-Merkle-proof-actually-work/answer/Belavadi-Prahalad>
4. <https://en.wikipedia.org/wiki/Merkle_tree>
5. [snprintf](https://www.geeksforgeeks.org/snprintf-c-library/), [sprintf](https://www.geeksforgeeks.org/sprintf-in-c/)
6. [**File Positioning**](https://www.gnu.org/software/libc/manual/html_node/File-Positioning.html)
