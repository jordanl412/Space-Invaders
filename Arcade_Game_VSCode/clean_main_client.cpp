
#include <iostream>
#include "raylib.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

const int NUM_ENEMIES = 30;
const int NUM_SHOOTS = 20;

typedef enum {
        TYPE_PLAYER,
        TYPE_PLAYER_PROJECTILE,
        TYPE_ENEMY,
        TYPE_POWERUP,
    } EntityType;

typedef struct Entity {
    EntityType type;
    Texture2D texture;
    float speed;
    bool active;
    Rectangle rectangle; // x/y upper left corner position, width/height dimensions
    float lifetime;
} Entity;

typedef struct LightEnemy {
    Rectangle rectangle;
    bool active;
} LightEnemy;

typedef struct LightProjectile {
    Rectangle rectangle;
    bool active;
} LightProjectile;

typedef struct ClientPacket {
    bool right_key_down;
    bool left_key_down;
    bool space_key_down;
    bool wants_to_play;
    bool play_again;
} ClientPacket;

typedef struct LightGameState {
    Entity player_one;
    Entity player_two;
    LightEnemy enemies_to_send[NUM_ENEMIES];
    int active_enemies;
    LightProjectile projectiles_to_send[NUM_SHOOTS];
    int active_projectiles;
    Entity powerup;
    bool game_over;
    bool pause_button;
    int lives_left;
    int enemies_killed;
    bool victory;
    bool opener_screen;
    bool help;
    bool player_one_powered_up;
    bool player_two_powered_up;
} LightGameState;

bool game_over = false;
bool pause_button = false;
int lives_left = 3;
int enemies_killed = 0;
int active_enemies = 30;
int active_projectiles = 0;
bool victory = false;
bool opener_screen = true;
bool help = false;
bool player_one_powered_up = false;
bool player_two_powered_up = false;

bool right_key_down = false;
bool left_key_down = false;
bool space_key_down = false;
bool wants_to_play = false;
bool play_again = false;

const int screen_width = 1200;
const int screen_height = 800;

typedef struct
    {
        float Lifetime;
    } Timer;

// start/restart timer
void StartTimer(Timer* timer, float lifetime)
{
    if (timer != NULL)
    {
        timer->Lifetime = lifetime;
    }
}

// update timer with current frame time
void UpdateTimer(Timer* timer)
{
    // decrease timer as frames pass
    if (timer != NULL && timer->Lifetime > 0)
    {
        timer->Lifetime -= GetFrameTime();
    }
}

// check if timer is done
bool TimerDone(Timer* timer)
{
    if (timer != NULL)
    {
        return timer->Lifetime <= 0;
    } else {
        return true;
    }
}

