import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_addr = ('127.0.0.1', 2112)
server_socket.bind(server_addr)

print(f"Server listening on port {server_addr}")

while True:
    data, client_addr = server_socket.recvfrom(1024)
    print(f"Received from {client_addr}: {data.decode()}")

    message = input("Enter message to send: ")
    server_socket.sendto(message.encode(), client_addr)

    server_socket.sendto(data, client_addr)

    