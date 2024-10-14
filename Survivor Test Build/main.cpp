#include <iostream>
#include "C:\raylib\raylib\src\raylib.h"
#include <cmath>
#include "C:\raylib\raylib\src\raymath.h"

using namespace std;

// Screen size values 
const int screenWidth = 1200;
const int screenHeight = 800;

// Core Variables and Values


    // Game Rules
    int enemiesDefeated = 0; // Tracks the number of enemies defeated
    bool isVictory = false;
    bool isGameOver = false; // Tracks if the game is over
    float survivalTime = 0.0f; // Tracks how long the player has survived
    float survivalGoal = 120.0f; // Player needs to survive for 60 seconds to win


    // Tank Class
        typedef enum TankClass {
        ASSAULT,
        SCOUT,
        SPREAD
    } TankClass;


    // Player Structure
   typedef struct Player {
    Vector2 position;
    float radius;
    float speed;
    float rotation;
    int health;
    int maxHealth;
    Rectangle cannon;
    float fireRate;
    float lastShotTime;
    float healthBarVisibleTime;
    int level;
    int experience;
    int experienceNeeded;
    TankClass tankClass;
    float bulletDamage;      // Damage dealt by bullets
    int cannonCount;         // Number of cannons the player has
    float repulseCooldown;   // Cooldown for repulse ability (Assault class)
    float lastRepulseTime;   // Time since last repulse ability use
    float lastDashTime;      // Time since last dash (Scout class)
    bool isDashing;          // Whether the player is currently dashing (Scout class)
} Player;

    Player player;


    // Player contact damage to enemies
    int playerContactDamage = 5;

    int gridSpacing = 25; // Distance between grid lines

    // Spawn Mechanics

    float spawnRate = 2.0f;  // Enemies spawn every 2 seconds at the start
    float timeSinceLastSpawn = 0.0f;
    float spawnRateDecrement = 0.05f;  // Spawn rate increases slightly over time
    float minSpawnRate = 0.5f;  // Limit the spawn rate increase
    bool classSelected = false; // Indicates if the player has selected a class

    // Menu Elements

    void DrawExperienceBar(Player &player) {
    float experiencePercentage = (float)player.experience / player.experienceNeeded;

    // Define experience bar dimensions
    float barWidth = 300.0f;
    float barHeight = 20.0f;
    float barX = (screenWidth / 2) - (barWidth / 2);
    float barY = screenHeight - 50;

    // Draw the experience bar border
    DrawRectangle(barX - 2, barY - 2, barWidth + 4, barHeight + 4, BLACK);

    // Draw the experience bar fill
    DrawRectangle(barX, barY, barWidth * experiencePercentage, barHeight, SKYBLUE);

    // Draw the current level above the experience bar
    DrawText(TextFormat("Level: %d", player.level), barX + barWidth / 2 - MeasureText(TextFormat("Level: %d", player.level), 20) / 2, barY - 30, 20, DARKBLUE);
}


   void ClassSelectionMenu() {
    DrawText("SELECT YOUR TANK CLASS", screenWidth / 2 - MeasureText("SELECT YOUR TANK CLASS", 40) / 2, 100, 40, DARKGRAY);

    DrawText("1. ASSAULT", screenWidth / 2 - 100, 200, 30, RED);
    DrawText("Higher Fire Rate, Moderate Health", screenWidth / 2 - 100, 240, 20, DARKGRAY);
    if (IsKeyPressed(KEY_ONE)) {
        player.tankClass = ASSAULT;
        player.fireRate = 0.2f;        // High fire rate
        player.speed = 200.0f;         // Moderate speed
        player.maxHealth = 120;        // Moderate health
        player.health = player.maxHealth;
        player.bulletDamage = 5.0f;    // Low bullet damage
        player.cannonCount = 1;        // Starts with 1 cannon
        player.repulseCooldown = 10.0f; // 10-second cooldown for repulse ability
        player.lastRepulseTime = -10.0f; // Initialize to be ready for use
        classSelected = true;
    }

    DrawText("2. SCOUT", screenWidth / 2 - 100, 300, 30, GREEN);
    DrawText("High Damage, Low Fire Rate", screenWidth / 2 - 100, 340, 20, DARKGRAY);
    if (IsKeyPressed(KEY_TWO)) {
        player.tankClass = SCOUT;
        player.fireRate = 0.8f;        // Slow fire rate
        player.speed = 300.0f;         // Fast speed
        player.maxHealth = 80;         // Low health
        player.health = player.maxHealth;
        player.bulletDamage = 20.0f;   // High bullet damage
        player.cannonCount = 1;        // Starts with 1 cannon
        player.lastDashTime = -10.0f;  // Initialize dash ability cooldown
        classSelected = true;
    }

    DrawText("3. SPREAD", screenWidth / 2 - 100, 400, 30, BLUE);
    DrawText("High HP, Multiple Cannons", screenWidth / 2 - 100, 440, 20, DARKGRAY);
    if (IsKeyPressed(KEY_THREE)) {
        player.tankClass = SPREAD;
        player.fireRate = 0.6f;        // Slower fire rate
        player.speed = 150.0f;         // Slow speed
        player.maxHealth = 150;        // Highest health
        player.health = player.maxHealth;
        player.bulletDamage = 8.0f;    // Moderate bullet damage
        player.cannonCount = 2;        // Starts with 2 cannons facing opposite directions
        classSelected = true;
    }
}

    //Enemy structure
    typedef struct Enemy {
    Vector2 position;
    float size;
    float speed;
    int sides;   // Number of sides (square, triangle, pentagon)
    int health;  // Health for the enemy
    int maxHealth;
    bool active;
    float healthBarVisibleTime; // Timer to control when the health bar fades out
    } Enemy;

    //Projectile structure