int main() {

    InitWindow(screen_width, screen_height, "Space Invaders");
/*
    InitAudioDevice();
    Music opener_music = LoadMusicStream("res/opener_screen_music.mp3");
    Music background_music = LoadMusicStream("res/space_invaders_background.mp3");
    SetMusicVolume(background_music, 0.25);
    Sound killed_enemy_sound = LoadSound("res/enemy_killed.mp3");
    Sound powerup_boost_sound = LoadSound("res/boost_sound.mp3");
    Sound lost_life_sound = LoadSound("res/lost_life.mp3");
    Music game_over_music = LoadMusicStream("res/game_over1.mp3");
    PlayMusicStream(background_music);
    float timePlayed = 0.0f;
*/
    Texture normal_player_texture = LoadTexture("res/player_icon.png");
    Texture powerup_player_texture = LoadTexture("res/powerup_player_icon.png");
    Texture enemy_texture = LoadTexture("res/enemy_icon.png");
    Texture projectile_texture = LoadTexture("res/player_projectile.png");
    Texture powerup_texture = LoadTexture("res/powerup_icon.png");

    LightGameState received_server_packet;

    ClientPacket client_packet_to_send = {0};

    Entity player_one;
    player_one.type = TYPE_PLAYER;
    player_one.texture = normal_player_texture;
    player_one.rectangle.width = 40;
    player_one.rectangle.height = 40;
    player_one.rectangle.x = (float)screen_width/2 - 20;
    player_one.rectangle.y = (float)screen_height - 40;
    player_one.speed = 1.0f;

    Entity player_two;
    player_two.type = TYPE_PLAYER;
    player_two.texture = normal_player_texture;
    player_two.rectangle.width = 40;
    player_two.rectangle.height = 40;
    player_two.rectangle.x = (float)screen_width/2 - 20;
    player_two.rectangle.y = (float)screen_height - 40;
    player_two.speed = 1.0f;   



    //const int NUM_SHOOTS = 20;
    Entity player_projectiles[NUM_SHOOTS];

    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    {
        player_projectiles[projectile].type = TYPE_PLAYER_PROJECTILE;
        player_projectiles[projectile].texture = projectile_texture;
        player_projectiles[projectile].rectangle.width = 10;
        player_projectiles[projectile].rectangle.height = 15;
        player_projectiles[projectile].speed = 1.0f;
        player_projectiles[projectile].active = false;
    }
    int projectile_rectangle_width = 10;
    int projectile_rectangle_height = 15;

    LightProjectile projectiles_to_send[NUM_SHOOTS];
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    {
        projectiles_to_send[projectile].active = false;
    }

    //const int NUM_ENEMIES = 30;
    Entity enemies[NUM_ENEMIES];
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
    {
        enemies[enemy].type = TYPE_ENEMY;
        enemies[enemy].texture = enemy_texture;
        enemies[enemy].rectangle.x = GetRandomValue(0, screen_width - 2*enemies[enemy].rectangle.width);
        enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
        enemies[enemy].rectangle.width = 25;
        enemies[enemy].rectangle.height = 25;
        enemies[enemy].speed = 1.5f;
        enemies[enemy].active = true;
    }
    int enemy_rectangle_width = 25;
    int enemy_rectangle_height = 25;

    LightEnemy enemies_to_send[NUM_ENEMIES];
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
    {
        enemies_to_send[enemy].active = true;
    }    

    Entity powerup;
    powerup.type = TYPE_POWERUP;
    powerup.texture = powerup_texture;
    powerup.rectangle.x = GetRandomValue(0, screen_width-2*powerup.rectangle.width);
    powerup.rectangle.y = GetRandomValue(-screen_height*1.5, -10);
    powerup.rectangle.width = 25;
    powerup.rectangle.height = 25;
    powerup.speed = 1.5f;
    powerup.active = false;
    powerup.lifetime = 4.0f;
    Timer powerupTimer = {0};

    int frame_counter = 0;

    // CLIENT CODE
    // Create struct for server's address
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

    int reuse = 1;
    if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Failed to set SO_REUSEADDR");
        return -1;
    }

    socklen_t serv_addr_len = sizeof(serv_addr);
    

    SetTargetFPS(120);
    while (!WindowShouldClose())
    {
        // CLIENT CODE

        int bytes_read = recvfrom(client_socket, &received_server_packet, sizeof(received_server_packet), 0, (struct sockaddr*)&serv_addr, &serv_addr_len);
        if (bytes_read > 0) {
            game_over = received_server_packet.game_over;
            pause_button = received_server_packet.pause_button;
            lives_left = received_server_packet.lives_left;
            enemies_killed = received_server_packet.enemies_killed;
            active_enemies = received_server_packet.active_enemies;
            victory = received_server_packet.victory;
            opener_screen = received_server_packet.opener_screen;
            help = received_server_packet.help;
            player_one = received_server_packet.player_one;
            player_two = received_server_packet.player_two;
            player_one_powered_up = received_server_packet.player_one_powered_up;
            player_two_powered_up = received_server_packet.player_two_powered_up;
            for (int enemy = 0; enemy < NUM_ENEMIES; enemy++) {
                if (received_server_packet.enemies_to_send[enemy].active) {
                    enemies[enemy].rectangle.x = received_server_packet.enemies_to_send[enemy].rectangle.x;
                    enemies[enemy].rectangle.y = received_server_packet.enemies_to_send[enemy].rectangle.y;
                    enemies[enemy].active = received_server_packet.enemies_to_send[enemy].active;
                } else {
                    enemies[enemy].active = false;
                }
            }
            for (int projectile = 0; projectile < NUM_SHOOTS; projectile++) { 
                if (received_server_packet.projectiles_to_send[projectile].active) {
                    player_projectiles[projectile].rectangle.x = received_server_packet.projectiles_to_send[projectile].rectangle.x;
                    player_projectiles[projectile].rectangle.y = received_server_packet.projectiles_to_send[projectile].rectangle.y;
                    player_projectiles[projectile].rectangle.width = received_server_packet.projectiles_to_send[projectile].rectangle.width;
                    player_projectiles[projectile].rectangle.height = received_server_packet.projectiles_to_send[projectile].rectangle.height;
                    player_projectiles[projectile].active = received_server_packet.projectiles_to_send[projectile].active;
                } else {
                    player_projectiles[projectile].active = false;
                }
            }
            powerup = received_server_packet.powerup;
            active_projectiles = received_server_packet.active_projectiles;
            if (active_projectiles > 0) {
                int foo = 5;
            }
        }

// INPUT

        if (!pause_button && !game_over && !opener_screen)
        {
            // Player movements left and right
            if (IsKeyDown(KEY_RIGHT))
            {
                client_packet_to_send.right_key_down = true;
            } else {
                client_packet_to_send.right_key_down = false;
            }
            if (IsKeyDown(KEY_LEFT))
            {
                client_packet_to_send.left_key_down = true;
            } else {
                client_packet_to_send.left_key_down = false;
            }

            // Projectile firing
            if (IsKeyDown(KEY_SPACE))
            {
                client_packet_to_send.space_key_down = true;
            } else {
                client_packet_to_send.space_key_down = false;
            }
        }


// RENDER
        BeginDrawing();
            ClearBackground(BLACK);
            if (!opener_screen && !help)
            {
                DrawText(TextFormat("Enemies Killed: %i/30", enemies_killed), 10, 10, 35, WHITE);
                DrawText(TextFormat("Lives: %i", lives_left), screen_width*.86, 10, 35, WHITE);
            }
            if (!game_over && !victory && !opener_screen)
            {
                for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
                {
                    if (player_projectiles[projectile].active)
                    {
                        Rectangle player_projectile_sourceRec{0.0f, 0.0f, (float)player_projectiles[projectile].texture.width, (float)player_projectiles[projectile].texture.height};
                        DrawTexturePro(player_projectiles[projectile].texture, player_projectile_sourceRec, player_projectiles[projectile].rectangle, {0,0}, 0.0f, WHITE);
                    }
                }
                for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
                {
                    if (enemies[enemy].active)
                    {
                        Rectangle enemy_sourceRec{0.0f, 0.0f, (float)enemies[enemy].texture.width, (float)enemies[enemy].texture.height};
                        DrawTexturePro(enemies[enemy].texture, enemy_sourceRec, enemies[enemy].rectangle, {0,0}, 0.0f, WHITE);
                    }
                }
                Rectangle powerup_sourceRec{0.0f, 0.0f, (float)powerup.texture.width, (float)powerup.texture.height};
                DrawTexturePro(powerup.texture, powerup_sourceRec, powerup.rectangle, {0,0}, 0.0f, WHITE);
            }
            if (!opener_screen && !help)
            {
                if (player_one_powered_up == true) {
                    player_one.texture = powerup_player_texture;
                    Rectangle player_one_sourceRec{0.0f, 0.0f, (float)player_one.texture.width, (float)player_one.texture.height};
                    DrawTexturePro(player_one.texture, player_one_sourceRec, player_one.rectangle, {0,0}, 0.0f, WHITE);
                }
                if (player_one_powered_up == false) {
                    player_one.texture = normal_player_texture;
                    Rectangle player_one_sourceRec{0.0f, 0.0f, (float)player_one.texture.width, (float)player_one.texture.height};
                    DrawTexturePro(player_one.texture, player_one_sourceRec, player_one.rectangle, {0,0}, 0.0f, WHITE);
                }
                if (player_two_powered_up == true) {
                    player_two.texture = powerup_player_texture;
                    Rectangle player_two_sourceRec{0.0f, 0.0f, (float)player_two.texture.width, (float)player_two.texture.height};
                    DrawTexturePro(player_two.texture, player_two_sourceRec, player_two.rectangle, {0,0}, 0.0f, WHITE);
                }
                if (player_two_powered_up == false) {
                    player_two.texture = normal_player_texture;
                    Rectangle player_two_sourceRec{0.0f, 0.0f, (float)player_two.texture.width, (float)player_two.texture.height};
                    DrawTexturePro(player_two.texture, player_two_sourceRec, player_two.rectangle, {0,0}, 0.0f, WHITE);
                }
            }
            if (pause_button)
            {
                DrawText("Game Paused", (screen_width/2)-180, (screen_height/2)-40, 50, WHITE);
            }
            if (opener_screen)
            {
                DrawText("SPACE INVADERS", (screen_width/2)-300, (screen_height/2)-80, 70, GREEN);
                DrawText("Hit Enter to play", (screen_width/2)-150, (screen_height/2)+20, 40, WHITE);
                DrawText("- Hit H for help -", (screen_width/2)-80, (screen_height/2)+90, 25, WHITE);
            }
            if (victory)
            {
                DrawText("Level Complete!", (screen_width/2)-180, (screen_height/2)-40, 50, GREEN);
            } 
            if (game_over)
            {
                DrawText("GAME OVER", (screen_width/2)-140, (screen_height/2)-40, 50, RED);
                DrawText("Hit Enter to play again", (screen_width/2)-180, (screen_height/2)+50, 35, WHITE);
            }
        EndDrawing();

// NETWORK
        if (frame_counter % 3 == 0) {
            sendto(client_socket, &client_packet_to_send, sizeof(client_packet_to_send), 0, (struct sockaddr*)&serv_addr, serv_addr_len);
        }
        frame_counter += 1;
        
    };

    //De-Initialization
    UnloadTexture(player_one.texture);
    UnloadTexture(player_two.texture);
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    {
        UnloadTexture(player_projectiles[projectile].texture);
    }
    UnloadTexture(powerup.texture);
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
    {
        UnloadTexture(enemies[enemy].texture);
    }
    //UnloadMusicStream(background_music);
    //UnloadSound(killed_enemy_sound);
    //UnloadSound(powerup_boost_sound);
    //UnloadSound(lost_life_sound);
    //CloseAudioDevice();
    CloseWindow();
    close(client_socket);
    return 0;
};

