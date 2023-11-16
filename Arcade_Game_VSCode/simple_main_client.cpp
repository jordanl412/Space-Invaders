#include <iostream>
#include "raylib.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

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
/*
    // CLIENT IMPLEMENTATION
    int client_socket;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    //struct GameState received_game_state;
    struct ClientPacket client_packet;
    char buffer[sizeof(struct GameState)];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2112);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    //server_addr.sin_addr.s_addr = inet_addr( "127.0.0.1");

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Failed to create client socket");
        return -1;
    }

    // END OF CLIENT IMPLEMENTATION
*/
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

    GameState received_game_state = {0};

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
    {/*
        // receive game_state packet from server
        int num_bytes = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &server_len);
        if (num_bytes == sizeof(struct GameState)) {
            memcpy(&received_game_state, buffer, sizeof(struct GameState));
*/
/*
            received_game_state = {
                received_game_state.game_over = received_game_state.game_over;
                received_game_state.lives_left = received_game_state.lives_left;
                received_game_state.enemies_killed = received_game_state.enemies_killed;
                received_game_state.active_enemies = received_game_state.active_enemies;
                received_game_state.victory = received_game_state.victory;
            }
*/
        //}
/*
        // update if NOT game_over
        if (!received_game_state.game_over)
        {
            UpdateMusicStream(background_music);
            timePlayed = GetMusicTimePlayed(background_music)/GetMusicTimeLength(background_music);
            if (timePlayed > 1.0f) timePlayed = 1.0f; // Time played can't be longer than the length of the music

            // check if player moved
            if (IsKeyDown(KEY_RIGHT))
            {
                client_packet.right_key_down = true;
            }
            if (IsKeyDown(KEY_LEFT))
            {
                client_packet.left_key_down = true;
            }

            // activating player_projectile, shooting with spacebar
            if (IsKeyPressed(KEY_SPACE)) 
            {
                client_packet.player_fired = true;
            }

            // active player_projectile logic
            for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
            {
                // check for projectile collision with enemy
                for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
                {
                    if (CheckCollisionRecs(player_projectiles[projectile].rectangle, enemies[enemy].rectangle))
                {
                    PlaySound(killed_enemy_sound);
                    // remove the projectile that hit the enemy
                    player_projectiles[projectile].active = false;
                    // remove the enemy that was hit
                    enemies[enemy].active = false;
                    client_packet.enemy_killed = true;
                }
                }
                // check for collision with powerup
                    if (CheckCollisionRecs(player_projectiles[projectile].rectangle, powerup.rectangle))
                {
                    PlaySound(powerup_boost_sound);
                    // remove the projectile that hit the powerup
                    player_projectiles[projectile].active = false;
                    // remove the powerup
                    powerup.active = false;
                    client_packet.player_powered_up = true;
                }
            }
        
            // enemies logic
            for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
            {
                if (CheckCollisionRecs(enemies[enemy].rectangle, player.rectangle))
                {
                    PlaySound(lost_life_sound);
                    client_packet.player_lost_life = true;
                }
            }
        }

        if (received_game_state.game_over)
        {
            StopMusicStream(background_music);
            PlayMusicStream(game_over_music);
            UpdateMusicStream(game_over_music);

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

        BeginDrawing();
            ClearBackground(BLACK);
            DrawText(TextFormat("Enemies Killed: 0/30"), 10, 10, 35, WHITE);
            DrawText(TextFormat("Lives: 3"), 10, 10, 35, WHITE);

            //DrawText(TextFormat("Enemies Killed: %i/30", received_game_state.enemies_killed), 10, 10, 35, WHITE);
            //DrawText(TextFormat("Lives: %i", received_game_state.lives_left), screen_width*.86, 10, 35, WHITE);
/*
            //if (!received_game_state.game_over && !received_game_state.victory && !received_game_state.opener_screen)
            if (!received_game_state.game_over && !received_game_state.victory)
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
            Rectangle player_sourceRec{0.0f, 0.0f, (float)player.texture.width, (float)player.texture.height};
            DrawTexturePro(player.texture, player_sourceRec, player.rectangle, {0,0}, 0.0f, WHITE);
            if (received_game_state.victory)
            {
                DrawText("Level Complete!", (screen_width/2)-180, (screen_height/2)-40, 50, GREEN);
            } 
            if (received_game_state.game_over)
            {
                DrawText("GAME OVER", (screen_width/2)-140, (screen_height/2)-40, 50, RED);
                //DrawText("Hit Enter to play again", (screen_width/2)-180, (screen_height/2)+50, 35, WHITE);
            }
        EndDrawing();
*/
/*
        // send client_packet packet to server
        sendto(client_socket, &client_packet, sizeof(struct ClientPacket), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
*/
    }
/*    
    //De-Initialization
    UnloadTexture(player.texture);
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    {
        UnloadTexture(player_projectiles[projectile].texture);
    }
    UnloadTexture(powerup.texture);
    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
    {
        UnloadTexture(enemies[enemy].texture);
    }
    UnloadMusicStream(background_music);
    UnloadSound(killed_enemy_sound);
    UnloadSound(powerup_boost_sound);
    UnloadSound(lost_life_sound);
    CloseAudioDevice();
    */
    CloseWindow();
    return 0;
};
