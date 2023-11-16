
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
    //Music music;
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


    InitAudioDevice();
    Music opener_music = LoadMusicStream("res/opener_screen_music.mp3");
    Music background_music = LoadMusicStream("res/space_invaders_background.mp3");
    SetMusicVolume(background_music, 0.25);
    Sound killed_enemy_sound = LoadSound("res/enemy_killed.mp3");
    Sound powerup_boost_sound = LoadSound("res/boost_sound.mp3");
    Sound lost_life_sound = LoadSound("res/lost_life.mp3");
    Music game_over_music = LoadMusicStream("res/game_over1.mp3");
    //PlayMusicStream(background_music);
    float timePlayed = 0.0f;


    Texture normal_player_texture = LoadTexture("res/player_icon.png");
    Texture powerup_player_texture = LoadTexture("res/powerup_player_icon.png");
    Texture enemy_texture = LoadTexture("res/enemy_icon.png");
    Texture projectile_texture = LoadTexture("res/player_projectile.png");
    Texture powerup_texture = LoadTexture("res/powerup_icon.png");

    ClientPacket received_client_packet;

    LightGameState server_packet_to_send;
    server_packet_to_send.game_over = false;
    server_packet_to_send.pause_button = false;
    server_packet_to_send.lives_left = 3;
    server_packet_to_send.enemies_killed = 0;
    server_packet_to_send.active_enemies = 30;
    server_packet_to_send.victory = false;
    server_packet_to_send.opener_screen = true;
    server_packet_to_send.help = false;
    server_packet_to_send.active_projectiles = 0;


    Entity player_one;
    player_one.type = TYPE_PLAYER;
    player_one.texture = normal_player_texture;
    player_one.rectangle.width = 40;
    player_one.rectangle.height = 40;
    player_one.rectangle.x = (float)screen_width/2 - 20;
    player_one.rectangle.y = (float)screen_height - 40;
    player_one.speed = 1.0f;

    server_packet_to_send.player_one = player_one;

    Entity player_two;
    player_two.type = TYPE_PLAYER;
    player_two.texture = normal_player_texture;
    player_two.rectangle.width = 40;
    player_two.rectangle.height = 40;
    player_two.rectangle.x = (float)screen_width/2 - 20;
    player_two.rectangle.y = (float)screen_height - 40;
    player_two.speed = 1.0f;    

    server_packet_to_send.player_two = player_two;

    Entity players[2];


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

    LightProjectile projectiles_to_send[NUM_SHOOTS];
    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
    {
        projectiles_to_send[projectile].active = false;
    }

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

    Timer projectile_shot_timer = {0};

    int frame_counter = 0;

    // SERVER CODE
    // Create struct for server's address
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(2112),
        .sin_addr.s_addr = INADDR_ANY
    };

    // create socket
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    int flags = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

    // check if socket creation fails
    if (server_socket < 0) {
        perror("Failed to create server socket");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Failed to set SO_REUSEADDR");
        return -1;
    }

    // bind socket to specied address and port
    if (bind(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Failed to bind to socket");
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    SetTargetFPS(120);
    while (!WindowShouldClose())
    {
    
        int bytes_read = recvfrom(server_socket, &received_client_packet, sizeof(received_client_packet), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytes_read > 0) {
            right_key_down = received_client_packet.right_key_down;
            left_key_down = received_client_packet.left_key_down;
            space_key_down = received_client_packet.space_key_down;
        }

    // INPUT
        if (opener_screen)
        {
            //PlayMusicStream(opener_music);
            //UpdateMusicStream(opener_music);
            if (IsKeyPressed(KEY_ENTER))
            {
                //StopMusicStream(opener_music);
                //PlayMusicStream(background_music);
                //UpdateMusicStream(background_music);
                game_over = false;
                pause_button = false;
                opener_screen = false;
                lives_left = 3;
                enemies_killed = 0;
                active_enemies = 30;
                victory = false;
                help = false;
            }
        }

        if (!pause_button && !game_over && !opener_screen)
        {
            //Stream(background_music);

            // Player movements left and right
            if (IsKeyDown(KEY_RIGHT))
            {
                if (player_one.rectangle.x < (screen_width - player_one.rectangle.width))
                {
                    player_one.rectangle.x += player_one.speed;
                }
            }
            if (IsKeyDown(KEY_LEFT))
            {
                if (player_one.rectangle.x > 0)
                {
                    player_one.rectangle.x -= player_one.speed;
                }
            }

            if (right_key_down) 
            {
                if (player_two.rectangle.x < (screen_width - player_two.rectangle.width))
                {
                    player_two.rectangle.x += player_two.speed;
                }
            }
            if (left_key_down)
            {
                if (player_two.rectangle.x > 0)
                {
                    player_two.rectangle.x -= player_two.speed;
                }
            }

            // activating player_projectile, shooting with spacebar
            if (IsKeyPressed(KEY_SPACE)) 
            {
                for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
                {
                    if (!player_projectiles[projectile].active)
                    {
                        player_projectiles[projectile].active = true; 
                        active_projectiles += 1;
                        player_projectiles[projectile].rectangle.x = player_one.rectangle.x + player_one.rectangle.width/2 - player_projectiles[projectile].rectangle.width/2;
                        player_projectiles[projectile].rectangle.y = player_one.rectangle.y;
                        break;
                    }
                }
            }

            if (space_key_down)
            {
                if (TimerDone(&projectile_shot_timer)) 
                {
                    for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
                    {
                        if (!player_projectiles[projectile].active)
                        {
                            player_projectiles[projectile].active = true; 
                            active_projectiles += 1;
                            player_projectiles[projectile].rectangle.x = player_two.rectangle.x + player_two.rectangle.width/2 - player_projectiles[projectile].rectangle.width/2;
                            player_projectiles[projectile].rectangle.y = player_two.rectangle.y;
                            break;
                        }
                    }
                    StartTimer(&projectile_shot_timer, 0.5f);
                }
            }

        UpdateTimer(&projectile_shot_timer);


        if (game_over)
        {
            //StopMusicStream(background_music);
            //PlayMusicStream(game_over_music);
            //UpdateMusicStream(game_over_music);

            if (IsKeyPressed(KEY_ENTER))
            {
                //StopMusicStream(game_over_music);
                //PlayMusicStream(background_music);
                //UpdateMusicStream(background_music);
                game_over = false;
                pause_button = false;
                opener_screen = false;
                lives_left = 3;
                enemies_killed = 0;
                active_enemies = 30;
                victory = false;
                help = false;
                active_projectiles = 0;

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
        }
    
    // GAME LOGIC

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
                        active_projectiles -= 1;
                    }
                    // check for collision with enemy
                    for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
                    {
                        if (CheckCollisionRecs(player_projectiles[projectile].rectangle, enemies[enemy].rectangle))
                        {
                            PlaySound(killed_enemy_sound);
                            // remove the projectile that hit the enemy
                            player_projectiles[projectile].active = false;
                            active_projectiles -= 1;
                            // remove the enemy that was hit
                            enemies[enemy].rectangle.x = GetRandomValue(0, screen_width - 2*enemies[enemy].rectangle.width);
                            enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                            enemies[enemy].active = false;
                            enemies_killed += 1;
                            active_enemies -= 1;
                        }
                    }
                    // check for collision with powerup
                     if (CheckCollisionRecs(player_projectiles[projectile].rectangle, powerup.rectangle))
                    {
                        PlaySound(powerup_boost_sound);
                        // remove the projectile that hit the powerup
                        player_projectiles[projectile].active = false;
                        active_projectiles -= 1;
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
                player_one.speed = 3.0f;
                player_two.speed = 3.0f;
                player_one_powered_up = true;
                player_two_powered_up = true;
            }

            if (TimerDone(&powerupTimer))
            {
                player_one.speed = 1.0f;
                player_two.speed = 1.0f;
                player_one_powered_up = false;
                player_two_powered_up = false;
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
                    if (CheckCollisionRecs(enemies[enemy].rectangle, player_one.rectangle))
                    {
                        PlaySound(lost_life_sound);
                        lives_left -= 1;
                        // remove and reset the enemy that hit the player
                        enemies[enemy].rectangle.x = GetRandomValue(0, screen_width-2*enemies[enemy].rectangle.width);
                        enemies[enemy].rectangle.y = GetRandomValue(-screen_height*1.5, -10);
                    }
                    if (CheckCollisionRecs(enemies[enemy].rectangle, player_two.rectangle))
                    {
                        PlaySound(lost_life_sound);
                        lives_left -= 1;
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

        if (lives_left <= 0)
            {
                game_over = true;
            }

        if (active_enemies == 0)
            {
                victory = true;
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

        server_packet_to_send.game_over = game_over;
        server_packet_to_send.pause_button =  pause_button;
        server_packet_to_send.opener_screen = opener_screen;
        server_packet_to_send.lives_left = lives_left;
        server_packet_to_send.enemies_killed = enemies_killed;
        server_packet_to_send.active_enemies = active_enemies;
        server_packet_to_send.victory = victory;
        server_packet_to_send.help = help;
        server_packet_to_send.active_projectiles = active_projectiles;
        server_packet_to_send.player_one.rectangle.x = player_one.rectangle.x;
        server_packet_to_send.player_one.rectangle.y = player_one.rectangle.y;
        server_packet_to_send.player_two.rectangle.x = player_two.rectangle.x;
        server_packet_to_send.player_two.rectangle.y = player_two.rectangle.y;
        server_packet_to_send.player_one_powered_up = player_one_powered_up;
        server_packet_to_send.player_two_powered_up = player_two_powered_up;
        server_packet_to_send.player_one.speed = player_one.speed;
        server_packet_to_send.player_two.speed = player_two.speed;
        server_packet_to_send.powerup.active = powerup.active;
        server_packet_to_send.powerup.rectangle.y = powerup.rectangle.y;
        server_packet_to_send.powerup.rectangle.x = powerup.rectangle.x;
        server_packet_to_send.powerup = powerup;
        server_packet_to_send.player_one = player_one;
        server_packet_to_send.player_two = player_two;


        for (int projectile = 0; projectile < NUM_SHOOTS; projectile++)
        {
            if (player_projectiles[projectile].active == true)
            {
                server_packet_to_send.projectiles_to_send[projectile].rectangle.x = player_projectiles[projectile].rectangle.x;
                server_packet_to_send.projectiles_to_send[projectile].rectangle.y = player_projectiles[projectile].rectangle.y;
                server_packet_to_send.projectiles_to_send[projectile].rectangle.width = player_projectiles[projectile].rectangle.width;
                server_packet_to_send.projectiles_to_send[projectile].rectangle.height = player_projectiles[projectile].rectangle.height;
                server_packet_to_send.projectiles_to_send[projectile].active = player_projectiles[projectile].active;
            } else {
                server_packet_to_send.projectiles_to_send[projectile].active = false;
            }
        }
        
        for (int enemy = 0; enemy < NUM_ENEMIES; enemy++)
        {
            if (enemies[enemy].active == true)
            {
                server_packet_to_send.enemies_to_send[enemy].rectangle.x = enemies[enemy].rectangle.x;
                server_packet_to_send.enemies_to_send[enemy].rectangle.y = enemies[enemy].rectangle.y;
                server_packet_to_send.enemies_to_send[enemy].active = enemies[enemy].active;
            } else {
                server_packet_to_send.enemies_to_send[enemy].active = false;
            }
        }

        if (frame_counter % 3 == 0) {
            sendto(server_socket, &server_packet_to_send, sizeof(server_packet_to_send), 0, (struct sockaddr*)&client_addr, client_addr_len);
        }
        frame_counter += 1;
    }
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
    UnloadSound(killed_enemy_sound);
    UnloadSound(powerup_boost_sound);
    UnloadSound(lost_life_sound);
    CloseAudioDevice();
    CloseWindow();
    close(server_socket);
    return 0;
};
