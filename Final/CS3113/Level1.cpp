/**
 * Author: Zhenwwei Zhan
 * Assignment: Adventurer's Journey
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
   
   // Load BGM and sound effects
   mGameState.bgm = LoadMusicStream("loop.mp3");
   SetMusicVolume(mGameState.bgm, 0.6f);
   PlayMusicStream(mGameState.bgm);

   mGameState.attackSound = LoadSound("attack.mp3");
   mGameState.hurtSound = LoadSound("hurt.mp3");
   mGameState.playerDeathSound = LoadSound("gameOver.mp3");
   mGameState.levelCompleteSound = LoadSound("myAssets/Sounds/sfx_magic.ogg");
   
   mWasAttacking = false;

   // Generate level map
   // Fill with grass (2)
   for (int i = 0; i < LEVEL1_WIDTH * LEVEL1_HEIGHT; i++) {
       mLevelData[i] = 2;
   }

   // Add borders and obstacles
   for (int row = 0; row < LEVEL1_HEIGHT; row++) {
       for (int col = 0; col < LEVEL1_WIDTH; col++) {
           int index = row * LEVEL1_WIDTH + col;
           
           // Outer borders
           if (row == 0 || row == LEVEL1_HEIGHT - 1 || col == 0 || col == LEVEL1_WIDTH - 1) {
               mLevelData[index] = 1; // Wall
           }
           // Top-Left Room 
           else if ((row == 9 && col <= 12 && !(col >= 6 && col <= 7)) || 
                    (col == 12 && row <= 9 && !(row >= 4 && row <= 5))) {
               mLevelData[index] = 1;
           }
           // Top-Right Room
           // Entrance: col 40-41 (bottom), row 4-5 (left)
           else if ((row == 9 && col >= 35 && col != 42 && !(col >= 40 && col <= 41)) || 
                    (col == 35 && row <= 9 && !(row >= 4 && row <= 5))) {
               mLevelData[index] = 1;
           }
           // Bottom-Left Room
           // Entrance: col 6-7 (top), row 19-20 (right)
           else if ((row == 16 && col <= 12 && !(col >= 6 && col <= 7)) || 
                    (col == 12 && row >= 16 && !(row >= 19 && row <= 20))) {
               mLevelData[index] = 1;
           }
           // Bottom-Right Room
           // Entrance: col 40-41 (top), row 19-20 (left)
           else if ((row == 16 && col >= 35 && col != 42 && !(col >= 40 && col <= 41)) || 
                    (col == 35 && row >= 16 && !(row >= 19 && row <= 20))) {
               mLevelData[index] = 1;
           }
       }
   }

   mGameState.map = new Map(
      LEVEL1_WIDTH, LEVEL1_HEIGHT,
      (unsigned int *) mLevelData,
      "tile.png",
      Level1::TILE_DIMENSION,
      1, 4,  
      mOrigin
   );
   float characterScale = 64.0f;

   mPlayerStartPosition.x = mOrigin.x - (LEVEL1_WIDTH * Level1::TILE_DIMENSION) / 2.0f + 4 * Level1::TILE_DIMENSION;
   mPlayerStartPosition.y = mOrigin.y - 2 * Level1::TILE_DIMENSION;
   
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
   mGameState.player->setDirection(DOWN);
   
   // Clear existing enemies
   mEnemies.clear();
   mEnemySpawnPositions.clear();

   // Create 5 enemies at fixed positions
   float enemyScale = 64.0f;
   float mapLeft = mOrigin.x - (LEVEL1_WIDTH * Level1::TILE_DIMENSION) / 2.0f;
   float mapTop = mOrigin.y - (LEVEL1_HEIGHT * Level1::TILE_DIMENSION) / 2.0f;
   
   // Animation frames
   std::vector<int> downIdle = {0};
   std::vector<int> downMove = {1, 2, 3, 4, 5, 6, 7};
   std::vector<int> upIdle = {8};
   std::vector<int> upMove = {9, 10, 11, 12, 13, 14, 15};
   std::vector<int> leftIdle = {16};
   std::vector<int> leftMove = {17, 18, 19, 20, 21, 22, 23};
   std::vector<int> rightIdle = {24};
   std::vector<int> rightMove = {25, 26, 27, 28, 29, 30, 31};
   
   Vector2 enemyPositions[6] = {
      {mapLeft + 6 * Level1::TILE_DIMENSION, mapTop + 4 * Level1::TILE_DIMENSION},   // Top-left room center
      {mapLeft + 41 * Level1::TILE_DIMENSION, mapTop + 4 * Level1::TILE_DIMENSION},  // Top-right room center
      {mapLeft + 6 * Level1::TILE_DIMENSION, mapTop + 20 * Level1::TILE_DIMENSION},  // Bottom-left room center
      {mapLeft + 41 * Level1::TILE_DIMENSION, mapTop + 20 * Level1::TILE_DIMENSION},  // Bottom-right room center
      {mapLeft + 20 * Level1::TILE_DIMENSION, mapTop + 12 * Level1::TILE_DIMENSION},  // Center Left
      {mapLeft + 28 * Level1::TILE_DIMENSION, mapTop + 12 * Level1::TILE_DIMENSION}   // Center Right
   };
   
   // Verify positions
   for (int i = 0; i < 6; i++) {
       Vector2 pos = enemyPositions[i];
       int tileCol = (int)((pos.x - mapLeft) / Level1::TILE_DIMENSION);
       int tileRow = (int)((pos.y - mapTop) / Level1::TILE_DIMENSION);
       if (tileCol >= 0 && tileCol < LEVEL1_WIDTH && tileRow >= 0 && tileRow < LEVEL1_HEIGHT) {
            int tileIndex = tileRow * LEVEL1_WIDTH + tileCol;
            if (mLevelData[tileIndex] == 1) {
                enemyPositions[i].x += Level1::TILE_DIMENSION; // Shift if wall
            }
       }
   }
   
   for (int i = 0; i < 6; i++) {
       Entity* enemy = new Entity(
          enemyPositions[i],
          {enemyScale, enemyScale},
          "enemy1.png",
          ATLAS,
          {8, 4},
          {
             {DOWN, downMove},
             {UP, upMove},
             {LEFT, leftMove},
             {RIGHT, rightMove}
          },
          NPC
       );
       
       enemy->setAIType(FOLLOWER);
       enemy->setAIState(WALKING);
       enemy->setSpeed(66); // Increased by 10% (was 60)
       enemy->setFrameSpeed(12);
       enemy->setColliderDimensions({enemyScale * 0.5f, enemyScale * 0.5f});
       enemy->setAcceleration({0.0f, 0.0f});
       
       mEnemies.push_back(enemy);
       mEnemySpawnPositions.push_back(enemyPositions[i]);
   }
   mGameState.enemy = mEnemies[0];
   
   // Portal in top-right room
   float portalScale = 64.0f;
   Vector2 portalPosition = {
       mapLeft + 43 * Level1::TILE_DIMENSION,
       mapTop + 5 * Level1::TILE_DIMENSION
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

void Level1::update(float deltaTime)
{
   mGameState.lives = gPlayerLives;
   if (mGameState.bgm.stream.buffer != nullptr) UpdateMusicStream(mGameState.bgm);
   
   if (mGameState.player != nullptr && mGameState.player->isAttacking() && !mWasAttacking) {
       PlaySound(mGameState.attackSound);
   }
   mWasAttacking = (mGameState.player != nullptr && mGameState.player->isAttacking());
   
   int activeEnemiesCount = 0;
   for (size_t i = 0; i < mEnemies.size(); i++) {
       Entity* enemy = mEnemies[i];
       enemy->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);
       
       if (enemy->isActive()) {
           activeEnemiesCount++;
           
           // Check player attack (Range + 20)
           if (mGameState.player != nullptr && mGameState.player->isActive() && mGameState.player->isAttacking()) {
                   float playerRadius = mGameState.player->getColliderDimensions().x / 2.0f;
                   float enemyRadius = enemy->getColliderDimensions().x / 2.0f;
                   float attackRange = playerRadius + enemyRadius + 20.0f;
                   
                   Vector2 playerPos = mGameState.player->getPosition();
                   Vector2 enemyPos = enemy->getPosition();
                   Direction playerDir = mGameState.player->getDirection();
                   Vector2 attackDir = {0.0f, 0.0f};
                   if (playerDir == UP) attackDir = {0.0f, -1.0f};
                   else if (playerDir == DOWN) attackDir = {0.0f, 1.0f};
                   else if (playerDir == LEFT) attackDir = {-1.0f, 0.0f};
                   else if (playerDir == RIGHT) attackDir = {1.0f, 0.0f};
                   
                   Vector2 toEnemy = {enemyPos.x - playerPos.x, enemyPos.y - playerPos.y};
                   if (Vector2Distance(playerPos, enemyPos) <= attackRange &&
                       (toEnemy.x * attackDir.x + toEnemy.y * attackDir.y) > 0.0f) {
                       Vector2 checkPos = {playerPos.x + attackDir.x * Level1::TILE_DIMENSION,
                                          playerPos.y + attackDir.y * Level1::TILE_DIMENSION};
                       float xOverlap, yOverlap;
                       if (!mGameState.map || !mGameState.map->isSolidTileAt(checkPos, &xOverlap, &yOverlap)) {
                           enemy->deactivate();
                       }
                   }
           }
           
           // Check player-enemy collision
           if (mGameState.player != nullptr && mGameState.player->isActive() && 
               !mGameState.player->isAttacking()) {
               if (mGameState.player->isColliding(enemy)) {
                   PlaySound(mGameState.hurtSound);
                   gPlayerLives--;
                   
                   // Reset player position
                   if (gPlayerLives > 0) {
                       mGameState.player->setPosition(mPlayerStartPosition);
                       mGameState.player->setVelocity({0.0f, 0.0f});
                       
                       // Reset alive enemies to spawn positions
                       for (size_t j = 0; j < mEnemies.size(); j++) {
                           if (mEnemies[j]->isActive()) {
                               mEnemies[j]->setPosition(mEnemySpawnPositions[j]);
                               mEnemies[j]->setAIState(WALKING);
                               // Ensure they are active (already checked, but good practice if logic changes)
                               mEnemies[j]->activate(); 
                           }
                       }
                   } else {
                       PlaySound(mGameState.playerDeathSound);
                       mGameState.nextSceneID = -2;
                   }
               }
           }
       }
   }
   
   if (mPortal != nullptr) mPortal->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

   mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
   
   // Camera Update
   float mapW = LEVEL1_WIDTH * Level1::TILE_DIMENSION;
   float mapH = LEVEL1_HEIGHT * Level1::TILE_DIMENSION;
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

   // Win Condition
   if (activeEnemiesCount == 0 && mPortal != nullptr && mGameState.player != nullptr) {
       if (mGameState.player->isColliding(mPortal)) {
           mGameState.nextSceneID = 2; 
       }
   }
}

void Level1::render()
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

void Level1::renderUI()
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

void Level1::shutdown()
{
   delete mGameState.player;
   delete mGameState.map;
   for (Entity* enemy : mEnemies) delete enemy;
   mEnemies.clear();
   if (mPortal != nullptr) delete mPortal;
   
   UnloadMusicStream(mGameState.bgm);
   UnloadSound(mGameState.attackSound);
   UnloadSound(mGameState.hurtSound);
   UnloadSound(mGameState.levelCompleteSound);
   UnloadSound(mGameState.playerDeathSound);
}