typedef struct Projectile {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool active;
    int piercesLeft;  // Number of enemies the projectile can pierce through (used for Scout class)
} Projectile;

Enemy enemies[100]; // Array of enemies
Projectile projectiles[50]; // Array of projectiles

    // Experience Pip Structure
typedef struct ExperiencePip {
    Vector2 position;
    bool active;
} ExperiencePip;

/* Player player;
Enemy enemies[100];
Projectile projectiles[50]; */
ExperiencePip experiencePips[50]; // Array of experience pips


// Function Definitions

void CheckCollisions();

void DrawExperiencePips(ExperiencePip experiencePips[]) {
    for (int i = 0; i < 50; i++) {
        if (experiencePips[i].active) {
            DrawCircleV(experiencePips[i].position, 5.0f, MAROON); // Draw pips as dark red circles
        }
    }
}

void InitExperiencePips(ExperiencePip experiencePips[], int count) {
    for (int i = 0; i < count; i++) {
        experiencePips[i].active = false; // All pips start inactive
    }
}


void InitPlayer(Player &player);
void UpdatePlayer(Player &player);
void DrawPlayer(Player &player);

void InitEnemies(Enemy enemies[], int count);
void UpdateEnemies(Enemy enemies[], Player &player);
void DrawEnemies(Enemy enemies[]);
void SpawnEnemy();

void InitProjectiles(Projectile projectiles[], int count);
void UpdateProjectiles(Projectile projectiles[]);
void DrawProjectiles(Projectile projectiles[]);
void FireProjectiles();

