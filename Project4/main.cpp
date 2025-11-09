/**
* Author: Zhenwwei Zhan
* Assignment: Rise of the AI
* Date due: 2025-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/MenuScene.h"
#include "CS3113/Level1.h"
#include "CS3113/Level2.h"
#include "CS3113/Level3.h"

// Global Constants
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120;

constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gScenes = {};

MenuScene *gMenuScene = nullptr;
Level1 *gLevel1 = nullptr;
Level2 *gLevel2 = nullptr;
Level3 *gLevel3 = nullptr;

// Global game state: lives shared across entire game
int gPlayerLives = 3;

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{   
    if (scene == nullptr)
    {
        return;
    }
    
    if (gCurrentScene == scene)
    {
        return;
    }
    
    gCurrentScene = scene;
    gCurrentScene->initialise();
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Travel");
    InitAudioDevice();

    gMenuScene = new MenuScene(ORIGIN, "#1a1a2e");
    gLevel1 = new Level1(ORIGIN, "#87CEEB");
    gLevel2 = new Level2(ORIGIN, "#87CEEB");
    gLevel3 = new Level3(ORIGIN, "#87CEEB");
    
    gScenes.push_back(gMenuScene);
    gScenes.push_back(gLevel1);
    gScenes.push_back(gLevel2);
    gScenes.push_back(gLevel3);
    
    switchToScene(gScenes[0]);

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (gCurrentScene == gMenuScene)
    {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
        {
            gPlayerLives = 3;
            if (gLevel1 != nullptr)
            {
                switchToScene(gLevel1);
            }
            return;
        }
    }
    
    if (gCurrentScene != gMenuScene && gCurrentScene->getState().player != nullptr)
    {
            gCurrentScene->getState().player->resetMovement();

            if      (IsKeyDown(KEY_A)) gCurrentScene->getState().player->moveLeft();
            else if (IsKeyDown(KEY_D)) gCurrentScene->getState().player->moveRight();

            if (IsKeyPressed(KEY_W) && 
                gCurrentScene->getState().player->isCollidingBottom())
            {
                gCurrentScene->getState().player->jump();
                PlaySound(gCurrentScene->getState().jumpSound);
            }

            if (GetLength(gCurrentScene->getState().player->getMovement()) > 1.0f) 
                gCurrentScene->getState().player->normaliseMovement();
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) 
    {
        gAppStatus = TERMINATED;
    }
}

void update() 
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCurrentScene->getState().camera);

    gCurrentScene->render();

    EndMode2D();
    
    if (gCurrentScene != nullptr && gCurrentScene != gMenuScene)
    {
        char livesText[32];
        snprintf(livesText, sizeof(livesText), "Lives: %d", gPlayerLives);
        DrawText(livesText, 20, 20, 30, WHITE);
    }
    
    EndDrawing();
}

void shutdown() 
{
    delete gMenuScene;
    delete gLevel1;
    delete gLevel2;
    delete gLevel3;
    gMenuScene = nullptr;
    gLevel1 = nullptr;
    gLevel2 = nullptr;
    gLevel3 = nullptr;
    gCurrentScene = nullptr;

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        
        int nextID = gCurrentScene->getNextSceneID();
        if (nextID > 0 && nextID < (int)gScenes.size())
        {
            switchToScene(gScenes[nextID]);
        }
        else if (nextID == -1)
        {
            switchToScene(gScenes[0]);
        }
        else if (nextID == -2)
        {
            static float loseTimer = 0.0f;
            if (gCurrentScene == gLevel1 || gCurrentScene == gLevel2 || gCurrentScene == gLevel3)
            {
                loseTimer += FIXED_TIMESTEP;
                if (loseTimer >= 2.0f)
                {
                    switchToScene(gScenes[0]);
                    loseTimer = 0.0f;
                }
            }
            else
            {
                loseTimer = 0.0f;
            }
        }
        else if (nextID == -3)
        {
            static float winTimer = 0.0f;
            winTimer += FIXED_TIMESTEP;
            if (winTimer >= 2.0f)
            {
                switchToScene(gScenes[0]);
                gPlayerLives = 3;
                winTimer = 0.0f;
            }
        }
        
        render();
    }

    shutdown();

    return 0;
}
