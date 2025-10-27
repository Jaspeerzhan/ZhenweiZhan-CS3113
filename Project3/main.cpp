#include "CS3113/Entity.h"
#include <map>
#include <vector>

/**
* Author: Zhenwei Zhan
* Assignment: Lunar Lander
* Date due: 2025-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// Global Constants
constexpr int SCREEN_WIDTH  = 1500,
              SCREEN_HEIGHT = 1000,
              FPS           = 120;


constexpr char BG_COLOUR[]    = "#98FB98";  // Light green background https://cssgradient.io/shades-of-green/
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float ACCELERATION_OF_GRAVITY = 20.0f,
                HORIZONTAL_ACCELERATION = 100.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;


Entity *bird1 = nullptr;
Entity *house = nullptr;
Entity *grass1 = nullptr;
Entity *grass2 = nullptr;

bool gGameWon = false;
bool gGameLost = false;

float gFuel = 850; 

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bird Lander");
    
    // Bird with sprite sheet
    std::map<Direction, std::vector<int>> birdAnimation;
    //ONLY use my first row two frames
    birdAnimation[LEFT] = {1};     
    birdAnimation[RIGHT] = {1};    
    birdAnimation[UP] = {0};      
    birdAnimation[DOWN] = {0};     
    
    // Bird starts on the left side
    bird1 = new Entity(
        {100.0f, 100.0f},           
        {100, 100},                   
        "assets/bird.png",           
        ATLAS,                       
        {2, 3},                      
        birdAnimation 
    );
    
    bird1->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
    
    // House platform at bottom right
    house = new Entity(
        {SCREEN_WIDTH - 150.0f, SCREEN_HEIGHT - 125.0f},
        {250, 250},
        "assets/house.png"
    );
    
    // Grass obstacles 
    grass1 = new Entity(
        {SCREEN_WIDTH / 2.5f, SCREEN_HEIGHT - 400.0f},
        {100, 100},
        "assets/grass.png"
    );
    
    grass2 = new Entity(
        {SCREEN_WIDTH / 1.5f, SCREEN_HEIGHT - 250.0f},
        {100, 100},
        "assets/grass.png"
    );
    
    SetTargetFPS(FPS);
}
void processInput()
{    
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()){
        gAppStatus = TERMINATED;
    }

    if(gGameWon || gGameLost){
        return;
    }

    // Gravity always occur
    Vector2 accel = {0.0f, ACCELERATION_OF_GRAVITY};
    bool isMoving = false;
    
    if (gFuel > 0.0f)
    {
        if (IsKeyDown(KEY_A))
        {
            accel.x = -HORIZONTAL_ACCELERATION; //Left
            gFuel -= 1.6f;
            isMoving = true;
        }
        else if (IsKeyDown(KEY_D))
        {
            accel.x = HORIZONTAL_ACCELERATION;    // Right 
            gFuel -= 1.6f; //100 * 1/60 ~= 1.6px
            isMoving = true;
        }
    }
    
    if (gFuel < 0.0f) gFuel = 0.0f;
    
    // Change animation based on movement 
    if (isMoving) {
        bird1->moveRight();
    } else {
        bird1->moveDown(); 
    }
    
    // Update acceleration
    bird1->setAcceleration(accel);
}

void update() 
{   
    if(gGameWon || gGameLost){
        return;
    }
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    // Create grass array
    Entity* grasses[] = {grass1, grass2};
    int grassCount = 2;
    
    // Create collidables array
    Entity* collidables[] = {house};
    int collidableCount = 1;
    
    while (deltaTime >= FIXED_TIMESTEP)
    {
        // Check grass collision
        for (int i = 0; i < grassCount; i++) {
            if (grasses[i] == nullptr) continue;
            
            Vector2 birdPos = bird1->getPosition();
            Vector2 birdSize = bird1->getColliderDimensions();
            Vector2 grassPos = grasses[i]->getPosition();
            Vector2 grassSize = grasses[i]->getColliderDimensions();
            float distX = fabs(birdPos.x - grassPos.x);
            float distY = fabs(birdPos.y - grassPos.y);
            if (distX < (birdSize.x / 2.0f + grassSize.x / 2.0f) &&
                distY < (birdSize.y / 2.0f + grassSize.y / 2.0f)) {
                gGameLost = true;  // Hit grass = Mission Failed
                break;
            }
        }
        
        // Mission Failed
        if (gGameLost) {
            deltaTime -= FIXED_TIMESTEP;
            break;
        }
        
        // CHECK HOUSE
        bird1->update(FIXED_TIMESTEP, collidables[0], collidableCount);

        // Check collisions
        if (!gGameWon && !gGameLost)
        {
            // Check if landed on house
            if (bird1->isCollidingBottom()) {
                gGameWon = true;  // Mission Accomplished
            }
            
            // Hit screen boundaries
            if (
                bird1->getPosition().y > SCREEN_HEIGHT-50 ||
                bird1->getPosition().x < 100 || 
                bird1->getPosition().x > SCREEN_WIDTH - 50)
            {
                gGameLost = true;
            }
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    
    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));
    
    // Render house
    house->render();
    
    // Render grass
    grass1->render();
    grass2->render();
    
    // Render bird
    bird1->render();
    
    // Display fuel
    float fuelPercentage = (gFuel /850) * 100.0f;
    if (fuelPercentage < 0) fuelPercentage = 0;
    // shows at 10, 10
    DrawText(TextFormat("Fuel: %.0f%%", fuelPercentage), 10, 10, 20, WHITE);
    // Draw fuel bar
    int barWidth = 200;
    int barHeight = 20;
    int barX = 10;
    int barY = 45;
    DrawRectangle(barX, barY, barWidth, barHeight, GRAY);
    // Remaining fuel bar 
    DrawRectangle(barX, barY, (int)(barWidth * gFuel / 850.0f), barHeight, GREEN);
    DrawRectangleLines(barX, barY, barWidth, barHeight, BLACK);

    // Game result messages
    if (gGameWon)
    {
        // mesure text width and center it
        DrawText("Mission Accomplished!", 
                (SCREEN_WIDTH - MeasureText("Mission Accomplished!", 60))/2, 
                SCREEN_HEIGHT / 2, 60, GREEN);
    }
    else if (gGameLost)
    {
        DrawText("Mission Failed", 
                (SCREEN_WIDTH - MeasureText("Mission Failed", 60))/2, 
                SCREEN_HEIGHT / 2, 60, RED);
    }

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}