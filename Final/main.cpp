/**
* Author: Zhenwwei Zhan
 * Assignment: Adventurer's Journey
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
#include "CS3113/ShaderProgram.h"

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

int gPlayerLives = 3;
ShaderProgram gShader;
Vector2 gLightPosition = { 0.0f, 0.0f };
Texture2D gHeartTexture = { 0 };

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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ninja Adventure");
    InitAudioDevice();
    gShader.load("shaders/vertex.glsl", "shaders/fragment.glsl");
    gHeartTexture = LoadTexture("heart.png");

    gMenuScene = new MenuScene(ORIGIN, "#1a1a2e");
    gLevel1 = new Level1(ORIGIN, "#87CEEB");
    gLevel2 = new Level2(ORIGIN, "#87CEEB");
    gLevel3 = new Level3(ORIGIN, "#87CEEB");
    
    gScenes.push_back(gMenuScene);
    gScenes.push_back(gLevel1);
    gScenes.push_back(gLevel2);
    gScenes.push_back(gLevel3);
    
    // Start at Menu
    switchToScene(gMenuScene);

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (gCurrentScene == gMenuScene)
    {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
                gPlayerLives = 3;
                if (gLevel1 != nullptr) switchToScene(gLevel1);
                return;
            }
    }
    
    if (gCurrentScene != gMenuScene && gCurrentScene->getState().player != nullptr)
    {
            gCurrentScene->getState().player->resetMovement();

            if (gPlayerLives > 0) {
                if (IsKeyDown(KEY_W)) gCurrentScene->getState().player->moveUp();
                if (IsKeyDown(KEY_S)) gCurrentScene->getState().player->moveDown();
                if (IsKeyDown(KEY_A)) gCurrentScene->getState().player->moveLeft();
                if (IsKeyDown(KEY_D)) gCurrentScene->getState().player->moveRight();
                if (IsKeyPressed(KEY_SPACE)) gCurrentScene->getState().player->attack();

                if (GetLength(gCurrentScene->getState().player->getMovement()) > 1.0f) 
                    gCurrentScene->getState().player->normaliseMovement();
            }
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
        if (gCurrentScene != gMenuScene && gCurrentScene->getState().player != nullptr)
            gLightPosition = gCurrentScene->getState().player->getPosition();
        deltaTime -= FIXED_TIMESTEP;
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCurrentScene->getState().camera);
    
    if (gCurrentScene != gMenuScene && gShader.isLoaded()) {
        gShader.begin();
        gShader.setVector2("lightPosition", gLightPosition);
        gShader.setInt("isCharging", 1);
    }

    gCurrentScene->render();
    
    if (gCurrentScene != gMenuScene && gShader.isLoaded())
        gShader.end();

    EndMode2D();

    // Render UI (unaffected by shader and camera)
    if (gCurrentScene) gCurrentScene->renderUI();
    
    if (gCurrentScene != gMenuScene && gHeartTexture.id != 0) {
        constexpr float heartSize = 40.0f;
        constexpr float heartSpacing = 50.0f;
        constexpr float startX = 20.0f;
        constexpr float startY = 20.0f;
        
        for (int i = 0; i < gPlayerLives; i++) {
            DrawTextureEx(gHeartTexture, {startX + i * heartSpacing, startY}, 0.0f, heartSize / gHeartTexture.width, WHITE);
        }
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
    gShader.unload();
    if (gHeartTexture.id != 0) UnloadTexture(gHeartTexture);

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
                if (loseTimer >= 2.0f) {
                    gPlayerLives = 3;
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
