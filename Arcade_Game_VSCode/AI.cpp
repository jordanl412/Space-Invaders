#include "main_header.h"

typedef enum HighLevelAIState {
    ProtectMode,
    FocusRight,
    FocusLeft,
} HighLevelState;

typedef enum LowLevelAIState {
    CalculateTargetLocation,
    CalculateNearestEnemyLocation,
    MoveToTargetLocation,
    MoveToEnemyLocation,
    Shoot
} LowLevelState;

typedef struct AI_Blackboard {
    Entity& ai_player;
    Entity& player;
    Entity enemies[num_enemies];
    LowLevelAIState low_level_state;

    AI_Blackboard(Entity& aiPlayer, Entity& realPlayer, Entity enemies[], LowLevelAIState low_level_state)
        : ai_player(aiPlayer), player(realPlayer), enemies(enemies), low_level_state(low_level_state) {}
} AI_Blackboard;

void UpdateAIPlayer(AIPlayer& aiPlayer, Entity& mainPlayer, Entity enemies[], int num_enemies) {

}