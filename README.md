# File System Integrity using Merkle Tree

1. Would need to install libraries for ssh. Run `sudo apt-get install libssl-dev
`
2. This design would not work with crash, if you are able to make the tests pass. Then think about it.
3. Assuming only one file will be open at a time => We have capacity of having only one in-memory merkle tree
4. <https://www.ibm.com/developerworks/community/blogs/58e72888-6340-46ac-b488-d31aa4058e9c/entry/understanding_linux_open_system_call?lang=en> gives a better explanation of open syscall, than it's man page

## To make this work on MacOS
1. `brew install openssl`
2. `cd /usr/local/include `
3. `ln -s ../opt/openssl/include/openssl .`
4. `export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/opt/openssl/lib/`
Refer: [1](https://github.com/mongodb/mongo-php-driver/issues/523), [2](https://github.com/brianmario/mysql2/issues/795#issuecomment-337006164)

## Run
> `make && make run`