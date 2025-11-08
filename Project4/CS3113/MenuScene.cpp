/**
 * Author: Zhenwwei Zhan
 * Assignment: Rise of the AI
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "MenuScene.h"

MenuScene::MenuScene() : Scene { {0.0f}, nullptr } {}

MenuScene::MenuScene(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

MenuScene::~MenuScene() { shutdown(); }

void MenuScene::initialise()
{
    mGameState.nextSceneID = 0;
    
    // Menu doesn't need player, map, or audio
    mGameState.player = nullptr;
    mGameState.map = nullptr;
    
    mGameState.camera = { 0 };
    mGameState.camera.target = mOrigin;
    mGameState.camera.offset = mOrigin;
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.zoom = 1.0f;
}

void MenuScene::update(float deltaTime)
{
}

void MenuScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    
    // Draw game title
    const char* gameTitle = "Space Travel";
    int titleFontSize = 60;
    Vector2 titleSize = MeasureTextEx(GetFontDefault(), gameTitle, titleFontSize, 2.0f);
    Vector2 titlePosition = {
        mOrigin.x - titleSize.x / 2.0f,
        mOrigin.y - 100.0f
    };
    DrawTextEx(GetFontDefault(), gameTitle, titlePosition, titleFontSize, 2.0f, WHITE);
    
    // Draw instruction text
    const char* instructionText = "Press Enter to Start";
    int instructionFontSize = 30;
    Vector2 instructionSize = MeasureTextEx(GetFontDefault(), instructionText, instructionFontSize, 2.0f);
    Vector2 instructionPosition = {
        mOrigin.x - instructionSize.x / 2.0f,
        mOrigin.y + 50.0f
    };
    DrawTextEx(GetFontDefault(), instructionText, instructionPosition, instructionFontSize, 2.0f, YELLOW);
}

void MenuScene::shutdown()
{
}

