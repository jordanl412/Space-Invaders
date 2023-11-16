#include "copy_of_main_header.h"

GameState local_game_state;
GameResources resources;

int main() {// organize by raylib, game logic, and network stuff
    InitWindow(screen_width, screen_height, "Space Invaders");
    InitAudioDevice();
    LoadResources(resources);
    SetMusicVolume(resources.background_music, 0.25);
    //PlayMusicStream(background_music);
    InitLocalGameState(local_game_state);
    ClientInputs received_client_packet;
    LightGameState server_packet_to_send;
    GenerateServerPacketFromLocalGameState(server_packet_to_send, local_game_state);

    Timer powerup_timer = {0};
    Timer projectile_shot_timer = {0};
    Timer send_timer = {0};
    StartTimer(&send_timer, 0.05f);
    Timer ai_timer = {0};
    Timer ai_cooldown_timer = {0};

    StartTimer(&ai_timer, 0.05f);
    float time_since_change_ai = 0.0f;

    AI_Blackboard ai_blackboard(local_game_state.player_two, local_game_state.player_one, HighLevelAIState::None, LowLevelAIState::CalculateTargetLocation);

// SERVER-CLIENT SETUP
    struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_port = htons(2112)};
    int server_socket = createSocketConnection(NULL, true, serv_addr);
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    SetTargetFPS(120);
    while (!WindowShouldClose())
    {
        int bytes_read = recvfrom(server_socket, &received_client_packet, sizeof(received_client_packet), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytes_read > 0) {
            local_game_state.client_right_key_down = received_client_packet.client_right_key_down;
            local_game_state.client_left_key_down = received_client_packet.client_left_key_down;
            local_game_state.client_space_key_down = received_client_packet.client_space_key_down;
        }
        UpdateTimer(&send_timer);
// INPUT
        if (local_game_state.opener_screen) {
            //PlayMusicStream(resources.opener_music);
            //UpdateMusicStream(resources.opener_music);
            if (IsKeyPressed(KEY_ENTER)) {
                //printf("Enter key pressed\n");
                //StopMusicStream(resources.opener_music);
                //PlayMusicStream(resources.background_music);
                //UpdateMusicStream(resources.background_music);
                InitLocalGameState(local_game_state);
                local_game_state.opener_screen = false;
            }
        }

        if (!local_game_state.game_over && !local_game_state.opener_screen) {
            //Stream(background_music);
            if (IsKeyDown(KEY_RIGHT)) { MovePlayerRight(local_game_state.player_one); }
            if (IsKeyDown(KEY_LEFT)) { MovePlayerLeft(local_game_state.player_one); }
            if (local_game_state.client_right_key_down) { MovePlayerRight(local_game_state.player_two); }
            if (local_game_state.client_left_key_down) { MovePlayerLeft(local_game_state.player_two); }
            if (IsKeyPressed(KEY_SPACE)) { ShootProjectile(local_game_state.player_projectiles, local_game_state.player_one, local_game_state.active_projectiles); }
            if (local_game_state.client_space_key_down) {
                if (TimerDone(&projectile_shot_timer)) { 
                    ShootProjectile(local_game_state.player_projectiles, local_game_state.player_two, local_game_state.active_projectiles);
                    StartTimer(&projectile_shot_timer, 0.3f);
                }
            }
            
// AI Stuff
            UpdateTimer(&ai_timer);
            UpdateTimer(&ai_cooldown_timer);
            if (IsKeyPressed(KEY_F)) {
                if (ai_blackboard.high_level_state != HighLevelAIState::ProtectMode) {
                    ai_blackboard.high_level_state = HighLevelAIState::ProtectMode;
                    ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
                } else {
                    ai_blackboard.high_level_state = HighLevelAIState::None;
                    ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
                }
            }
            if ((screen_width > GetMouseX()) && (GetMouseX() > screen_width/2) && (screen_height > GetMouseY() > 0)) {
            //if (IsKeyPressed(KEY_R)) {
                if (ai_blackboard.high_level_state != HighLevelAIState::FocusRight) {
                    ai_blackboard.high_level_state = HighLevelAIState::FocusRight;
                    ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
                } //else {
                //    ai_blackboard.high_level_state = HighLevelAIState::None;
                //    ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
                //}
            }
            if ((0 < GetMouseX()) && (GetMouseX() < screen_width/2) && (screen_height > GetMouseY() > 0)) {
            //if (IsKeyPressed(KEY_L)) {
                if (ai_blackboard.high_level_state != HighLevelAIState::FocusLeft) {
                    ai_blackboard.high_level_state = HighLevelAIState::FocusLeft;
                    ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
                } //else {
                   // ai_blackboard.high_level_state = HighLevelAIState::None;
                    //ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
                //}      
            }
            run_ai(ai_blackboard, screen_width, ai_timer, ai_cooldown_timer, local_game_state.enemies, local_game_state.player_projectiles);
            
        UpdateTimer(&projectile_shot_timer);
    
// GAME LOGIC
            for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) {
                if (local_game_state.player_projectiles[projectile].active) {
                    local_game_state.player_projectiles[projectile].rectangle.y -= local_game_state.player_projectiles[projectile].speed;
                    if (local_game_state.player_projectiles[projectile].rectangle.y <= (0)) {
                        local_game_state.player_projectiles[projectile].active = false;
                        local_game_state.active_projectiles -= 1;
                    }
                    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
                        if (CheckCollisionRecs(local_game_state.player_projectiles[projectile].rectangle, local_game_state.enemies[enemy].rectangle)) {
                            //PlaySound(resources.killed_enemy_sound);
                            local_game_state.player_projectiles[projectile].active = false;
                            local_game_state.active_projectiles -= 1;
                            InitEnemyEntities(local_game_state.enemies[enemy]);
                            local_game_state.enemies[enemy].active = false;
                            local_game_state.enemies_killed += 1;
                            local_game_state.active_enemies -= 1;
                        }
                    }
                     if (CheckCollisionRecs(local_game_state.player_projectiles[projectile].rectangle, local_game_state.powerup.rectangle)) {
                        //PlaySound(resources.powerup_boost_sound);
                        local_game_state.player_projectiles[projectile].active = false;
                        local_game_state.active_projectiles -= 1;
                        InitPowerupEntity(local_game_state.powerup);
                        StartTimer(&powerup_timer, local_game_state.powerup.lifetime);
                    }
                }
            }
        
            UpdateTimer(&powerup_timer);

            if (!TimerDone(&powerup_timer)) {
                PowerupTimerActive(local_game_state.player_one);
                PowerupTimerActive(local_game_state.player_two);
            }

            if (TimerDone(&powerup_timer)) {
                PowerupTimerDone(local_game_state.player_one);
                PowerupTimerDone(local_game_state.player_two);
            }

            for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
                if (local_game_state.enemies[enemy].active) {
                    local_game_state.enemies[enemy].rectangle.y += local_game_state.enemies[enemy].speed;
                    if (local_game_state.enemies[enemy].rectangle.y > screen_height) { RespawnEnemyPosition(local_game_state.enemies[enemy]); }
                    if (CheckCollisionRecs(local_game_state.enemies[enemy].rectangle, local_game_state.player_one.rectangle)) {
                        //PlaySound(resources.lost_life_sound);
                        local_game_state.player_one.player_lives_left -= 1;
                        RespawnEnemyPosition(local_game_state.enemies[enemy]);
                    }
                    if (CheckCollisionRecs(local_game_state.enemies[enemy].rectangle, local_game_state.player_two.rectangle)) {
                        //PlaySound(resources.lost_life_sound);
                        local_game_state.player_two.player_lives_left -= 1;
                        RespawnEnemyPosition(local_game_state.enemies[enemy]);
                    }
                    int check_for_powerup = GetRandomValue(0, 10);
                    if (check_for_powerup == 2) { local_game_state.powerup.active = true; }
                }
            }
            if (local_game_state.powerup.rectangle.y > screen_height) { InitPowerupEntity(local_game_state.powerup); }
            if (local_game_state.powerup.active) { local_game_state.powerup.rectangle.y += local_game_state.powerup.speed; }
        }
        if (local_game_state.player_one.player_lives_left <= 0 || local_game_state.player_two.player_lives_left <= 0) {local_game_state.game_over = true;}
        if (local_game_state.active_enemies == 0) {local_game_state.victory = true;}

        if (local_game_state.game_over) {
            //StopMusicStream(resources.background_music);
            //PlayMusicStream(resources.game_over_music);
            //UpdateMusicStream(resources.game_over_music);
            if (IsKeyPressed(KEY_ENTER)) {
                //StopMusicStream(resources.game_over_music);
                //PlayMusicStream(resources.background_music);
                //UpdateMusicStream(resources.background_music);
                InitLocalGameState(local_game_state);
                local_game_state.opener_screen = false;
                ai_blackboard.high_level_state = HighLevelAIState::None;
                ai_blackboard.low_level_state = LowLevelAIState::CalculateTargetLocation;
            }
        }

// RENDER
        RenderGame(local_game_state.player_projectiles, local_game_state.enemies, local_game_state.player_one, local_game_state.player_two, local_game_state.powerup, resources, ai_blackboard);

// NETWORK
        GenerateServerPacketFromLocalGameState(server_packet_to_send, local_game_state);

        UpdateTimer(&send_timer);
        if (TimerDone(&send_timer)) {
            sendto(server_socket, &server_packet_to_send, sizeof(server_packet_to_send), 0, (struct sockaddr*)&client_addr, client_addr_len);
            StartTimer(&send_timer, 0.05f);
        }
    }
    UnloadResources(resources);
    CloseAudioDevice();
    CloseWindow();
    close(server_socket);
    return 0;
};
