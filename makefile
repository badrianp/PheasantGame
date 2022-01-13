all:
	gcc -pthread server.c -o server.o
	gcc client.c -o client.o
	@echo "project files compiled."
server:
	gcc -pthread server.c -o server.o
	@echo "server file compiled."
client:
	gcc client.c -o client.o
	@echo "client file compiled."
clean:
	rm server.o client.o
help:
	open README.md