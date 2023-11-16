#include "main_header.h"

GameState local_game_state;
GameResources resources;

int main() {
    InitWindow(screen_width, screen_height, "Space Invaders");
    //InitAudioDevice();
    LoadResources(resources);
    //SetMusicVolume(resources.background_music, 0.25);
    //PlayMusicStream(background_music);
    InitLocalGameState(local_game_state);
    LightGameState received_server_packet;
    ClientInputs client_packet_to_send = {0};
    Timer send_timer = {0};
    StartTimer(&send_timer, 0.05f);

// CLIENT - SERVER SETUP
    struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_port = htons(2112)};
    int client_socket = createSocketConnection("127.0.0.1", false, serv_addr);
    socklen_t serv_addr_len = sizeof(serv_addr);
    
    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        int bytes_read = recvfrom(client_socket, &received_server_packet, sizeof(received_server_packet), 0, (struct sockaddr*)&serv_addr, &serv_addr_len);
        if (bytes_read > 0) {
            GenerateClientStateFromServerPacket(local_game_state, received_server_packet);
        }

// INPUT
        if (!local_game_state.game_over && !local_game_state.opener_screen) {
            client_packet_to_send.client_right_key_down = IsKeyDown(KEY_RIGHT) ? true : false;
            client_packet_to_send.client_left_key_down = IsKeyDown(KEY_LEFT) ? true : false;
            client_packet_to_send.client_space_key_down = IsKeyDown(KEY_SPACE) ? true : false;
        }

// RENDER
        RenderGame(local_game_state.player_projectiles, local_game_state.enemies, local_game_state.player_one, local_game_state.player_two, local_game_state.powerup, resources);

// NETWORK
        UpdateTimer(&send_timer);
        if (TimerDone(&send_timer)) {
            sendto(client_socket, &client_packet_to_send, sizeof(client_packet_to_send), 0, (struct sockaddr*)&serv_addr, serv_addr_len);
            StartTimer(&send_timer, 0.05f);
        }
    }
    UnloadResources(resources);
    CloseWindow();
    close(client_socket);
    return 0;
};

