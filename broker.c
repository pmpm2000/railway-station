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
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define PORT "8696"
#define MAX_IP_LEN 20
#define BUFFER 1024
#define MAX_CONTENTS 20
#define MAX_CONTENT_ID_LEN 20
#define QUEUE_SIZE 10
#define SLEEP_TIME 10000000
#define MAX_MESSAGE_LEN 32

#define clients_limit 20

short saved;
short readd;

char dict[MAX_CONTENTS][MAX_CONTENT_ID_LEN];
short subscribers[MAX_CONTENTS][clients_limit+1]; // NIE MA KLIENTA O NUMERZE 0! JEST ZAREZERWOWANY DLA BRAKU KLIENTA
char message_queue[QUEUE_SIZE][BUFFER];
int received_messages = 0, received_publish = 0, received_subscribe = 0, received_unsubscribe = 0, received_bits = 0;


struct connection_info {
	int n; // liczba klientow znanych
	struct sockaddr_in* client; // info o kliencie polaczonym najpozniej
	int client_len; // dlugosc klienta
	int server_socket; // socket serwera
	struct sockaddr_in clients_list[clients_limit]; // lista wszystkich klientow, pusta komorka jesli wolne miejsce; wskaznik na tablice clients[] przy tworzeniu serwera
};

void* sending(void* param)
{
	bool if_checked = false;
	struct connection_info* info = (struct connection_info*)param;
	struct sockaddr_in* client_addr = info->client;
	
	for(int i=0; i < QUEUE_SIZE;i++)
	{
		for(int x = 0; x < BUFFER; x++)
		{
			message_queue[i][x] = '\0';
		}
		
	}
	
	for(int i=0;i<MAX_CONTENTS;i++)
	{
		for(int j = 0;j<MAX_CONTENT_ID_LEN;j++)
		{
			dict[i][j] = '0';
		}
	}
	
		
	for (;;)
	{
		
		// tu bedzie branie najnowszej wiadomosci z kolejki i wysylanie jej do subskrybentow
		// sprawdzanie czy nie przekroczono liczby klientow
		if (info->n + 1 > clients_limit)
		{
			printf("S: Przekroczono liczbe klientow! Dodawanie nie powiodlo sie. \n");
			continue;
		}
		// przepisywanie danych nowego klienta do clients_list[], zwiekszenie liczby n

		bool if_new = true;
		short client_id = -1;
		for (int i = 0; i < info->n; i++)
		{
			if(info->client->sin_addr.s_addr == info->clients_list[i].sin_addr.s_addr && info->client->sin_port == info->clients_list[i].sin_port && info->client->sin_family == info->clients_list[i].sin_family)
			{
				if_new = false; // jesli ostatnio polaczony klient jest taki sam jak klient na liscie to nie jest nowy
				client_id = i + 1;
				break;
			}

		}
		if (if_new) // jesli jest nowy to dopisujemy go
		{
			memcpy(&(info->clients_list[info->n]), (struct sockaddr_in*)info->client, info->client_len);
			client_id = (info->n) + 1;
			info->n++;
		}
		char received_data[BUFFER];
		// wczytywanie wiadomości z kolejki 
		if(readd == saved && if_checked == false && message_queue[saved][0] != '\0')
		{
			for(int i=0; i< BUFFER; i++)
			{
				received_data[i] = message_queue[readd][i];
			}
			readd = (readd + 1) % QUEUE_SIZE;
			printf("S: Wiadomosc sciagnieta z kolejki z pozycji %d, bo wskazniki takie same: %s \n", saved, received_data);
			if_checked = true;
		}
		else if(readd != saved)
		{
			for(int i=0; i< BUFFER; i++)
			{
				received_data[i] = message_queue[readd][i];
			}
			printf("S: Wiadomosc sciagnieta z kolejki z zpozycji %d, bo wskazniki rozne: %s \n", readd, received_data);
			readd = (readd + 1) % QUEUE_SIZE;
			if (readd == saved) if_checked = true;
			else if_checked = false;
			
		}
		
		received_bits += sizeof(received_data) * 8;

		if (received_data[0] == 's')
		{
			printf("S: Dostalem wiadomosc s %s \n", received_data);
			// kod dla subscribe
			received_subscribe++;
			char* token = strtok(received_data, ","); // podzial wiadomosci na podstawie spacji
			printf("S: Otrzymalem %s\n", token);
			token = strtok(NULL, ","); // teraz token to liczba podana jako drugi wyraz
			if (token == NULL)
			{
				printf("S: Bledna konstrukcja wiadomosci. Pomijanie... \n");
				continue;
			}
			int arguments = atoi(token); // convert stringa na inta
			printf("S: Liczba argumentow: %d\n", arguments);
			
			for (int i = 0; i < arguments; i++) // dla kazdego argumentu
			{
				bool active_subscription = false;
				token = strtok(NULL, ",");
				if (token == NULL)
				{
					printf("S: Bledna konstrukcja wiadomosci. Pomijanie... \n");
					break;
				}
				short content_id = -1;
				for (int j = 0; j < MAX_CONTENTS; j++) // przeszukuje slownik w poszukiwaniu ID danego contentu
				{
					if (strcmp(token, dict[j]) == 0)
					{
						content_id = j;
						printf("S: Znalazlem content %s\n", token);
						break;
					}
				}
				if (content_id == -1) // jesli nie znaleziono ID contentu to dodaje nowy wpis do slownika
				{
					for (int j = 0; j < MAX_CONTENTS; j++)
					{
						if (dict[j][0] == '0')
						{
							strcpy(dict[j], token);
							printf("S: Dopisalem content %s\n", token);
							content_id = j;
							break;
						}
					}
				}
				if (content_id == -1)
				{
					printf("S: Slownik jest pelny. Nie dopisano nowego contentu. \n");
					continue; // continue bo to oznacza ze slownik jest pelny
				}
				for (int j = 0; j < clients_limit; j++) // na liscie subskrybentow danego contentu szukamy subskrybenta
				{
					if(subscribers[content_id][j] == client_id)
					{
						printf("Subskrypcja już aktywna. \n");
						active_subscription = true;
					}
				}

				for (int j = 0; j < clients_limit; j++) // na liscie subskrybentow danego contentu szukamy wolnego miejsca
				{		
					if (subscribers[content_id][j] == 0 && active_subscription == false) // dopisujemy nowego subskrybenta
					{
						subscribers[content_id][j] = client_id;
						printf("S: Subskrypcja zarejestrowana. \n");
						break;
					}
				}
			}
		}
		else if (received_data[0] == 'u')
		{
			// kod dla unsubscribe
			received_unsubscribe++;
			char* token = strtok(received_data, ","); // podzial wiadomosci na podstawie spacji
			printf("S: Odebralem %s, ide dalej \n", token);
			token = strtok(NULL, ","); // teraz token to liczba podana jako drugi wyraz
			if (token == NULL)
			{
				printf("S: Bledna konstrukcja wiadomosci. Pomijanie... \n");
				continue;
			}
			int arguments = atoi(token); // convert stringa na inta
			printf("S: Liczba argumentow: %d \n", arguments);
			for (int i = 0; i < arguments; i++) // dla kazdego argumentu
			{
				token = strtok(NULL, ",");
				if (token == NULL)
				{
					printf("S: Bledna konstrukcja wiadomosci. Pomijanie... \n");
					break;
				}
				short content_id = -1;
				for (int j = 0; j < MAX_CONTENTS; j++) // przeszukuje slownik w poszukiwaniu ID danego contentu
				{
					if (strcmp(token, dict[j]) == 0)
					{   printf("S: Znalazlem content %s \n", token);
						content_id = j;
						break;
					}
				}
				if (content_id == -1)
				{
					printf("S: Brak odpowiedniego contentu w slowniku. Pomijam zadanie unsubscribe. \n");
					continue; // continue bo to oznacza ze slownik jest pelny
				}
				for (int j = 0; j < MAX_CONTENTS; j++) // na liscie subskrybentow danego contentu szukamy naszego klienta
				{
					if (subscribers[content_id][j] == client_id) // zamieniamy subskrybenta na puste pole
					{
						subscribers[content_id][j] = 0;
						printf("S: Subskrypcja anulowana. \n");
						break;
					}
				}
			}
		}
		else if (received_data[0] == 'p')
		{
			// kod dla publish
			// pamietac ze id klienta w tablicy clients_list[] jest o 1 mniejsze niz id klienta w tablicy subscribers[] (czyli client_id)
			received_publish++;
			char* token = strtok(received_data, ","); // podzial wiadomosci na podstawie przecinkow
			printf("S: Otrzymana wiadomosc %s \n", token);
			token = strtok(NULL, ","); // teraz token to nazwa contentu
			if (token == NULL)
			{
				printf("S: Bledna konstrukcja wiadomosci. Pomijanie... \n");
				continue;
			}
			printf("S: Szukam contentu %s \n", token);
			char content_name[MAX_CONTENT_ID_LEN];
			strcpy(content_name, token);
			token = strtok(NULL, ","); // teraz token to wiadomosc do przekazania
			if (token == NULL)
			{
				printf("S: Bledna konstrukcja wiadomosci. Pomijanie... \n");
				continue;
			}
			char str[MAX_MESSAGE_LEN];
			strcpy(str,"w,");
			strcat(str,token);
			short content_id = -1;
			for (int i = 0; i < MAX_CONTENTS; i++) // szukamy w slowniku ID danego contentu
			{
				if (strcmp(dict[i], content_name) == 0)
				{
					printf("S: Znalazlem content %s \n", content_name);
					content_id = i;
					break;
				}
			}
			if (content_id == -1) // jesli nie znaleziono w slowniku to trzeba dopisac nowy content
			{
				for (int i = 0; i < MAX_CONTENTS; i++)
				{
					if (dict[i][0] == '0')
					{
						strcpy(dict[i], content_name);
						printf("S: Dodałem do słownika %s\n", content_name);
						content_id = i;
						break;
					}
				}
			}
			if (content_id == -1)
			{
				printf("S: Slownik jest pelny. Nie dopisano nowego contentu. \n");
				continue; // continue bo to oznacza ze slownik jest pelny
			}
			for (int i = 0; i < clients_limit; i++)
			{
				short subscriber_id = subscribers[content_id][i]; // temu subskrybentowi trzeba przeslac wiadomosc
				if (subscriber_id == 0) continue; // bo to puste pole
				printf("S: Przesylam wiadomosc do klienta numer %d\n", subscriber_id);

				// przesyl danych
				struct sockaddr_in* caught_client = (struct sockaddr_in*)&info->clients_list[subscriber_id - 1];
				int local_server = info->server_socket;
				size_t msg = sendto(local_server, str, strlen(str), 0, caught_client, info->client_len);
				if (msg < 0)
				{
					printf("S: Blad w sendto() w publish \n");
					continue;
				}
				char ip4[MAX_IP_LEN];
				strcpy(ip4,inet_ntoa(caught_client->sin_addr));
				printf("S: Wiadomosc przekazana klientowi %s\n", ip4);
			}
		}
		
		for(int i=0;i<BUFFER;i++)
		{
			received_data[i] = '\0';
				 
		}
			
	}
}

