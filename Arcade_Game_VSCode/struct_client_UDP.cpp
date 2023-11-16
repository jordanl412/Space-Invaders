#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

typedef struct DataPacket {
    bool game_over;
    int lives_left;
    int enemies_killed;
    int active_enemies;
    bool victory;
    int client_id;
    bool right_key_down;
    bool left_key_down;
    bool player_fired;
    bool enemy_killed;
    bool player_powered_up;
    bool player_lost_life;
} DataPacket;


int main() {
    int client_socket;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    struct DataPacket data_packet;
   // struct ClientPacket client_packet;
    char buffer[sizeof(DataPacket)];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2112);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Failed to create socket");
        return -1;
    }

    printf("Client is ready...\n");

    // REPL

    while(1) {
        ssize_t num_received = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &server_len);
        if (num_received <= 0) {
            perror("Error receiving data");
            return -1;
        }
        if (num_received == sizeof(DataPacket)) {
            memcpy(&data_packet, buffer, sizeof(DataPacket));

            printf("Received Data Packet: game_over: %d, lives_left:%d, enemies_killed:%d, active_enemies:%d, victory:%d",
            data_packet.game_over, data_packet.lives_left, data_packet.enemies_killed, 
            data_packet.active_enemies, data_packet.victory);
        }

        data_packet.client_id = 1;
        data_packet.right_key_down = true;
        data_packet.left_key_down = false;
        data_packet.player_fired = false;
        data_packet.enemy_killed = true;
        data_packet.player_powered_up = false;
        data_packet.player_lost_life = false;
        data_packet.game_over = false;
        data_packet.lives_left = 3;
        data_packet.enemies_killed = 3;
        data_packet.active_enemies = 27;
        data_packet.victory = false;

        memcpy(buffer, &data_packet, sizeof(DataPacket));
        sendto(client_socket, buffer, sizeof(DataPacket), 0, (struct sockaddr*)&server_addr, server_len);

/*
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // send user's input to server
        sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        // receive data from server, stone in buffer
        socklen_t serv_addr_len = sizeof(serv_addr);
        int num_read = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv_addr, &serv_addr_len);
        if (num_read > 0) {
            buffer[num_read] = '\0';
            printf("< %s \n", buffer);
        }
*/    
    }

    close(client_socket);
    return 0;
}   