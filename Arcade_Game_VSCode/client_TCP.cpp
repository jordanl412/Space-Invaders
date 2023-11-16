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

    // Setup connection details
    // Create struct
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(2112)
    };
    // Convert IP address to binary, store in serv_addr
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Create socket and connect
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    // check if socket creation fails
    if (client_socket < 0) {
        perror("failed to create socket");
        return -1;
    }
    // connect socket to server in serv_addr, check for failure
    if (connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("failed to connect to server");
        return -1;
    }

    // REPL

    // character array to store user input and server responses
    char buffer[1024] = {0};
    while (1) {
        printf("> ");
        // read line of text from user input and store in buffer
        fgets(buffer, sizeof(buffer), stdin);
        // remove new line character from user input
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // send user's input to server through the socket
        send(client_socket, buffer, strlen(buffer), 0);

        // receive data from server, store it in buffer
        int num_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (num_read > 0) {
            buffer[num_read] = '\0';
            printf("< %s\n", buffer); 
        }
    }

    close(client_socket);
    return 0;

}