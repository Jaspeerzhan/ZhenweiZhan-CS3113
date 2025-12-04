#include "MenuScene.h"
#include "raylib.h"

MenuScene::MenuScene() : Scene{{0.0f}, nullptr} {}
MenuScene::MenuScene(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} 
{
    // Load menu texture
    mMenuTexture = LoadTexture("menu.jpg");
}
MenuScene::~MenuScene() 
{
    UnloadTexture(mMenuTexture);
}

void MenuScene::initialise()
{
    mGameState.nextSceneID = 0;
    mGameState.camera.zoom = 1.0f;
    mGameState.camera.offset = {0, 0};
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.target = {0, 0};
}

void MenuScene::update(float deltaTime)
{
    // Nothing to update
}

void MenuScene::render()
{
    constexpr int SCREEN_WIDTH = 1000;
    constexpr int SCREEN_HEIGHT = 600;

    // Draw texture scaled to fit screen
    Rectangle source = {0.0f, 0.0f, (float)mMenuTexture.width, (float)mMenuTexture.height};
    Rectangle dest = {0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
    DrawTexturePro(mMenuTexture, source, dest, {0, 0}, 0.0f, WHITE);

    // "Press Enter" text (No blinking)
    const char* text = "PRESS ENTER TO START";
    int fontSize = 40;
    int textWidth = MeasureText(text, fontSize);
    // Draw Shadow
    DrawText(text, (SCREEN_WIDTH - textWidth) / 2 + 2, 320 + 2, fontSize, BLACK);
    // Draw Text
    DrawText(text, (SCREEN_WIDTH - textWidth) / 2, 320, fontSize, WHITE);
}

void MenuScene::shutdown()
{
    // UnloadTexture(mMenuTexture); // Handled in destructor
}
