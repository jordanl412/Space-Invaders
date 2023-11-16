#include <iostream>
#include "raylib.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

const int NUM_ENEMIES = 30;
const int NUM_SHOOTS = 20;
const int screen_width = 1200;
const int screen_height = 800;

typedef struct GameResources {
    Music opener_music;
    Music background_music;
    Music game_over_music;
    Sound killed_enemy_sound;
    Sound powerup_boost_sound;
    Sound lost_life_sound;
    Texture normal_player_texture;
    Texture powerup_player_texture;
    Texture enemy_texture;
    Texture projectile_texture;
    Texture powerup_texture;
} GameResources; // 392UL
extern GameResources resources;

void LoadResources(GameResources& resources) {
    resources.opener_music = LoadMusicStream("res/opener_screen_music.mp3");
    resources.background_music = LoadMusicStream("res/space_invaders_background.mp3");
    resources.game_over_music = LoadMusicStream("res/game_over1.mp3");
    resources.killed_enemy_sound = LoadSound("res/enemy_killed.mp3");
    resources.powerup_boost_sound = LoadSound("res/boost_sound.mp3");
    resources.lost_life_sound = LoadSound("res/lost_life.mp3");
    resources.normal_player_texture = LoadTexture("res/player_icon.png");
    resources.powerup_player_texture = LoadTexture("res/powerup_player_icon.png");
    resources.enemy_texture = LoadTexture("res/enemy_icon.png");
    resources.projectile_texture = LoadTexture("res/player_projectile.png");
    resources.powerup_texture = LoadTexture("res/powerup_icon.png");
}

void UnloadResources(GameResources& resources) {
    UnloadMusicStream(resources.opener_music);
    UnloadMusicStream(resources.background_music);
    UnloadMusicStream(resources.game_over_music);
    UnloadSound(resources.killed_enemy_sound);
    UnloadSound(resources.powerup_boost_sound);
    UnloadSound(resources.lost_life_sound);
    UnloadTexture(resources.normal_player_texture);
    UnloadTexture(resources.powerup_player_texture);
    UnloadTexture(resources.enemy_texture);
    UnloadTexture(resources.projectile_texture);
    UnloadTexture(resources.powerup_texture);
}

typedef enum {
        TYPE_PLAYER,
        TYPE_PLAYER_PROJECTILE,
        TYPE_ENEMY,
        TYPE_POWERUP,
} EntityType; // 4UL

typedef struct Entity {
    EntityType type;
    Texture2D texture;
    Rectangle rectangle;
    int player_lives_left;
    float speed;
    float lifetime;
    bool active;
    bool powered_up;
} Entity; // 56UL

typedef struct LightEnemy {
    Rectangle rectangle;
    bool active;
} LightEnemy; // 20UL

typedef struct LightProjectile {
    Rectangle rectangle;
    bool active;
} LightProjectile; // 20UL

typedef struct ClientInputs {
    bool client_right_key_down;
    bool client_left_key_down;
    bool client_space_key_down;
    bool client_wants_to_play;
    bool client_play_again;
} ClientInputs; // 5UL

typedef struct LightGameState { 
    Entity player_one;
    Entity player_two;
    Entity powerup;
    LightEnemy enemies[NUM_ENEMIES];
    int active_enemies;
    LightProjectile projectiles[NUM_SHOOTS];
    int active_projectiles;
    int enemies_killed;
    bool game_over;
    bool victory;
    bool opener_screen;
} LightGameState; // 1184UL

typedef struct GameState {
    Entity enemies[NUM_ENEMIES];
    Entity player_projectiles[NUM_SHOOTS];
    Entity player_one;
    Entity player_two;
    Entity powerup;
    int enemies_killed;
    int active_enemies;
    int active_projectiles;
    bool game_over;
    bool victory;
    bool opener_screen;
    bool client_right_key_down;
    bool client_left_key_down;
    bool client_space_key_down;
    bool client_wants_to_play;
    bool client_play_again;
} GameState; // 20UL or 2988UL (with all entities added)
extern GameState local_game_state;

void InitPlayerEntity(Entity& player) {
    player.type = TYPE_PLAYER;
    player.texture = resources.normal_player_texture;
    player.rectangle = {(float)screen_width/2 - 20, (float)screen_height - 40, 40, 40};
    player.speed = 1.0f;
    player.powered_up = false;
    player.player_lives_left = 3;
}

