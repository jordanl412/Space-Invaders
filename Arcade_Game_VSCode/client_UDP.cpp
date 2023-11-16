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
    // setup connection details
    // create struct for server's address
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(2112),
    };

    // convert IP address to binary, store in serv_addr
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // create socket for UDP communication
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

    // check if socket creation fails
    if (client_socket < 0) {
        perror("Failed to create socket");
        return -1;
    }

    // no constant connection to server in UDP

    // REPL

    // character array to store user input and server responses
    //char buffer[1024] = {0};

    TestStruct struct_to_receive;
    TestStruct struct_to_send = {
        .test_int = 4,
        .test_str = "Please work"
    };

    //while(1) {
        socklen_t serv_addr_len = sizeof(serv_addr);
        /*
        int bytes_read = recvfrom(client_socket, &struct_to_receive, sizeof(struct_to_receive), 0, (struct sockaddr*)&serv_addr, &serv_addr_len);
        if (bytes_read > 0) {
            printf("Received data: %d and %s\n", struct_to_receive.test_int, struct_to_receive.test_str);
        }
        if (bytes_read <= 0) {
            perror("Failed to receive data");
            return -1;
        }
        */

        //recvfrom(client_socket, &struct_to_receive, sizeof(struct_to_receive), 0, (struct sockaddr*)&serv_addr, &serv_addr_len);
        //printf("Received data: %d and %s\n", struct_to_receive.test_int, struct_to_receive.test_str);

        sendto(client_socket, &struct_to_send, sizeof(struct_to_send), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        /*while(1) {
            int bytes_read = recvfrom(client_socket, &struct_to_receive, sizeof(struct_to_receive), 0, (struct sockaddr*)&serv_addr, &serv_addr_len);
            if (bytes_read > 0) {
                printf("Received data: %d and %s\n", struct_to_receive.test_int, struct_to_receive.test_str);
                break;
            }
        }*/


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
        }*/
    //}

    close(client_socket);
    return 0;
}   