int main () {

    
    cout << "Hello World" << endl;

    InitWindow(screenWidth, screenHeight, "Survivor");

    InitPlayer(player);
    InitEnemies(enemies, 10);
    InitProjectiles(projectiles, 50); 


    SetTargetFPS(60);

    while (WindowShouldClose() == false){
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

      if (!classSelected) {
        ClassSelectionMenu();
    } else if (!isGameOver && !isVictory) {
        survivalTime += GetFrameTime();
        if (survivalTime >= survivalGoal) {
            isVictory = true; // Set victory state if survival goal is reached
        }

        // Update health bar timers
        if (player.healthBarVisibleTime > 0) {
            player.healthBarVisibleTime -= GetFrameTime();  // Decrease the visibility timer for player
        }

        for (int i = 0; i < 100; i++) {
            if (enemies[i].active && enemies[i].healthBarVisibleTime > 0) {
                enemies[i].healthBarVisibleTime -= GetFrameTime();  // Decrease the visibility timer for enemies
            }
        }

        // Draw the Grid
        for (int x = 0; x < screenWidth; x += gridSpacing) {
            DrawLine(x, 0, x, screenHeight, DARKGRAY); // Vertical
        }
        for (int y = 0; y < screenHeight; y += gridSpacing) {
            DrawLine(0, y, screenWidth, y, DARKGRAY); // Horizontal
        }

        // Update player, enemies, and projectiles
        UpdatePlayer(player);
        UpdateEnemies(enemies, player);
        UpdateProjectiles(projectiles);

        // Enemy spawning logic
        timeSinceLastSpawn += GetFrameTime();
        if (timeSinceLastSpawn >= spawnRate) {
            SpawnEnemy();
            timeSinceLastSpawn = 0.0f;

            // Gradually increase the spawn rate (enemies spawn faster over time)
            spawnRate -= spawnRateDecrement;
            if (spawnRate < minSpawnRate) {
                spawnRate = minSpawnRate;  // Cap the spawn rate at the minimum value
            }
        }

        CheckCollisions();

        // Check if player health is zero
        if (player.health <= 0) {
            isGameOver = true; // Set game over state if player health reaches 0
        }

        // Draw Everything
        DrawPlayer(player);
        DrawEnemies(enemies);
        DrawProjectiles(projectiles);
        DrawExperiencePips(experiencePips);
        DrawExperienceBar(player); // Draw experience bar at the bottom of the screen

        // Survival timer
        DrawText(TextFormat("Survival Time: %.1f / %.1f", survivalTime, survivalGoal), 20, 20, 20, BLACK);
    } else if (isVictory) {
        // Victory Screen
        DrawText("VICTORY!", screenWidth / 2 - MeasureText("VICTORY!", 50) / 2, screenHeight / 2 - 100, 50, GREEN);
        DrawText(TextFormat("Tank Class Used: %s", 
            (player.tankClass == ASSAULT) ? "Assault" : 
            (player.tankClass == SCOUT) ? "Scout" : "Spread"), 
            screenWidth / 2 - MeasureText("Tank Class Used:", 20) / 2, screenHeight / 2, 20, BLACK);
        DrawText(TextFormat("Enemies Defeated: %d", enemiesDefeated), screenWidth / 2 - MeasureText("Enemies Defeated: ", 20) / 2, screenHeight / 2 + 50, 20, BLACK);
        DrawText("Press R to Restart or ESC to Quit", screenWidth / 2 - MeasureText("Press R to Restart or ESC to Quit", 20) / 2, screenHeight / 2 + 100, 20, BLACK);

        // Restart or Quit Logic
        if (IsKeyPressed(KEY_R)) {
            survivalTime = 0.0f;
            isGameOver = false;
            isVictory = false;
            enemiesDefeated = 0;
            player.health = player.maxHealth;
            player.level = 1;
            player.experience = 0;
            player.experienceNeeded = 5;
            InitEnemies(enemies, 100);
            InitProjectiles(projectiles, 50);
            InitExperiencePips(experiencePips, 50);
            classSelected = false; // Reset class selection
        }
    } else {
        // Game Over Screen
        DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 50) / 2, screenHeight / 2 - 50, 50, RED);
        DrawText("Press R to Restart or ESC to Quit", screenWidth / 2 - MeasureText("Press R to Restart or ESC to Quit", 20) / 2, screenHeight / 2 + 20, 20, BLACK);

        // Restart or Quit Logic
        if (IsKeyPressed(KEY_R)) {
            survivalTime = 0.0f;
            isGameOver = false;
            isVictory = false;
            enemiesDefeated = 0;
            player.health = player.maxHealth;
            player.level = 1;
            player.experience = 0;
            player.experienceNeeded = 5;
            InitEnemies(enemies, 100);
            InitProjectiles(projectiles, 50);
            InitExperiencePips(experiencePips, 50);
            classSelected = false; // Reset class selection
        }
    }

    EndDrawing();
}

    CloseWindow();
    return 0;
}

