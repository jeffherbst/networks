compiled with : gcc -o server UDP_EchoServer.c
	          : gcc -o client UDP_EchoClient.c

run server with parameters: ./server [int drop chance] [ARQ protocol]
run client with parameters: ./client [servername] [filename] [ARQ protocol]
output file with be put in ./subdir/

Stop and wait works.
Go Back N has timeout issues and doesn't do properly work. Transfers a file but not in true Go back N style.
