import socket

server_socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind( ('127.0.0.1', 2112) )
server_socket.listen( 5 )

while True:
    conn, addr = server_socket.accept()
    print( f'New connection {conn} from address {addr}' )
    while True:
        data = conn.recv(1024).decode()
        if not data:
            print(f'No more data, dropping connection')
            break
        print(f'Data from connected user: {data}')
        data = input(' > ')
        conn.sendall(data.encode())

    conn.close()