// Health Bar Function
void DrawHealthBar(Vector2 position, int health, int maxHealth, float size) {
    // Health bar attributes
    float healthBarWidth = size;
    float healthBarHeight = 5.0f;
    float rounding = 0.2f; // Rounding amount for rounded rectangle

    // Calculate health percentage
    float healthPercentage = (float)health / maxHealth;

    // Draw black border
    DrawRectangleRounded((Rectangle){position.x - healthBarWidth / 2, position.y + size / 2 + 5, healthBarWidth, healthBarHeight}, rounding, 4, BLACK);

    // Draw green health bar
    DrawRectangleRounded((Rectangle){position.x - healthBarWidth / 2, position.y + size / 2 + 5, healthBarWidth * healthPercentage, healthBarHeight}, rounding, 4, GREEN);
}

// Player Functions
void InitPlayer(Player &player) {
    player.position = {400, 300};
    player.radius = 20.0f;
    player.speed = 200.0f;
    player.rotation = 0.0f;
    player.health = 100;
    player.maxHealth = 100;
    player.cannon = {player.position.x, player.position.y, 40, 10};
    player.fireRate = 0.3f;
    player.lastShotTime = 0.0f;
    player.healthBarVisibleTime = 0.0f;
    player.level = 1;
    player.experience = 0;
    player.experienceNeeded = 2; // Level 2 requires 5 XP
}

void FireProjectiles(Player &player) {
    static int shotCounter = 0; // Track shots for level 5 ability

    if (player.tankClass == SPREAD) {
        // Fire projectiles from all cannons for Spread class
        for (int j = 0; j < player.cannonCount; j++) {
            for (int i = 0; i < 50; i++) {
                if (!projectiles[i].active) {
                    projectiles[i].active = true;

                    // Set position and velocity for each cannon
                    float angleOffset = (360.0f / player.cannonCount) * j; // Spread evenly
                    float angle = player.rotation + angleOffset;
                    projectiles[i].position = player.position; // Start from player position
                    projectiles[i].velocity = {
                        cos(angle * DEG2RAD) * 400.0f,
                        sin(angle * DEG2RAD) * 400.0f
                    };

                    break; // Move to the next cannon after firing
                }
            }
        }
    } else {
        // Regular projectile logic for other classes (e.g., Assault, Scout)
        for (int i = 0; i < 50; i++) {
            if (!projectiles[i].active) {
                projectiles[i].active = true;
                projectiles[i].piercesLeft = (player.tankClass == SCOUT && player.level >= 3) ? (player.level == 5 ? -1 : 3) : 0; // Piercing for Scout

                // Set projectile properties for Scout class
                if (player.tankClass == SCOUT) {
                    projectiles[i].radius = 10.0f; // Larger projectile for Scout
                    projectiles[i].position = {
                        player.position.x + cos(player.rotation * DEG2RAD) * player.radius,
                        player.position.y + sin(player.rotation * DEG2RAD) * player.radius
                    };
                    projectiles[i].velocity = {
                        cos(player.rotation * DEG2RAD) * 500.0f, // Faster projectile velocity for Scout
                        sin(player.rotation * DEG2RAD) * 500.0f
                    };
                } else {
                    // Regular projectile logic for Assault
                    projectiles[i].position = {
                        player.position.x + cos(player.rotation * DEG2RAD) * player.radius,
                        player.position.y + sin(player.rotation * DEG2RAD) * player.radius
                    };
                    projectiles[i].velocity = {
                        cos(player.rotation * DEG2RAD) * 400.0f,
                        sin(player.rotation * DEG2RAD) * 400.0f
                    };

                    if (player.cannonCount == 3 && (shotCounter % 10 == 0)) {
                        // Special third cannon for Assault level 5 evolution
                        projectiles[i].radius = 10.0f; // Larger projectile
                        projectiles[i].velocity = {
                            cos(player.rotation * DEG2RAD) * 300.0f,
                            sin(player.rotation * DEG2RAD) * 300.0f
                        };
                    }
                }

                shotCounter++;
                break; // Fire only one set of projectiles at a time for non-Spread classes
            }
        }
    }
}

