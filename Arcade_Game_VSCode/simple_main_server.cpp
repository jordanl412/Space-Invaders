#include <iostream>
#include "raylib.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

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

const int screen_width = 1200;
const int screen_height = 800;

typedef struct GameState {
    bool game_over;
    //bool pause;
    int lives_left;
    int enemies_killed;
    int active_enemies;
    bool victory;
    //bool opener_screen;
    //bool help;
} GameState;

typedef struct ClientPacket {
    int client_id;
    bool right_key_down;
    bool left_key_down;
    bool player_fired;
    bool enemy_killed;
    bool player_powered_up;
    bool player_lost_life;
} ClientPacket;

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
    }
}

int main() {

    // SERVER IMPLEMENTATION
    int server_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct ClientPacket received_client_packet;
    struct GameState game_state;
    char buffer[sizeof(struct ClientPacket)];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2112);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Failed to create server socket");
        return -1;
    }

    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags == -1) {
        perror("Failed to get socket flags");
        return -1;
    }

    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Failed to set socket to non-blocking");
        return -1;
    }

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) {
        perror("Failed to bind server socket");
        return -1;
    }

    printf("Server is listening on port 2112...");
    
    // END OF SERVER IMPLEMENTATION

    InitWindow(screen_width, screen_height, "Space Invaders");

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

    game_state.game_over = false;
    //game_state.pause = false;
    game_state.lives_left = 3;
    game_state.enemies_killed = 0;
    game_state.active_enemies = 30;
    game_state.victory = false;
    //game_state.opener_screen = true;
    //game_state.help = false;

    Entity player;
    player.type = TYPE_PLAYER;
    player.texture = LoadTexture("res/player_icon.png");
    player.rectangle.width = 40;
    player.rectangle.height = 40;
    player.rectangle.x = (float)screen_width/2 - 20;
    player.rectangle.y = (float)screen_height - 40;
    player.speed = 1.0f;

    const int NUM_SHOOTS = 20;
    Entity player_projectiles[NUM_SHOOTS];

    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    {
        player_projectiles[projectile].type = TYPE_PLAYER_PROJECTILE;
        player_projectiles[projectile].texture = LoadTexture("res/player_projectile.png");
        player_projectiles[projectile].rectangle.x;
        player_projectiles[projectile].rectangle.y;
        player_projectiles[projectile].rectangle.width = 10;
        player_projectiles[projectile].rectangle.height = 15;
        player_projectiles[projectile].speed = 1.0f;
        player_projectiles[projectile].active = false;
    }

    const int NUM_ENEMIES = 30;
    Entity enemies[NUM_ENEMIES];
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
    {
        enemies[enemy].type = TYPE_ENEMY;
        enemies[enemy].texture = LoadTexture("res/enemy_icon.png");
        enemies[enemy].rectangle.x = GetRandomValue(0, screen_width - 2*enemies[enemy].rectangle.width);
        enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
        enemies[enemy].rectangle.width = 25;
        enemies[enemy].rectangle.height = 25;
        enemies[enemy].speed = 1.5f;
        enemies[enemy].active = true;
    }

    Entity powerup;
    powerup.type = TYPE_POWERUP;
    powerup.texture = LoadTexture("res/powerup_icon.png");
    powerup.rectangle.x = GetRandomValue(0, screen_width-2*powerup.rectangle.width);
    powerup.rectangle.y = GetRandomValue(-screen_height*1.5, -10);
    powerup.rectangle.width = 25;
    powerup.rectangle.height = 25;
    powerup.speed = 1.5f;
    powerup.active = false;
    powerup.lifetime = 4.0f;
    Timer powerupTimer = {0};
    
    SetTargetFPS(120);
    while (!WindowShouldClose())
    {
        // get client_packet from client
        int num_bytes = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
        if (num_bytes == sizeof(struct ClientPacket)) {
            memcpy(&received_client_packet, buffer, sizeof(struct ClientPacket));
/*
            received_client_packet = {
                received_client_packet.client_id = received_client_packet.client_id;
                received_client_packet.right_key_down = received_client_packet.right_key_down;
                received_client_packet.left_key_down = received_client_packet.left_key_down;
                received_client_packet.player_fired = received_client_packet.player_fired;
                received_client_packet.enemy_killed = received_client_packet.enemy_killed;
                received_client_packet.player_powered_up = received_client_packet.player_powered_up;
                received_client_packet.player_lost_life = received_client_packet.player_lost_life;
            }
*/
        }

        // update positions if NOT PAUSED and NOT GAME OVER
        if (!game_state.game_over)
        {
            UpdateMusicStream(background_music);
            timePlayed = GetMusicTimePlayed(background_music)/GetMusicTimeLength(background_music);
            if (timePlayed > 1.0f) timePlayed = 1.0f; // Time played can't be longer than the length of the music

            // Player movements left and right
            if (received_client_packet.right_key_down)
            {
                if (player.rectangle.x < (screen_width - player.rectangle.width))
                {
                    player.rectangle.x += player.speed;
                }
            }
            if (received_client_packet.left_key_down)
            {
                if (player.rectangle.x > 0)
                {
                    player.rectangle.x -= player.speed;
                }
            }

            // activating player_projectile, shooting with spacebar
            if (received_client_packet.player_fired) 
            {
                for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
                {
                    if (!player_projectiles[projectile].active)
                    {
                        player_projectiles[projectile].active = true; 
                        player_projectiles[projectile].rectangle.x = player.rectangle.x + player.rectangle.width/2 - player_projectiles[projectile].rectangle.width/2;
                        player_projectiles[projectile].rectangle.y = player.rectangle.y;
                        break;
                    }
                }
            }

            // active player_projectile logic
            for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
            {
                if (player_projectiles[projectile].active)
                {
                    player_projectiles[projectile].rectangle.y -= player_projectiles[projectile].speed;
                    // remove projectile if it reaches the top of the screen
                    if (player_projectiles[projectile].rectangle.y <= (0))
                    {
                        player_projectiles[projectile].active = false;
                        player_projectiles[projectile].rectangle.x = player.rectangle.x + player.rectangle.width*.75;
                        player_projectiles[projectile].rectangle.y = player.rectangle.y + player.rectangle.height/4;
                    }
                    // check for collision with enemy
                    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
                    {
                        // enemy and projectile
                        if (received_client_packet.enemy_killed)
                        {
                            // remove the projectile that hit the enemy
                            player_projectiles[projectile].active = false;
                            // remove the enemy that was hit
                            enemies[enemy].rectangle.x = GetRandomValue(0, screen_width - 2*enemies[enemy].rectangle.width);
                            enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                            enemies[enemy].active = false;
                            game_state.enemies_killed += 1;
                            game_state.active_enemies -= 1;
                        }
                    }
                    // check for collision with powerup
                     if (received_client_packet.player_powered_up)
                    {
                        // remove the projectile that hit the powerup
                        player_projectiles[projectile].active = false;
                        // remove the powerup
                        powerup.rectangle.x = GetRandomValue(0, screen_width-2*powerup.rectangle.width);
                        powerup.rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                        powerup.active = false;
                        // give the player a powerup
                        StartTimer(&powerupTimer, powerup.lifetime);
                    }
                }
            }
        
            // powerup timer logic
            UpdateTimer(&powerupTimer);

            if (!TimerDone(&powerupTimer))
            {
                player.texture = LoadTexture("res/powerup_player_icon.png");
                player.speed = 3.0f;
            }

            if (TimerDone(&powerupTimer))
            {
                player.texture = LoadTexture("res/player_icon.png");
                player.speed = 1.0f;
            }

            // enemies logic
            for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
            {
                if (enemies[enemy].active)
                {
                    enemies[enemy].rectangle.y += enemies[enemy].speed;
                    // remove and reset enemy if it reaches the bottom of the screen
                    if (enemies[enemy].rectangle.y > screen_height)
                    {
                        enemies[enemy].rectangle.x = GetRandomValue(0, screen_width-2*enemies[enemy].rectangle.width);
                        enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                    }

                    // check for enemy player collision
                    if (received_client_packet.player_lost_life)
                    {
                        game_state.lives_left -= 1;
                        // remove and reset the enemy that hit the player
                        enemies[enemy].rectangle.x = GetRandomValue(0, screen_width-2*enemies[enemy].rectangle.width);
                        enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                    }

                    // check for powerup
                    int check_for_powerup = GetRandomValue(0, 10);
                    if (check_for_powerup == 2)
                    {
                        powerup.active = true;
                    }
                }
            }
            // remove and reset powerup if it reaches the bottom of the screen
            if (powerup.rectangle.y > screen_height)
            {
                powerup.rectangle.x = GetRandomValue(0, screen_width-2*powerup.rectangle.width);
                powerup.rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                powerup.active = false;
            }
            // move active powerup
            if (powerup.active)
            {
                powerup.rectangle.y += powerup.speed;
            }
        }

        if (game_state.lives_left <= 0)
        {
            game_state.game_over = true;
        }

        if (game_state.active_enemies == 0)
        {
            game_state.victory = true;
        }

        //if (game_state.game_over)
        //{
        //    StopMusicStream(background_music);
        //    PlayMusicStream(game_over_music);
        //    UpdateMusicStream(game_over_music);
    
/*
            if (IsKeyPressed(KEY_ENTER))
            {
                StopMusicStream(game_over_music);
                PlayMusicStream(background_music);
                UpdateMusicStream(background_music);
                game_state.game_over = false;
                //game_state.pause =  false;
                //game_state.opener_screen = false;
                game_state.lives_left = 3;
                game_state.enemies_killed = 0;
                game_state.active_enemies = 30;
                game_state.victory = false;
                //game_state.help = false;
                for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
                {
                    enemies[enemy].rectangle.x = GetRandomValue(0, screen_width - 2*enemies[enemy].rectangle.width);
                    enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                    enemies[enemy].active = true;
                }
                powerup.rectangle.x = GetRandomValue(0, screen_width-2*powerup.rectangle.width);
                powerup.rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
                {
                    player_projectiles[projectile].active = false;
                }
            }
*/
            
        //}

        // send updated game_state to client
        sendto(server_socket, &game_state, sizeof(struct GameState), 0, (struct sockaddr*)&client_addr, client_len);

    }
    //De-Initialization
    //UnloadTexture(player.texture);
    //for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    //{
    //    UnloadTexture(player_projectiles[projectile].texture);
    //}
    //UnloadTexture(powerup.texture);
    //for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
    //{
    //    UnloadTexture(enemies[enemy].texture);
    //}
    //UnloadMusicStream(background_music);
    //UnloadSound(killed_enemy_sound);
    //UnloadSound(powerup_boost_sound);
    //UnloadSound(lost_life_sound);
    //CloseAudioDevice();
    CloseWindow();
    return 0;
};
