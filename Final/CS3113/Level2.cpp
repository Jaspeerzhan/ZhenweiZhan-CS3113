/**
 * Author: Zhenwwei Zhan
 * Assignment: Adventurer's Journey
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
   
   mGameState.bgm = LoadMusicStream("loop.mp3");
   SetMusicVolume(mGameState.bgm, 0.6f);
   PlayMusicStream(mGameState.bgm);

   mGameState.attackSound = LoadSound("attack.mp3");
   mGameState.hurtSound = LoadSound("hurt.mp3");
   mGameState.playerDeathSound = LoadSound("gameOver.mp3");
   
   mWasAttacking = false;
   
   for (int row = 0; row < LEVEL2_HEIGHT; row++) {
       for (int col = 0; col < LEVEL2_WIDTH; col++) {
           int index = row * LEVEL2_WIDTH + col;
           if (row == 0 || row == LEVEL2_HEIGHT - 1 || col == 0 || col == LEVEL2_WIDTH - 1) {
               mLevelData[index] = 1;
           }
           else if (
               (row >= 4 && row <= 7 && col >= 6 && col <= 9) ||
               (row >= 4 && row <= 7 && col >= 38 && col <= 41) ||
               (row >= 16 && row <= 19 && col >= 6 && col <= 9) ||
               (row >= 16 && row <= 19 && col >= 38 && col <= 41)
           ) {
               mLevelData[index] = 1;
           }
           else if (row >= 11 && row <= 12 && col >= 23 && col <= 24) {
               mLevelData[index] = 1;
           }
           else {
               mLevelData[index] = 2;
           }
       }
   }
   
   mGameState.map = new Map(
      LEVEL2_WIDTH, LEVEL2_HEIGHT,
      (unsigned int *) mLevelData,
      "tile.png",
      Level2::TILE_DIMENSION,
      1, 4,  
      mOrigin
   );

   float characterScale = 64.0f;
   mPlayerStartPosition.x = mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f + 3 * Level2::TILE_DIMENSION;
   mPlayerStartPosition.y = mOrigin.y;
   
   mGameState.player = new Entity(
      mPlayerStartPosition,
      {characterScale, characterScale},
      "player/IDLE/idle_down.png",
      PLAYER
   );
   
   mGameState.player->setTextureType(ATLAS);
   mGameState.player->setSpriteSheetDimensions({8, 1});
   std::vector<int> idleAnim = {0,1,2,3,4,5,6,7};
   mGameState.player->setAnimationAtlas({{UP,idleAnim},{DOWN,idleAnim},{LEFT,idleAnim},{RIGHT,idleAnim}});
   mGameState.player->setFrameSpeed(12);
   mGameState.player->setColliderDimensions({characterScale * 0.5f, characterScale * 0.5f});
   mGameState.player->setAcceleration({0.0f, 0.0f});
   mGameState.player->setSpeed(150);
   mGameState.player->setDirection(RIGHT);
   
   // Clear existing enemies
   mEnemies.clear();
   mEnemySpawnPositions.clear();

   float enemyScale = 64.0f;
   Vector2 chargerPositions[5] = {
       {mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f + 42 * Level2::TILE_DIMENSION, mOrigin.y},
       {mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f + 36 * Level2::TILE_DIMENSION, mOrigin.y - 6 * Level2::TILE_DIMENSION},
       {mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f + 36 * Level2::TILE_DIMENSION, mOrigin.y + 5 * Level2::TILE_DIMENSION},
       {mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f + 24 * Level2::TILE_DIMENSION, mOrigin.y - 6 * Level2::TILE_DIMENSION},
       {mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f + 24 * Level2::TILE_DIMENSION, mOrigin.y + 5 * Level2::TILE_DIMENSION}
   };

   for (int i = 0; i < 5; i++) {
       Entity* charger = new Entity(
           chargerPositions[i],
           {enemyScale, enemyScale},
           "enemy2.png",
           ATLAS,
           {4, 1},
           {
               {DOWN, {0, 1, 2, 3}},
               {UP, {0, 1, 2, 3}},
               {LEFT, {0, 1, 2, 3}},
               {RIGHT, {0, 1, 2, 3}}
           },
           NPC
       );

       charger->setAIType(CHARGER);
       charger->setAIState(IDLE);
       charger->setSpeed(80);
       charger->setFrameSpeed(10);
       charger->setColliderDimensions({enemyScale * 0.6f, enemyScale * 0.6f});
       charger->setAcceleration({0.0f, 0.0f});
       
       mEnemies.push_back(charger);
       mEnemySpawnPositions.push_back(chargerPositions[i]);
   }
   mGameState.enemy = mEnemies[0];

   float portalScale = 64.0f;
   float mapLeft = mOrigin.x - (LEVEL2_WIDTH * Level2::TILE_DIMENSION) / 2.0f;
   float mapTop = mOrigin.y - (LEVEL2_HEIGHT * Level2::TILE_DIMENSION) / 2.0f;
   Vector2 portalPosition = {
       mapLeft + 42 * Level2::TILE_DIMENSION,
       mOrigin.y
   };
   
   std::vector<int> portalAnim = {0, 1, 2, 3, 4, 5};
   mPortal = new Entity(
       portalPosition,
       {portalScale, portalScale},
       "portal.png",
       ATLAS,
       {3, 2},
       {
           {DOWN, portalAnim},
           {UP, portalAnim},
           {LEFT, portalAnim},
           {RIGHT, portalAnim}
       },
       NONE
   );
   mPortal->setFrameSpeed(10);

   // Initialize Camera
   mGameState.camera.zoom = 1.0f;
   mGameState.camera.offset = {500.0f, 300.0f}; // Screen Center
   mGameState.camera.rotation = 0.0f;
   mGameState.camera.target = mGameState.player->getPosition();
}

void Level2::update(float deltaTime)
{
   mGameState.lives = gPlayerLives;
   if (mGameState.bgm.stream.buffer != nullptr) UpdateMusicStream(mGameState.bgm);
   
   if (mGameState.player != nullptr && mGameState.player->isAttacking() && !mWasAttacking) {
       PlaySound(mGameState.attackSound);
   }
   mWasAttacking = (mGameState.player != nullptr && mGameState.player->isAttacking());
   
   int activeEnemies = 0;
   for (size_t i = 0; i < mEnemies.size(); i++) {
       Entity* enemy = mEnemies[i];
       enemy->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);
       
       if (enemy->isActive()) {
           activeEnemies++;
           // Check player attack (Extended Range)
           if (mGameState.player->isAttacking()) {
               Vector2 playerPos = mGameState.player->getPosition();
               Vector2 enemyPos = enemy->getPosition();
               Direction playerDir = mGameState.player->getDirection();
               
               float attackRange = 42.0f; 
               bool hit = false;
               
               if (playerDir == UP && enemyPos.y < playerPos.y && enemyPos.y > playerPos.y - attackRange && abs(enemyPos.x - playerPos.x) < 40) hit = true;
               else if (playerDir == DOWN && enemyPos.y > playerPos.y && enemyPos.y < playerPos.y + attackRange && abs(enemyPos.x - playerPos.x) < 40) hit = true;
               else if (playerDir == LEFT && enemyPos.x < playerPos.x && enemyPos.x > playerPos.x - attackRange && abs(enemyPos.y - playerPos.y) < 40) hit = true;
               else if (playerDir == RIGHT && enemyPos.x > playerPos.x && enemyPos.x < playerPos.x + attackRange && abs(enemyPos.y - playerPos.y) < 40) hit = true;
               
               if (hit) {
                   enemy->deactivate();
               }
           }

           // Check collision with player
           if (mGameState.player->isColliding(enemy) && !mGameState.player->isAttacking()) {
               gPlayerLives--;
               PlaySound(mGameState.hurtSound);
               if (gPlayerLives > 0) {
                   mGameState.player->setPosition(mPlayerStartPosition);
                   mGameState.player->setVelocity({0.0f, 0.0f});
                   enemy->setPosition(mEnemySpawnPositions[i]); 
                   enemy->setAIState(IDLE);
               } else {
                   PlaySound(mGameState.playerDeathSound);
                   mGameState.nextSceneID = -2;
               }
           }
       }
   }
   
   if (mPortal != nullptr) mPortal->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

   mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
   
   float mapW = LEVEL2_WIDTH * Level2::TILE_DIMENSION;
   float mapH = LEVEL2_HEIGHT * Level2::TILE_DIMENSION;
   constexpr int SCREEN_WIDTH = 1000;
   constexpr int SCREEN_HEIGHT = 600;
   mGameState.camera.target = mGameState.player->getPosition();
   
   if (mGameState.camera.target.x < mOrigin.x - mapW/2.0f + SCREEN_WIDTH/2.0f) 
       mGameState.camera.target.x = mOrigin.x - mapW/2.0f + SCREEN_WIDTH/2.0f;
   if (mGameState.camera.target.x > mOrigin.x + mapW/2.0f - SCREEN_WIDTH/2.0f) 
       mGameState.camera.target.x = mOrigin.x + mapW/2.0f - SCREEN_WIDTH/2.0f;
   if (mGameState.camera.target.y < mOrigin.y - mapH/2.0f + SCREEN_HEIGHT/2.0f) 
       mGameState.camera.target.y = mOrigin.y - mapH/2.0f + SCREEN_HEIGHT/2.0f;
   if (mGameState.camera.target.y > mOrigin.y + mapH/2.0f - SCREEN_HEIGHT/2.0f) 
       mGameState.camera.target.y = mOrigin.y + mapH/2.0f - SCREEN_HEIGHT/2.0f;
   
   if (activeEnemies == 0 && mPortal != nullptr && mGameState.player != nullptr) {
       if (mGameState.player->isColliding(mPortal)) {
           mGameState.nextSceneID = 3;
       }
   }
}

void Level2::render()
{
   DrawRectangleRec({-10000, -10000, 20000, 20000}, ColorFromHex(mBGColourHexCode));
   
   if (mGameState.map != nullptr) {
       float mapLeft = mGameState.map->getLeftBoundary();
       float mapRight = mGameState.map->getRightBoundary();
       float mapTop = mGameState.map->getTopBoundary();
       float mapBottom = mGameState.map->getBottomBoundary();
       DrawRectangleRec({mapLeft - 10000, mapTop - 10000, 10000, mapBottom - mapTop + 20000}, BLACK);
       DrawRectangleRec({mapRight, mapTop - 10000, 10000, mapBottom - mapTop + 20000}, BLACK);
       DrawRectangleRec({mapLeft - 10000, mapTop - 10000, mapRight - mapLeft + 20000, 10000}, BLACK);
       DrawRectangleRec({mapLeft - 10000, mapBottom, mapRight - mapLeft + 20000, 10000}, BLACK);
       mGameState.map->render();
   }

   if (mGameState.player != nullptr) mGameState.player->render();
   for (Entity* enemy : mEnemies) if (enemy != nullptr && enemy->isActive()) enemy->render();
   if (mPortal != nullptr) mPortal->render();
}

void Level2::renderUI()
{
   int activeEnemies = 0;
   for (Entity* enemy : mEnemies) if (enemy != nullptr && enemy->isActive()) activeEnemies++;
   
   char enemyText[32];
   snprintf(enemyText, sizeof(enemyText), "Enemies: %d", activeEnemies);
   DrawText(enemyText, 26, 71, 30, BLACK);
   DrawText(enemyText, 25, 70, 30, WHITE);

   if (gPlayerLives <= 0)
   {
       const char* loseText = "You Lose!";
       int fontSize = 60;
       Vector2 textSize = MeasureTextEx(GetFontDefault(), loseText, fontSize, 2.0f);
       Vector2 textPosition = { 1000 / 2.0f - textSize.x / 2.0f, 600 / 2.0f - textSize.y / 2.0f };
       DrawTextEx(GetFontDefault(), loseText, (Vector2){textPosition.x + 3, textPosition.y + 3}, fontSize, 2.0f, BLACK);
       DrawTextEx(GetFontDefault(), loseText, textPosition, fontSize, 2.0f, YELLOW);
   }
}

void Level2::shutdown()
{
   delete mGameState.player;
   delete mGameState.map;
   for (Entity* enemy : mEnemies) delete enemy;
   mEnemies.clear();
   if (mPortal != nullptr) delete mPortal;
   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.attackSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.playerDeathSound);
}