void RepulseEnemies(Vector2 playerPosition, float radius) {
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            if (CheckCollisionCircles(playerPosition, radius, enemies[i].position, enemies[i].size)) {
                Vector2 direction = Vector2Normalize(Vector2Subtract(enemies[i].position, playerPosition));
                enemies[i].position = Vector2Add(enemies[i].position, Vector2Scale(direction, 50.0f)); // Push enemies away
            }
        }
    }
}


void UpdatePlayer(Player &player) {
    // Movement
    if (IsKeyDown(KEY_W)) player.position.y -= player.speed * GetFrameTime();
    if (IsKeyDown(KEY_S)) player.position.y += player.speed * GetFrameTime();
    if (IsKeyDown(KEY_A)) player.position.x -= player.speed * GetFrameTime();
    if (IsKeyDown(KEY_D)) player.position.x += player.speed * GetFrameTime();

    // Collect experience pips
    for (int i = 0; i < 50; i++) {
        if (experiencePips[i].active && CheckCollisionCircles(player.position, player.radius, experiencePips[i].position, 5.0f)) {
            experiencePips[i].active = false;
            player.experience += 1;

            // Check if player should level up
            if (player.experience >= player.experienceNeeded) {
                player.level++;
                player.experience = 0; // Reset experience after leveling up
                player.experienceNeeded += 5; // Increase XP needed for next level

                // Show player health bar to indicate level-up (optional feedback)
                player.healthBarVisibleTime = 2.0f;
            }
        }
    }

    // Cannon movement
    Vector2 mousePosition = GetMousePosition();
    player.rotation = atan2f(mousePosition.y - player.position.y, mousePosition.x - player.position.x) * (180 / PI); // Convert to degrees

    // Update cannon position and rotation
    player.cannon.x = player.position.x + cos(player.rotation * DEG2RAD) * player.radius;
    player.cannon.y = player.position.y + sin(player.rotation * DEG2RAD) * player.radius;

    // Level Up Mechanics
    if (player.level == 3 && player.cannonCount == 1) {
        player.cannonCount = 2; // Add a second cannon at level 3
    }
    if (player.level == 5 && player.cannonCount == 2) {
        player.cannonCount = 3; // Add a third cannon at level 5
    }

    // Dash Ability for Scout (activated by pressing space bar)
    if (player.tankClass == SCOUT && player.level >= 3 && IsKeyPressed(KEY_SPACE)) {
        if (GetTime() - player.lastDashTime >= 10.0f) { // 10-second cooldown
            player.isDashing = true; // Start dashing
            player.lastDashTime = GetTime();
        }

    if (player.tankClass == SPREAD) {
        if (player.level == 3 && player.cannonCount == 2) {
            player.cannonCount = 4; // Add 2 more cannons at level 3
        }
        if (player.level == 5 && player.cannonCount == 4) {
            player.cannonCount = 8; // Add 4 more cannons at level 5
            player.speed *= 0.5f; // Reduce speed by half
        }
    }


    // Repulse Ability for Assault (activated by pressing space bar)
    if (player.tankClass == ASSAULT && player.level >= 3 && IsKeyPressed(KEY_SPACE)) {
        if (GetTime() - player.lastRepulseTime >= player.repulseCooldown) {
            RepulseEnemies(player.position, 100.0f); // Repulse enemies within 100 units
            player.lastRepulseTime = GetTime();
        }
    }

    }

    // Handle dash effect for Scout
    if (player.isDashing) {
        player.speed = 500.0f; // Increase speed temporarily
        DrawCircleV(player.position, player.radius + 5.0f, Fade(SKYBLUE, 0.3f)); // Draw translucent blue trail
        if (GetTime() - player.lastDashTime >= 1.0f) { // Dash lasts 1 second
            player.isDashing = false; // End dash
            player.speed = 200.0f; // Reset speed
        }
    }

    // Shooting logic 
    player.lastShotTime += GetFrameTime();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && player.lastShotTime >= player.fireRate) {
        FireProjectiles(player);
        player.lastShotTime = 0.0f;
    }
}