void InitPowerupEntity(Entity& powerup) {
    powerup.type = TYPE_POWERUP;
    powerup.texture = resources.powerup_texture;
    powerup.rectangle = {(float)GetRandomValue(0, screen_width-2*powerup.rectangle.width), (float)GetRandomValue(-screen_height*1.5, -10), 25, 25};
    powerup.speed = 1.5f;
    powerup.active = false;
    powerup.lifetime = 4.0f;
}

void InitProjectileEntity(Entity& projectile) {
    projectile.type = TYPE_PLAYER_PROJECTILE;
    projectile.texture = resources.projectile_texture;
    projectile.rectangle = {0, 0, 10, 15};
    projectile.speed = 1.0f;
    projectile.active = false;
}

void InitEnemyEntities(Entity& enemy) {
    enemy.type = TYPE_ENEMY;
    enemy.texture = resources.enemy_texture;
    enemy.rectangle = {(float)GetRandomValue(0, screen_width - 2*enemy.rectangle.width), (float)GetRandomValue(-screen_height*1.5, -10), 25, 25};
    enemy.speed = 1.5f;
    enemy.active = true;
}

void RespawnEnemyPosition(Entity& enemy) {
    enemy.rectangle.x = GetRandomValue(0, screen_width-2*enemy.rectangle.width);
    enemy.rectangle.y = GetRandomValue(-screen_height*1.5, -10);
}

void InitLocalGameState(GameState& local_game_state) {
    local_game_state.enemies_killed = 0;
    local_game_state.active_projectiles = 0;
    local_game_state.game_over = false;
    local_game_state.victory = false;
    local_game_state.client_right_key_down = false;
    local_game_state.client_left_key_down = false;
    local_game_state.client_space_key_down = false;
    local_game_state.client_wants_to_play = false;
    local_game_state.client_play_again = false;
    local_game_state.active_enemies = 30;
    local_game_state.opener_screen = true;
    InitPlayerEntity(local_game_state.player_one);
    InitPlayerEntity(local_game_state.player_two);
    InitPowerupEntity(local_game_state.powerup);
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
        InitEnemyEntities(local_game_state.enemies[enemy]);
    }
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) {
        InitProjectileEntity(local_game_state.player_projectiles[projectile]);
    }
}

void GenerateServerPacketFromLocalGameState(LightGameState& server_packet_to_send, GameState& local_game_state) {
    server_packet_to_send.active_enemies = local_game_state.active_enemies;
    server_packet_to_send.active_projectiles = local_game_state.active_projectiles;
    server_packet_to_send.enemies_killed = local_game_state.enemies_killed;
    server_packet_to_send.game_over = local_game_state.game_over;
    server_packet_to_send.victory = local_game_state.victory;
    server_packet_to_send.opener_screen = local_game_state.opener_screen;
    server_packet_to_send.player_one = local_game_state.player_one;
    server_packet_to_send.player_two = local_game_state.player_two;
    server_packet_to_send.powerup = local_game_state.powerup;
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) {
            server_packet_to_send.projectiles[projectile].rectangle = local_game_state.player_projectiles[projectile].rectangle;
            server_packet_to_send.projectiles[projectile].active = local_game_state.player_projectiles[projectile].active;
        } // can make a conversion function from full struct to light struct
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
        server_packet_to_send.enemies[enemy].rectangle = local_game_state.enemies[enemy].rectangle;
        server_packet_to_send.enemies[enemy].active = local_game_state.enemies[enemy].active;
    }
}

void GenerateClientStateFromServerPacket(GameState& local_client_game_state, LightGameState& received_server_packet) {
    local_client_game_state.game_over = received_server_packet.game_over;
    local_client_game_state.enemies_killed = received_server_packet.enemies_killed;
    local_client_game_state.active_enemies = received_server_packet.active_enemies;
    local_client_game_state.victory = received_server_packet.victory;
    local_client_game_state.opener_screen = received_server_packet.opener_screen;
    local_client_game_state.active_projectiles = received_server_packet.active_projectiles;
    local_client_game_state.player_one = received_server_packet.player_one;
    local_client_game_state.player_two = received_server_packet.player_two;
    local_client_game_state.powerup = received_server_packet.powerup;
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
        local_client_game_state.enemies[enemy].rectangle = received_server_packet.enemies[enemy].rectangle;
        local_client_game_state.enemies[enemy].active = received_server_packet.enemies[enemy].active;
    }
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) { 
        local_client_game_state.player_projectiles[projectile].rectangle = received_server_packet.projectiles[projectile].rectangle;
        local_client_game_state.player_projectiles[projectile].active = received_server_packet.projectiles[projectile].active;
    }
}

