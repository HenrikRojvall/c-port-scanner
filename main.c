#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(void) { 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket");
		return 1;
	} else {
		printf("Socket created with number: %d\n", sock);
	}

	struct sockaddr_in target;
	target.sin_family = AF_INET;

	target.sin_port = htons(80);
	inet_pton(AF_INET, "45.33.32.156", &target.sin_addr);

	int result = connect(sock, (struct sockaddr *)&target, sizeof(target));

	if (result == -1) {
		printf("Connection failed / Port is closed\n");
		return 1;
	} else {
		printf("Port is open\n");
	}

	close(sock);

	return 0;
}