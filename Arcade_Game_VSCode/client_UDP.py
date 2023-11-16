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

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_addr = ('127.0.0.1', 2112)

print("Client is ready")


while True:
    data, server_addr = client_socket.recvfrom(1024)
    received_data = json.loads(data.decode())

    received_data_packet = DataPacket(**received_data)
    print(f"Game over: {received_data_packet.game_over}")
    print(f"Lives left: {received_data_packet.lives_left}")
    print(f"Enemies killed: {received_data_packet.enemies_killed}")
    print(f"Active enemies: {received_data_packet.active_enemies}")
    print(f"Victory: {received_data_packet.victory}")

    data_packet_to_send = DataPacket()
    data_packet_to_send.client_id = 1
    data_packet_to_send.player_moved_right=True
    data_packet_to_send.player_moved_left=False
    data_packet_to_send.player_fired=True
    data_packet_to_send.enemy_killed=False
    data_packet_to_send.player_powered_up=False
    data_packet_to_send.player_lost_life=False

    client_socket.sendto(json.dumps(data_packet_to_send.__dict__).encode(), server_addr)


client_socket.close()