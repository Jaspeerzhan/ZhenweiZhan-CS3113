/**
 * Author: Zhenwwei Zhan
 * Assignment: Rise of the AI
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "Level1.h"

extern int gPlayerLives;

Level1::Level1() : Scene { {0.0f}, nullptr } {}
Level1::Level1(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
Level1::~Level1() { shutdown(); }

void Level1::initialise()
{
   mGameState.nextSceneID = 0;
   
   // Load bgm
   mGameState.bgm = LoadMusicStream("myAssets/Sounds/bgm.mp3");
   SetMusicVolume(mGameState.bgm, 0.33f);
   PlayMusicStream(mGameState.bgm);
   
   // Load  sounds
   mGameState.jumpSound = LoadSound("myAssets/Sounds/sfx_jump.ogg");
   mGameState.hurtSound = LoadSound("myAssets/Sounds/sfx_hurt.ogg");
   mGameState.levelCompleteSound = LoadSound("myAssets/Sounds/sfx_coin.ogg");
   mGameState.playerDeathSound = LoadSound("myAssets/Sounds/sfx_disappear.ogg");

   // Initialize map
   mGameState.map = new Map(
      LEVEL1_WIDTH, LEVEL1_HEIGHT,
      (unsigned int *) mLevelData,
      "myAssets/Spritesheets/spritesheet-tiles-default.png",
      TILE_DIMENSION,
      18, 18,
      mOrigin
   );

   // Animation atlas
   std::map<Direction, std::vector<int>> playerAnimationAtlas = {
      {DOWN,  { 33 }},      // idle
      {LEFT,  { 9, 17 }},   // walk
      {UP,    { 10, 2 }},   // climb
      {RIGHT, { 9, 17 }},   // walk
   };

   float characterScale = 128.0f;

   // player entity
   mGameState.player = new Entity(
      {mOrigin.x - 400.0f, mOrigin.y - 200.0f},
      {characterScale, characterScale},
      "myAssets/Spritesheets/spritesheet-characters-default.png",
      ATLAS,
      //since there are 2 cols are empty, so its 8
      { 8, 8 },
      playerAnimationAtlas,
      PLAYER
   );
   // I tried 550 and another, 650 seems the best
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
      {DOWN,  { 42 }},      // idle
      {LEFT,  { 18, 26 }},  // walk
      {UP,    { 19, 11 }},  // climb
      {RIGHT, { 18, 26 }},  // walk
   };

   float enemyScale = 128.0f;

   // Create enemy follower
   mGameState.enemy = new Entity(
      {mOrigin.x + 100.0f, mOrigin.y - 200.0f},
      {enemyScale, enemyScale},
      "myAssets/Spritesheets/spritesheet-characters-default.png",
      ATLAS,
      { 8, 8 },
      enemyAnimationAtlas,
      NPC
   );

   mGameState.enemy->setAIType(FOLLOWER);
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

void Level1::update(float deltaTime)
{
   mGameState.lives = gPlayerLives;
   
   // Update background music stream
   UpdateMusicStream(mGameState.bgm);

   // Update player
   mGameState.player->update(
      deltaTime,
      nullptr,
      mGameState.map,
      nullptr,
      0
   );

   // Update enemy
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
               PlaySound(mGameState.playerDeathSound);
           }
       }
   }

   Vector2 playerPos = mGameState.player->getPosition();
   
   // Calculate map boundaries
   float mapLeftBoundary = mOrigin.x - (LEVEL1_WIDTH * TILE_DIMENSION) / 2.0f;
   float mapTopBoundary = mOrigin.y - (LEVEL1_HEIGHT * TILE_DIMENSION) / 2.0f;
   float mapRightBoundary = mapLeftBoundary + (LEVEL1_WIDTH * TILE_DIMENSION);
   float mapBottomBoundary = mapTopBoundary + (LEVEL1_HEIGHT * TILE_DIMENSION);
   
   // Restrict player movement to left boundary
   Vector2 playerScale = mGameState.player->getScale();
   float playerHalfWidth = playerScale.x / 2.0f;
   
   if (playerPos.x - playerHalfWidth < mapLeftBoundary)
   {
       mGameState.player->setPosition({mapLeftBoundary + playerHalfWidth, playerPos.y});
       mGameState.player->setVelocity({0.0f, mGameState.player->getVelocity().y});
   }
   
   playerPos = mGameState.player->getPosition();
   

   // Check level completion (player walks past right boundary)
   if (playerPos.x + playerHalfWidth > mapRightBoundary)
   {
       mGameState.nextSceneID = 2;
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

void Level1::render()
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

void Level1::shutdown()
{
   delete mGameState.player;
   delete mGameState.enemy;
   delete mGameState.map;

   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.jumpSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.levelCompleteSound);
   UnloadSound(mGameState.playerDeathSound);
}
