
#include <iostream>
#include "raylib.h"

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


// enum for game state
static bool game_over = false;
static bool pause =  false;
static int lives_left = 3;
static int enemies_killed = 0;
static int active_enemies = 30;
static bool victory = false;
static bool opener_screen = true;
static bool help = false;

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
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
        }


        if (opener_screen)
        {
            PlayMusicStream(opener_music);
            UpdateMusicStream(opener_music);
            if (IsKeyPressed(KEY_ENTER))
            {
                StopMusicStream(opener_music);
                PlayMusicStream(background_music);
                UpdateMusicStream(background_music);
                game_over = false;
                pause =  false;
                opener_screen = false;
                lives_left = 3;
                enemies_killed = 0;
                active_enemies = 30;
                victory = false;
                help = false;
            }
            if (IsKeyPressed(KEY_H))
            {
                help = !help;
                opener_screen = !opener_screen;
            }
        }

        // update positions if NOT PAUSED and NOT GAME OVER
        if (!pause && !game_over && !opener_screen)
        {
            UpdateMusicStream(background_music);
            timePlayed = GetMusicTimePlayed(background_music)/GetMusicTimeLength(background_music);
            if (timePlayed > 1.0f) timePlayed = 1.0f; // Time played can't be longer than the length of the music

            // Player movements left and right
            if (IsKeyDown(KEY_RIGHT))
            {
                if (player.rectangle.x < (screen_width - player.rectangle.width))
                {
                    player.rectangle.x += player.speed;
                }
            }
            if (IsKeyDown(KEY_LEFT))
            {
                if (player.rectangle.x > 0)
                {
                    player.rectangle.x -= player.speed;
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
                        if (CheckCollisionRecs(player_projectiles[projectile].rectangle, enemies[enemy].rectangle))
                        {
                            PlaySound(killed_enemy_sound);
                            // remove the projectile that hit the enemy
                            player_projectiles[projectile].active = false;
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
                    if (CheckCollisionRecs(enemies[enemy].rectangle, player.rectangle))
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

        if (game_over)
        {
            StopMusicStream(background_music);
            PlayMusicStream(game_over_music);
            UpdateMusicStream(game_over_music);

            if (IsKeyPressed(KEY_ENTER))
            {
                StopMusicStream(game_over_music);
                PlayMusicStream(background_music);
                UpdateMusicStream(background_music);
                game_over = false;
                pause =  false;
                opener_screen = false;
                lives_left = 3;
                enemies_killed = 0;
                active_enemies = 30;
                victory = false;
                help = false;
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
                Rectangle player_sourceRec{0.0f, 0.0f, (float)player.texture.width, (float)player.texture.height};
                DrawTexturePro(player.texture, player_sourceRec, player.rectangle, {0,0}, 0.0f, WHITE);
            }
            if (pause)
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
            /*
            if (help)
            {
                int rules_width = MeasureText("RULES", 50);
                DrawText("RULES", (screen_width/2)-140, (screen_height/2)-40, 50, SKYBLUE);
                int win_width = MeasureText("- To win level: defeat number enemies in top left corner", 30);
                DrawText("- To win level: defeat number enemies in top left corner", (screen_width/2)-140, (screen_height/2)-40, 30, WHITE);
                int move_width = MeasureText("- To move: press the left and right arrow keys", 30)
                DrawText("- To move: press the left and right arrow keys", (screen_width/2)-140, (screen_height/2)-40, 50, WHITE);
                DrawText("- To shoot: hit the spacebar", (screen_width/2)-140, (screen_height/2)-40, 50, WHITE);
                DrawText("- Lives: if you lose all 3 lives, game over", (screen_width/2)-140, (screen_height/2)-40, 50, WHITE);
                DrawText("- Pink Powerup: Boost player speed for 4 seconds", (screen_width/2)-140, (screen_height/2)-40, 50, WHITE);
                DrawText("- To pause: hit P", (screen_width/2)-140, (screen_height/2)-40, 50, WHITE);
                DrawText("- For help: hit H", (screen_width/2)-140, (screen_height/2)-40, 50, WHITE);
                DrawText("Hit H again to go back", (screen_width/2)-140, (screen_height/2)-40, 50, YELLOW);
    
            }
            */
        EndDrawing();

    }
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
    CloseWindow();
    return 0;
};

/*
    -- some enemies are coming in partly offscreen (right), but all rectangle.x values shouldn't allow that
*/