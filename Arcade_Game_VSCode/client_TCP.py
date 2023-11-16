import socket

client_socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
client_socket.connect( ('127.0.0.1', 2112) )

while True:
    message = input('> ')
    if message == 'exit':
        break
    client_socket.sendall(message.encode())
    output = client_socket.recv(1024).decode()
    print( f'Received from server: {output}' )

client_socket.close()