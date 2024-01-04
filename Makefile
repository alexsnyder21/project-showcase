all: server client

server: server.c
	gcc -g server.c -o server

client: client.c
	gcc -g client.c -o client

.PHONY: clean
clean:
	rm -rf server client

.PHONY: submit
submit:
	~schwesin/bin/submit csc328 project