void* diagnostics(void* param)
{
	for (;;)
	{
		// tu beda wypisywane dane diagnostyczne co jakis czas
		usleep(SLEEP_TIME);
		printf("================== \n");
		printf("DANE DIAGNOSTYCZNE \n");
		printf("================== \n");
		// aktywni subskrybenci kazdego contentu
		for (int i = 0; i < MAX_CONTENTS; i++)
		{
			short counter = 0;
			for (int j = 0; j < clients_limit; j++)
			{
				if (subscribers[i][j] != 0) counter++;
			}
			if(dict[i][0]!='0')
			{
			printf("Jest %d subscrybentow contentu %s \n", counter, dict[i]);
			}
		}
		
		// liczba otrzymanych dotychczas wiadomosci
		printf("Otrzymane dotychczas wiadomosci: %d \n", received_messages);
		printf("Otrzymane wiadomosci publish: %d \n", received_publish);
		printf("Otrzymane wiadomosci subscribe: %d \n", received_subscribe);
		printf("Otrzymane wiadomosci unsubscribe: %d \n", received_unsubscribe);
		printf("================== \n");
	}
}

void* communication(void* param)
{
	struct connection_info* info = (struct connection_info*)param;
	char received_data[BUFFER];
	for (;;)
	{	printf("C: polaczenie \n");
		ssize_t rec = recvfrom(info->server_socket, received_data, BUFFER, 0, (struct sockaddr*)info->client, &info->client_len);
		if (rec < 0)
		{
			printf("Blad w recvfrom w *communication \n");
			continue;
		}
		char ip4[MAX_IP_LEN];
		strcpy(ip4,inet_ntoa(info->client->sin_addr));
		printf("C: Odebralem pakiet %s od adresu %s\n", received_data, ip4);
		received_messages++;
		received_data[rec] = '\0';
		
		// dodawanie do kolejki
		strcpy(message_queue[saved], received_data);
		printf("C: Wpisuje wiadomosc do kolejki na miejsce %d\n", saved);
		saved = (saved + 1) % QUEUE_SIZE;
	}
}