void DrawPlayer(Player &player) {
    // Draw the main body of the player as a circle
    DrawCircleV(player.position, player.radius, BLUE);

    // Draw the cannons for Spread class or others
    if (player.tankClass == SPREAD) {
        for (int i = 0; i < player.cannonCount; i++) {
            float angleOffset = (360.0f / player.cannonCount) * i;
            Vector2 cannonPos = {
                player.position.x + cos((player.rotation + angleOffset) * DEG2RAD) * player.radius,
                player.position.y + sin((player.rotation + angleOffset) * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){cannonPos.x, cannonPos.y, 40, 10},
                (Vector2){0, 5},
                player.rotation + angleOffset,
                DARKBLUE
            );
        }
    } else {
        // Draw the cannons for other classes (Assault, Scout)
        if (player.cannonCount >= 1) {
            // First Cannon (Main Cannon)
            Vector2 firstCannonPos = {
                player.position.x + cos(player.rotation * DEG2RAD) * player.radius,
                player.position.y + sin(player.rotation * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){firstCannonPos.x, firstCannonPos.y, 40, 10},
                (Vector2){0, 5},
                player.rotation,
                DARKBLUE
            );
        }

        if (player.cannonCount >= 2) {
            // Second Cannon (Offset Left)
            Vector2 secondCannonPos = {
                player.position.x + cos((player.rotation + 30) * DEG2RAD) * player.radius,
                player.position.y + sin((player.rotation + 30) * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){secondCannonPos.x, secondCannonPos.y, 40, 10},
                (Vector2){0, 5},
                player.rotation,
                DARKBLUE
            );
        }

        if (player.cannonCount >= 3) {
            // Third Cannon (Offset Right)
            Vector2 thirdCannonPos = {
                player.position.x + cos((player.rotation - 30) * DEG2RAD) * player.radius,
                player.position.y + sin((player.rotation - 30) * DEG2RAD) * player.radius
            };
            DrawRectanglePro(
                (Rectangle){thirdCannonPos.x, thirdCannonPos.y, 40, 10},
                (Vector2){0, 5},
                player.rotation,
                DARKBLUE
            );
        }
    }

    // Draw the player's health bar if recently damaged
    if (player.healthBarVisibleTime > 0) {
        DrawHealthBar(player.position, player.health, player.maxHealth, 40.0f);
    }
}

void InitEnemies(Enemy enemies[], int count) {
    for (int i = 0; i < count; i++) {
        enemies[i].position = {(float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight)};
        enemies[i].sides = GetRandomValue(3, 5); // Random shape: 3 = triangle, 4 = square, 5 = pentagon

        // Set size and speed based on the enemy type
        if (enemies[i].sides == 3) {            // Triangles: smaller and faster
            enemies[i].size = 15.0f;
            enemies[i].speed = 150.0f;
        } else if (enemies[i].sides == 4) {     // Squares: medium size and speed
            enemies[i].size = 35.0f;
            enemies[i].speed = 100.0f;
        } else if (enemies[i].sides == 5) {     // Pentagons: larger and slower
            enemies[i].size = 45.0f;
            enemies[i].speed = 75.0f;
        }

        enemies[i].active = false;
    }
}

