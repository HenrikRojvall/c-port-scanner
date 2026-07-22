#include <stdio.h>
#include <sys/socket.h>

int main(void) { 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket");
		return 1;
	} else {
		printf("Socket created with number: %d\n", sock);
	}

	return 0;
}