int server() {
	struct sockaddr_in client;
	struct addrinfo h, * r;
	int n = 0;
	int client_len = sizeof(client);
	struct sockaddr_in clients[clients_limit];

	memset(&h, 0, sizeof(struct addrinfo));
	h.ai_family = PF_INET;
	h.ai_socktype = SOCK_DGRAM;
	h.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, PORT, &h, &r) != 0)
	{
		printf("Blad przy getaddrinfo \n");
		exit(1);
	}

	int server_socket = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
	if (server_socket == -1)
	{
		printf("Blad w socket() \n");
		exit(1);
	}

	if (bind(server_socket, r->ai_addr, r->ai_addrlen) != 0)
	{
		printf("Blad w bind() \n");
		exit(1);
	}
	struct connection_info connections =
	{
		n, (struct sockaddr_in*)&client, client_len, server_socket, *clients
	};

	pthread_t communication_thread;
	pthread_t sending_thread;
	pthread_t diagnostics_thread;

	pthread_create(&communication_thread, NULL, communication, (void*)&connections);
	pthread_create(&sending_thread, NULL, sending, (void*)&connections);
	pthread_create(&diagnostics_thread, NULL, diagnostics, (void*)&connections);

	pthread_join(communication_thread, NULL);
	pthread_join(sending_thread, NULL);
	pthread_join(diagnostics_thread, NULL);

	pthread_exit(NULL);
}


int main() {
	
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	printf("Uruchamianie programu... \n");
	server();
	return 0;
}
