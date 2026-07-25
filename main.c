#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#define MAX_THREADS 500

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int input_controll(int num_arg, char *args[], int *start, int *end);
void *scanner(void *arg);

int main(int argc, char *argv[]) { 
	
	int start, end;

	if (input_controll(argc, argv, &start, &end) == 1) {
		return 1;
	}

	int current_port = start;

	while (current_port <= end) {
		int ports_remaning = end - current_port + 1;

		int threads_this_round = (ports_remaning < MAX_THREADS) ? ports_remaning : MAX_THREADS;

		pthread_t threads[MAX_THREADS];

		int thread_args[MAX_THREADS];

		for (int i = 0; i < threads_this_round; i++) {
			thread_args[i] = current_port;
			pthread_create(&threads[i], NULL, scanner, &thread_args[i]);
			current_port++;
		}

		for (int j = 0; j < threads_this_round; j++) {
			pthread_join(threads[j], NULL);
		}
	}

	pthread_mutex_destroy(&lock);

	return 0;
}

int input_controll(int num_arg, char *args[], int *start, int *end) {
	if (num_arg != 3) {
		printf("Usage: ./scanner <start_port> <end_port>\n");
		return 1;
	}

	char *endptr;
	errno = 0;

	long start_long = strtol(args[1], &endptr, 10);

	if (errno == ERANGE || *endptr != '\0' || start_long < 0 || start_long > 65535) {
		printf("Invalid start port\n");
		return 1;
	}

	errno = 0;

	long end_long = strtol(args[2], &endptr, 10);

	if (errno == ERANGE || *endptr != '\0' || end_long < 0 || end_long > 65535) {
		printf("Invalid end port\n");
		return 1;
	}

	*start = start_long; 
	*end = end_long; 

	if (*start > *end) {
		int temp = *start;
		*start = *end;
		*end = temp;
	}

	return 0;
}

void *scanner(void *arg) {
	int port = *(int *)arg;
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == -1) {
		printf("Could not create socket\n");
		return NULL;
	}

	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl()\n");
		return NULL;
	}

	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl()\n");
		return NULL;
	}

	struct sockaddr_in target;
	target.sin_family = AF_INET;
	
	target.sin_port = htons(port);
	inet_pton(AF_INET, "45.33.32.156", &target.sin_addr);

	fd_set writefds; 
	FD_ZERO(&writefds);
	FD_SET(sock, &writefds);

	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	connect(sock, (struct sockaddr *)&target, sizeof(target));
	int retval = select(sock + 1, NULL, &writefds, NULL, &timeout);

	if (retval > 0) {
		int so_error;
		socklen_t so_len = sizeof(so_error);
		getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &so_len);
		if (so_error != 0 ) {
			printf("Port %d is closed: %s\n", port, strerror(so_error));
		} else {
			pthread_mutex_lock(&lock);
			printf("Port %d is open\n", port);
			pthread_mutex_unlock(&lock);
		}
	} 

	close(sock);

	return NULL;
	
}