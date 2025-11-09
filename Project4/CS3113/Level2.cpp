/**
 * Author: Zhenwwei Zhan
 * Assignment: Rise of the AI
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "Level2.h"

extern int gPlayerLives;

Level2::Level2() : Scene { {0.0f}, nullptr } {}
Level2::Level2(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
Level2::~Level2() { shutdown(); }

void Level2::initialise()
{
   mGameState.nextSceneID = 0;
   
   // Load BGM
   mGameState.bgm = LoadMusicStream("myAssets/Sounds/bgm.mp3");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);
   
   // Load sound effects
   mGameState.jumpSound = LoadSound("myAssets/Sounds/sfx_jump.ogg");
   mGameState.hurtSound = LoadSound("myAssets/Sounds/sfx_hurt.ogg");
   mGameState.levelCompleteSound = LoadSound("myAssets/Sounds/sfx_coin.ogg");

   // Initialize map
   mGameState.map = new Map(
      LEVEL2_WIDTH, LEVEL2_HEIGHT,
      (unsigned int *) mLevelData,
      "myAssets/Spritesheets/spritesheet-tiles-default.png",
      TILE_DIMENSION,
      18, 18,
      mOrigin
   );

   // Player animation atlas (purple character)
   std::map<Direction, std::vector<int>> playerAnimationAtlas = {
      {DOWN,  { 33 }},      
      {LEFT,  { 9, 17 }},   
      {UP,    { 10, 2 }},  
      {RIGHT, { 9, 17 }},   
   };
   
   float characterScale = 128.0f;

   mGameState.player = new Entity(
      {mOrigin.x - 400.0f, mOrigin.y - 200.0f},
      {characterScale, characterScale},
      "myAssets/Spritesheets/spritesheet-characters-default.png",
      ATLAS,
      { 8, 8 },
      playerAnimationAtlas,
      PLAYER
   );

   mGameState.player->setJumpingPower(650.0f);
   mGameState.player->setColliderDimensions({
      characterScale * 0.6f,
      characterScale * 0.8f
   });
   mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
   
   mPlayerStartPosition = {mOrigin.x - 400.0f, mOrigin.y - 200.0f};
   mEnemyStartPosition = {mOrigin.x + 100.0f, mOrigin.y - 200.0f};

   // Enemy animation atlas
   std::map<Direction, std::vector<int>> enemyAnimationAtlas = {
      {DOWN,  { 51 }},      
      {LEFT,  { 27, 35 }}, 
      {UP,    { 28, 20 }},  
      {RIGHT, { 27, 35 }}, 
   };

   float enemyScale = 128.0f;

   mGameState.enemy = new Entity(
      {mOrigin.x + 100.0f, mOrigin.y - 200.0f},
      {enemyScale, enemyScale},
      "myAssets/Spritesheets/spritesheet-characters-default.png",
      ATLAS,
      { 8, 8 },
      enemyAnimationAtlas,
      NPC
   );

   mGameState.enemy->setAIType(WANDERER);
   mGameState.enemy->setAIState(IDLE);
   mGameState.enemy->setSpeed(Entity::DEFAULT_SPEED * 0.2f);
   mGameState.enemy->setColliderDimensions({
      enemyScale * 0.6f,
      enemyScale * 0.8f
   });
   mGameState.enemy->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});
   mGameState.enemy->setDirection(LEFT);

   // Initialize camera
   mGameState.camera = { 0 };
   mGameState.camera.target = mGameState.player->getPosition();
   mGameState.camera.offset = mOrigin;
   mGameState.camera.rotation = 0.0f;
   mGameState.camera.zoom = 1.0f;
}

void Level2::update(float deltaTime)
{
   mGameState.lives = gPlayerLives;
   
   // Update background music stream
   UpdateMusicStream(mGameState.bgm);

   mGameState.player->update(
      deltaTime,
      nullptr,
      mGameState.map,
      nullptr,
      0
   );

   mGameState.enemy->update(
      deltaTime,
      mGameState.player,
      mGameState.map,
      nullptr,
      0
   );
   
   // Check player-enemy collision
   if (mGameState.player != nullptr && mGameState.enemy != nullptr &&
       mGameState.player->isActive() && mGameState.enemy->isActive())
   {
       if (mGameState.player->isColliding(mGameState.enemy))
       {
           gPlayerLives--;
           mGameState.lives = gPlayerLives;
           PlaySound(mGameState.hurtSound);
           
           // Reset positions
           mGameState.player->setPosition(mPlayerStartPosition);
           mGameState.player->setVelocity({0.0f, 0.0f});
           
           mGameState.enemy->setPosition(mEnemyStartPosition);
           mGameState.enemy->setVelocity({0.0f, 0.0f});
           mGameState.enemy->setDirection(LEFT);
           
           if (gPlayerLives <= 0)
           {
               mGameState.nextSceneID = -2;
           }
       }
   }

   Vector2 playerPos = mGameState.player->getPosition();
   
   // Calculate map boundaries
   float mapLeftBoundary = mOrigin.x - (LEVEL2_WIDTH * TILE_DIMENSION) / 2.0f;
   float mapTopBoundary = mOrigin.y - (LEVEL2_HEIGHT * TILE_DIMENSION) / 2.0f;
   float mapRightBoundary = mapLeftBoundary + (LEVEL2_WIDTH * TILE_DIMENSION);
   float mapBottomBoundary = mapTopBoundary + (LEVEL2_HEIGHT * TILE_DIMENSION);
   
   // Restrict player to left boundary
   Vector2 playerScale = mGameState.player->getScale();
   float playerHalfWidth = playerScale.x / 2.0f;
   
   if (playerPos.x - playerHalfWidth < mapLeftBoundary)
   {
       mGameState.player->setPosition({mapLeftBoundary + playerHalfWidth, playerPos.y});
       mGameState.player->setVelocity({0.0f, mGameState.player->getVelocity().y});
   }
   
   playerPos = mGameState.player->getPosition();
   
   // Check if player fell off 
   constexpr float scale = 128.0f;
   if (playerPos.y > mapBottomBoundary + scale && 
       playerPos.x >= mapLeftBoundary && 
       playerPos.x <= mapRightBoundary)
   {
       gPlayerLives = 0;
       mGameState.lives = 0;
       mGameState.nextSceneID = -2;
   }

   // Check level completion
   if (playerPos.x + playerHalfWidth > mapRightBoundary)
   {
       mGameState.nextSceneID = 3;
       PlaySound(mGameState.levelCompleteSound);
   }

   // Update camera
   constexpr int SCREEN_WIDTH = 1000;
   float cameraMaxX = mapRightBoundary - SCREEN_WIDTH / 2.0f;
   float cameraMinX = mapLeftBoundary + SCREEN_WIDTH / 2.0f;
   
   float targetCameraX = playerPos.x;
   if (targetCameraX > cameraMaxX) targetCameraX = cameraMaxX;
   if (targetCameraX < cameraMinX) targetCameraX = cameraMinX;
   
   mGameState.camera.target.x = targetCameraX;
   mGameState.camera.target.y = mOrigin.y;
}

void Level2::render()
{
   ClearBackground(ColorFromHex(mBGColourHexCode));

   mGameState.player->render();
   mGameState.enemy->render();
   mGameState.map->render();
   
   if (gPlayerLives <= 0)
   {
       const char* loseText = "You Lose!";
       int fontSize = 60;
       Vector2 textSize = MeasureTextEx(GetFontDefault(), loseText, fontSize, 2.0f);
       Vector2 textPosition = {
           mOrigin.x - textSize.x / 2.0f,
           mOrigin.y - textSize.y / 2.0f
       };
       DrawTextEx(GetFontDefault(), loseText, textPosition, fontSize, 2.0f, RED);
   }
}

void Level2::shutdown()
{
   delete mGameState.player;
   delete mGameState.enemy;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.levelCompleteSound);
}
