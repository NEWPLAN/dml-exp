all:server client

server: epoll_server.c
	gcc $^ -o $@ -Os

client: epoll_client.c
	gcc  $^ -o $@ -Os

clean:
	rm -rf client server