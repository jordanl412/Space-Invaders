import socket
import json

class DataPacket: 
    def __init__(self):
        self.game_over=False
        self.lives_left=3
        self.enemies_killed=0
        self.active_enemies=30
        self.victory=False
        self.client_id=0
        self.player_moved_right=False
        self.player_moved_left=False
        self.player_fired=False
        self.enemy_killed=False
        self.player_powered_up=False
        self.player_lost_life=False

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_addr = ('127.0.0.1', 2112)
server_socket.bind(server_addr)

print(f"Server listening on port {server_addr}")

while True:
    data, client_addr = server_socket.recvfrom(1024)
    received_data = json.loads(data.decode())

    received_data_packet = DataPacket(**received_data)
    print(f"Received from client {received_data_packet.client_id}")
    print(f"Player moved right: {received_data_packet.player_moved_right}")
    print(f"Player moved left: {received_data_packet.player_moved_left}")
    print(f"Player fired: {received_data_packet.player_fired}")
    print(f"Enemy killed: {received_data_packet.enemy_killed}")
    print(f"Player powered up: {received_data_packet.player_powered_up}")
    print(f"Player lost life: {received_data_packet.player_lost_life}")

    data_packet_to_send = DataPacket()
    data_packet_to_send.game_over = False
    data_packet_to_send.lives_left = 3
    data_packet_to_send.enemies_killed=2
    data_packet_to_send.active_enemies=28
    data_packet_to_send.victory=False

    server_socket.sendto(json.dumps(data_packet_to_send.__dict__).encode(), client_addr)