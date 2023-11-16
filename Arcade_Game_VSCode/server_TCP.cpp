#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct GameState {
    bool game_over;
    bool pause;
    int lives_left;
    int enemies_killed;
    int active_enemies;
    bool victory;
    bool opener_screen;
    bool help;
};

void sendGameState(int client_socket, struct GameState game_state) {
    send(client_socket, &game_state, sizeof(struct GameState), 0);
}

void receiveGameState(int client_socket, struct GameState game_state) {
    recv(client_socket, &game_state, sizeof(struct GameState), 0);
}


int main() {

    // create game_state struct
    struct GameState game_state = {
        .game_over = false,
        .pause = false,
        .lives_left = 3,
        .enemies_killed = 0,
        .active_enemies = 30,
        .victory = false,
        .opener_screen = true,
        .help = false,
    };

    // create struct
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(2112)
    };
    // Convert IP address to binary, store in serv_addr
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // create socket 
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    // check if socket creation fails
    if (server_socket < 0) {
        perror("failed to create server socket");
        return -1;
    }

    // bind socket to specified address and port
    if (bind(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("failed to bind to socket");
        return -1;
    }

    // listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("failed to listen for connections");
        return -1;
    }

    printf("Server is listening on port 2112...\n");

    // accept incoming client connections
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // accept connection from client, and get client's socket
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

    if (client_socket < 0) {
        perror("failed to accept client connection");
        return -1;
    }

    // REPL

    // character array to store user input and server responses
    char buffer[1024] = {0};

    while(1) {
        // receive data from client
        int num_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (num_read <= 0) {
            break;
        }
        buffer[num_read] = '\0';
        printf("Received: %s\n", buffer);

        // send response back to client
        printf("Enter a response: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
    close(server_socket);
    return 0;
}