void UpdateEnemies(Enemy enemies[], Player &player) {
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            Vector2 direction = Vector2Normalize(Vector2Subtract(player.position, enemies[i].position));
            enemies[i].position.x += direction.x * enemies[i].speed * GetFrameTime();
            enemies[i].position.y += direction.y * enemies[i].speed * GetFrameTime();
        }
    }
}


void SpawnEnemy() {
    for (int i = 0; i < 100; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;

            // Randomly select which edge to spawn the enemy from
            int edge = GetRandomValue(0, 3);
            if (edge == 0) {  // Top edge
                enemies[i].position = {(float)GetRandomValue(0, screenWidth), 0.0f};
            } else if (edge == 1) {  // Bottom edge
                enemies[i].position = {(float)GetRandomValue(0, screenWidth), (float)screenHeight};
            } else if (edge == 2) {  // Left edge
                enemies[i].position = {0.0f, (float)GetRandomValue(0, screenHeight)};
            } else if (edge == 3) {  // Right edge
                enemies[i].position = {(float)screenWidth, (float)GetRandomValue(0, screenHeight)};
            }

            // Set the enemy size, speed, and health based on its type
            enemies[i].sides = GetRandomValue(3, 5); // Random shape: 3 = triangle, 4 = square, 5 = pentagon
            if (enemies[i].sides == 3) {            // Triangles: smaller and faster
                enemies[i].size = 20.0f;
                enemies[i].speed = 150.0f;
                enemies[i].health = 5;
                enemies[i].maxHealth = 5;
            } else if (enemies[i].sides == 4) {     // Squares: medium size and speed
                enemies[i].size = 30.0f;
                enemies[i].speed = 100.0f;
                enemies[i].health = 10;
                enemies[i].maxHealth = 10;
            } else if (enemies[i].sides == 5) {     // Pentagons: larger and slower
                enemies[i].size = 40.0f;
                enemies[i].speed = 75.0f;
                enemies[i].health = 20;
                enemies[i].maxHealth = 20;
            }

            enemies[i].healthBarVisibleTime = 0.0f; // Start with health bar invisible
            break;
        }
    }
}

void DrawEnemies(Enemy enemies[]) {
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active) {
            Color enemyColor = (enemies[i].sides == 3) ? RED : (enemies[i].sides == 4) ? YELLOW : DARKBLUE;

            // Draw the enemy shapes
            if (enemies[i].sides == 3) {
                DrawTriangle((Vector2){enemies[i].position.x, enemies[i].position.y - enemies[i].size},
                             (Vector2){enemies[i].position.x - enemies[i].size, enemies[i].position.y + enemies[i].size},
                             (Vector2){enemies[i].position.x + enemies[i].size, enemies[i].position.y + enemies[i].size}, enemyColor);
            } else if (enemies[i].sides == 4) {
                DrawRectangleV((Vector2){enemies[i].position.x - enemies[i].size / 2, enemies[i].position.y - enemies[i].size / 2}, 
                               (Vector2){enemies[i].size, enemies[i].size}, enemyColor);
            } else if (enemies[i].sides == 5) {
                DrawPoly(enemies[i].position, 5, enemies[i].size, 0.0f, enemyColor);
            }

            // Draw the enemy's health bar if recently damaged
            if (enemies[i].healthBarVisibleTime > 0) {
                DrawHealthBar(enemies[i].position, enemies[i].health, enemies[i].maxHealth, enemies[i].size);
            }
        }
    }
}

