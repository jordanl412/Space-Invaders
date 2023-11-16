import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_addr = ('127.0.0.1', 2112)

while True:
    message = input("Enter a message to send (or 'exit'): ")

    if message.lower() == 'exit':
        break

    client_socket.sendto(message.encode(), server_addr)

    data, _ = client_socket.recvfrom(1024)
    print(f"Received from server: {data.decode()}")

client_socket.close()