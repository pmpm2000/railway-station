#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stddef.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_IP_LEN 16
#define PORT "8696"
#define MAX_MESSAGE_LEN 200

int main()
{
	char ip[MAX_IP_LEN];
	printf("Podaj adres IP subskrybenta: ");
	scanf("%s", ip);
	struct sockaddr_in addr;
	struct addrinfo h, *r;
	int port = 8696;

	int length;
	
	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family = PF_INET;
	h.ai_socktype = SOCK_DGRAM;
	h.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo(NULL, PORT, &h, &r) != 0)
	{
		printf("Blad przy getaddrinfo \n");
		exit(1);
	}

	int sck = socket(AF_INET, SOCK_DGRAM, 0);
	if (sck == -1)
	{
		printf("Blad w socket() \n");
		exit(1);
	}
	if(bind(sck, r->ai_addr, r->ai_addrlen) != 0)
	{
		printf("Blad w bind() \n");
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	
	for (;;)
	{
		char msg[MAX_MESSAGE_LEN];
		bzero(msg, MAX_MESSAGE_LEN);
		printf("Wpisz wiadomosc: ");
		scanf("%s", msg);
		length = strlen(msg);

		ssize_t send = sendto(sck, msg, length, 0, (struct sockaddr*)&addr, sizeof(addr));
		if (send < 0)
		{
			printf("Blad w sendto() w publish \n");
			printf("ERROR: %s (%s:%d\n", strerror(errno), __FILE__, __LINE__);
			continue;
		}
		printf("Wiadomosc przekazana.\n");

	}
	return 0;
}