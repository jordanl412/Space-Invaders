#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

typedef struct TestStruct {
    int test_int;
    char test_str[30];
} TestStruct;

int main() {
    // Create struct for server's address
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(2112),
        .sin_addr.s_addr = INADDR_ANY
    };

    // create socket
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    int flags = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

    // check if socket creation fails
    if (server_socket < 0) {
        perror("Failed to create server socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Failed to set SO_REUSEADDR");
        return -1;
    }

    // non-blocking


    // bind socket to specied address and port
    if (bind(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Failed to bind to socket");
        return -1;
    }

    printf("Server is listening on port 2112...\n");

    // no constant connection, constant listening for incoming connections with UDP

    //REPL
    // store incoming/outgoing data in buffer
    //char buffer[1024]; 
    TestStruct struct_to_receive;
    TestStruct struct_to_send = {
        .test_int = 5,
        .test_str = "It did!"
    };

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while(1) {


        int bytes_read = recvfrom(server_socket, &struct_to_receive, sizeof(struct_to_receive), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytes_read > 0) {
            printf("Received data: %d and %s\n", struct_to_receive.test_int, struct_to_receive.test_str);
            struct_to_receive.test_int += 1;
            sendto(server_socket, &struct_to_receive, sizeof(struct_to_receive), 0, (struct sockaddr*)&client_addr, client_addr_len);
        }

        /*
        // receive data from a client
        int num_read = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (num_read <= 0) {
            perror("Error receiving data");
            break;
        }

        buffer[num_read] = '\0';
        printf("Received: %s\n", buffer);

        // send response back to client
        printf("Enter a response: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        */
    }

    close(server_socket);
    return 0;
}