void MovePlayerRight(Entity& player) {
    if (player.rectangle.x < (screen_width - player.rectangle.width)) {
        player.rectangle.x += player.speed;
    }
}

void MovePlayerLeft(Entity& player) {
    if (player.rectangle.x > 0) {
        player.rectangle.x -= player.speed;
    }
}

void ShootProjectile(Entity projectiles[], Entity& player, int& active_projectiles) {
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) {
        if (!projectiles[projectile].active) {
            projectiles[projectile].active = true;
            projectiles[projectile].rectangle.x = player.rectangle.x + player.rectangle.width/2 - projectiles[projectile].rectangle.width/2;
            projectiles[projectile].rectangle.y = player.rectangle.y;
            active_projectiles += 1;
            break;
        }
    }
}

// TIMER
typedef struct { float Lifetime; } Timer;

void StartTimer(Timer* timer, float lifetime) {
    if (timer != NULL) { timer->Lifetime = lifetime; }
}
void UpdateTimer(Timer* timer) {
    if (timer != NULL && timer->Lifetime > 0) { timer->Lifetime -= GetFrameTime(); }
}
bool TimerDone(Timer* timer) {
    if (timer != NULL) { return timer->Lifetime <= 0;
    } else { return true; }
}

void PowerupTimerActive(Entity& player) {
    player.speed = 3.0f;
    player.powered_up = true;
}

void PowerupTimerDone(Entity& player) {
    player.speed = 1.0f;
    player.powered_up = false;
}

void DrawTextureWithSourceRec(Entity& entity_to_draw) {
    Rectangle source_rec = {0.0f, 0.0f, (float)entity_to_draw.texture.width, (float)entity_to_draw.texture.height};
    DrawTexturePro(entity_to_draw.texture, source_rec, entity_to_draw.rectangle, {0.0}, 0.0f, WHITE);
}

// RENDER
void RenderGame(Entity player_projectiles[], Entity enemies[], Entity& player_one, Entity& player_two, Entity& powerup, GameResources& resources) {
    BeginDrawing();
            ClearBackground(BLACK);
            if (!local_game_state.game_over && !local_game_state.victory && !local_game_state.opener_screen) {
                for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) {
                    if (player_projectiles[projectile].active) {
                        DrawTextureWithSourceRec(player_projectiles[projectile]);
                    }
                }
                for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
                    if (enemies[enemy].active) {
                        DrawTextureWithSourceRec(enemies[enemy]);
                    }
                }
                DrawTextureWithSourceRec(powerup);
            }
            if (!local_game_state.opener_screen) {
                player_one.texture = player_one.powered_up ? resources.powerup_player_texture : resources.normal_player_texture;
                DrawTextureWithSourceRec(player_one);
                player_two.texture = player_two.powered_up ? resources.powerup_player_texture : resources.normal_player_texture;
                DrawTextureWithSourceRec(player_two);
            }
            if (!local_game_state.opener_screen) {
                DrawText(TextFormat("Enemies Killed: %i/30", local_game_state.enemies_killed), 10, 10, 35, WHITE);
                DrawText(TextFormat("Lives: %i", player_one.player_lives_left), screen_width*.86, 10, 35, WHITE);
            }
            if (local_game_state.opener_screen) {
                DrawText("SPACE INVADERS", (screen_width/2)-300, (screen_height/2)-80, 70, GREEN);
                DrawText("Hit Enter to play", (screen_width/2)-150, (screen_height/2)+20, 40, WHITE);
            }
            if (local_game_state.victory) {
                DrawText("Level Complete!", (screen_width/2)-180, (screen_height/2)-40, 50, GREEN);
            } 
            if (local_game_state.game_over) {
                DrawText("GAME OVER", (screen_width/2)-140, (screen_height/2)-40, 50, RED);
                DrawText("Hit Enter to play again", (screen_width/2)-180, (screen_height/2)+50, 35, WHITE);
            }
    EndDrawing();
}
// NETWORK
int createSocketConnection(const char* address, bool isServer, struct sockaddr_in serv_addr) {
    int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_desc < 0) {
        perror("Failed to create socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Failed to set SO_REUSEADDR");
        return -1;
    }

    if (isServer) {
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(socket_desc, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Failed to bind to socket");
            return -1;
        }
    } else {
        inet_pton(AF_INET, address, &serv_addr.sin_addr);
    }

    int flags = fcntl(socket_desc, F_GETFL, 0);
    fcntl(socket_desc, F_SETFL, flags | O_NONBLOCK);
    return socket_desc;
}