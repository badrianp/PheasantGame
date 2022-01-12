all:
	gcc -pthread server.c -o server
	gcc client.c -o client
	@echo "project files compiled."
server:
	gcc -pthread server.c -o server
	@echo "server file compiled."
client:
	gcc client.c -o client
	@echo "client file compiled."
clean:
	rm server client
help:
	open README.md