// Projectile functions
void InitProjectiles(Projectile projectiles[], int count) {
    for (int i = 0; i < count; i++) {
        projectiles[i].active = false;
        projectiles[i].radius = 10.0f; // Projectiles are small circles
    }
}

void UpdateProjectiles(Projectile projectiles[]) {
    for (int i = 0; i < 50; i++) {
        if (projectiles[i].active) {
            projectiles[i].position.x += projectiles[i].velocity.x * GetFrameTime();
            projectiles[i].position.y += projectiles[i].velocity.y * GetFrameTime();

             // Deactivate the projectile if it goes off screen
            if (projectiles[i].position.x < 0 || projectiles[i].position.x > screenWidth ||
                projectiles[i].position.y < 0 || projectiles[i].position.y > screenHeight) {
                projectiles[i].active = false;
            }
        }
    }
}

void DrawProjectiles(Projectile projectiles[]) {
    for (int i = 0; i < 50; i++) {
        if (projectiles[i].active) {
            DrawCircleV(projectiles[i].position, projectiles[i].radius, SKYBLUE); // Projectiles are black circles
        }
    }
}

// Collision checking
void CheckCollisions() {
    // Enemy-Enemy Collision
    for (int i = 0; i < 100; i++) {
        if (!enemies[i].active) continue;

        for (int j = i + 1; j < 100; j++) {
            if (!enemies[j].active) continue;

            if (CheckCollisionCircles(enemies[i].position, enemies[i].size, enemies[j].position, enemies[j].size)) {
                // Calculate direction vector between the two enemies
                Vector2 direction = Vector2Normalize(Vector2Subtract(enemies[j].position, enemies[i].position));

                // Move each enemy slightly apart to prevent overlap
                enemies[i].position = Vector2Subtract(enemies[i].position, Vector2Scale(direction, 2.0f));
                enemies[j].position = Vector2Add(enemies[j].position, Vector2Scale(direction, 2.0f));
            }
        }
    }

    // Enemy-Player Collision
    for (int i = 0; i < 100; i++) {
        if (enemies[i].active && CheckCollisionCircleRec(enemies[i].position, enemies[i].size, (Rectangle){player.position.x - player.radius, player.position.y - player.radius, player.radius * 2, player.radius * 2})) {
            // Handle player taking damage
            player.health -= 10; // Reduce player health by 10

            // Show player health bar when damaged
            player.healthBarVisibleTime = 2.0f;

            // Ensure player health doesn't go below 0
            if (player.health < 0) {
                player.health = 0;
            }

            // Handle enemy taking damage from player contact
            enemies[i].health -= playerContactDamage;

            // Show enemy health bar when hit
            enemies[i].healthBarVisibleTime = 2.0f;

            // Deactivate the enemy if health reaches 0
            if (enemies[i].health <= 0) {
                enemies[i].active = false;
                enemiesDefeated++;
            }
        }
    }

    // Projectile-Enemy Collision
    for (int i = 0; i < 100; i++) {
        if (!enemies[i].active) continue;

        for (int j = 0; j < 50; j++) {
            if (!projectiles[j].active) continue;

            if (CheckCollisionCircles(projectiles[j].position, projectiles[j].radius, enemies[i].position, enemies[i].size)) {
                enemies[i].health -= player.bulletDamage; // Use player's bullet damage
                projectiles[j].piercesLeft--;

                enemies[i].healthBarVisibleTime = 2.0f;

                if (enemies[i].health <= 0) {
                    enemies[i].active = false;
                    enemiesDefeated++; // Increment enemies defeated count

                    // Drop an experience pip
                    for (int k = 0; k < 50; k++) {
                        if (!experiencePips[k].active) {
                            experiencePips[k].active = true;
                            experiencePips[k].position = enemies[i].position;
                            break;
                        }
                    }
                }

                // Deactivate projectile if no pierces left
                if (projectiles[j].piercesLeft <= 0) {
                    projectiles[j].active = false;
                }
                break;
            }
        }
    }
}