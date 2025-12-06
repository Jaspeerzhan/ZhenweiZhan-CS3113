/**
 * Author: Zhenwwei Zhan
 * Assignment: Adventurer's Journey
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "Level3.h"
#include "raymath.h"

extern int gPlayerLives;

Level3::Level3() : Scene{{0.0f}, nullptr} {}
Level3::Level3(Vector2 origin, const char *bgHexCode) : Scene{origin, bgHexCode} {}
Level3::~Level3() { shutdown(); }

void Level3::initialise()
{
    mGameState.nextSceneID = 0;
    mGameState.bgm = LoadMusicStream("loop.mp3");
    SetMusicVolume(mGameState.bgm, 0.6f);
    PlayMusicStream(mGameState.bgm);

    mGameState.attackSound = LoadSound("attack.mp3");
    mGameState.hurtSound = LoadSound("hurt.mp3");
    mGameState.playerDeathSound = LoadSound("gameOver.mp3");
    mGameState.levelCompleteSound = LoadSound("myAssets/Sounds/sfx_magic.ogg");
    mWasAttacking = false;

    // Map Gen
    for (int r=0; r<LEVEL3_HEIGHT; r++) {
        for (int c=0; c<LEVEL3_WIDTH; c++) {
            int i = r*LEVEL3_WIDTH + c;
            // Skip bricks at Watcher positions
            if ((r==10 && c==32) || (r==22 && c==32) || (r==6 && c==46)) {
                mLevelData[i]=2;
            }
            else if (r==0 || r==LEVEL3_HEIGHT-1 || c==0 || c==LEVEL3_WIDTH-1) mLevelData[i]=1;
            else if (r%4==0 && c%4==0) mLevelData[i]=1;
            else if (r==16 && c>10 && c<54) mLevelData[i]=1;
            else mLevelData[i]=2;
        }
    }
    mGameState.map = new Map(LEVEL3_WIDTH, LEVEL3_HEIGHT, (unsigned int*)mLevelData, "tile.png", Level3::TILE_DIMENSION, 1, 4, mOrigin);

    // Player
    float scale = 64.0f;
    mPlayerStartPosition = { mOrigin.x - (LEVEL3_WIDTH*TILE_DIMENSION)/2.0f + 3.5f*TILE_DIMENSION, 
                             mOrigin.y - (LEVEL3_HEIGHT*TILE_DIMENSION)/2.0f + 3.5f*TILE_DIMENSION };
    mGameState.player = new Entity(mPlayerStartPosition, {scale, scale}, "player/IDLE/idle_down.png", PLAYER);
    mGameState.player->setTextureType(ATLAS);
    mGameState.player->setSpriteSheetDimensions({8,1});
    std::vector<int> idle={0,1,2,3,4,5,6,7};
    mGameState.player->setAnimationAtlas({{UP,idle},{DOWN,idle},{LEFT,idle},{RIGHT,idle}});
    mGameState.player->setFrameSpeed(12);
    mGameState.player->setColliderDimensions({scale*0.5f, scale*0.5f});
    mGameState.player->setSpeed(150);
    mGameState.player->setDirection(DOWN);

    // Enemies
    mEnemies.clear(); mEnemySpawnPositions.clear(); mBullets.clear();
    float mapL = mOrigin.x - (LEVEL3_WIDTH*TILE_DIMENSION)/2.0f;
    float mapT = mOrigin.y - (LEVEL3_HEIGHT*TILE_DIMENSION)/2.0f;

    // Watchers (3)
    Vector2 wPos[] = {
        {mapL + 32.5f*TILE_DIMENSION, mapT + 10.5f*TILE_DIMENSION},  // North
        {mapL + 32.5f*TILE_DIMENSION, mapT + 22.5f*TILE_DIMENSION}, // South
        {mapL + 46.5f*TILE_DIMENSION, mapT + 6.5f*TILE_DIMENSION}   // East/North
    };
    for (auto& p : wPos) {
        Entity* e = new Entity(p, {scale, scale}, "witcher.png", ATLAS, {10,1}, 
            {{DOWN,{0,1,2,3,4,5,6,7,8,9}},{UP,{0,1,2,3,4,5,6,7,8,9}},
             {LEFT,{0,1,2,3,4,5,6,7,8,9}},{RIGHT,{0,1,2,3,4,5,6,7,8,9}}}, NPC);
        e->setAIType(WATCHER); e->setAIState(IDLE); e->setSpeed(0); e->setFrameSpeed(12);
        e->setDetectionRange(450.0f); // Increased range
        mEnemies.push_back(e); mEnemySpawnPositions.push_back(p);
    }

    // Chargers
    std::vector<int> dm={1,2,3,4,5,6,7}, um={9,10,11,12,13,14,15}, lm={17,18,19,20,21,22,23}, rm={25,26,27,28,29,30,31};
    Vector2 cPos[] = {
        {mapL + 50.5f*TILE_DIMENSION, mapT + 5.5f*TILE_DIMENSION},
        {mapL + 10.5f*TILE_DIMENSION, mapT + 25.5f*TILE_DIMENSION}
    };
    for (auto& p : cPos) {
        Entity* e = new Entity(p, {scale, scale}, "enemy1.png", ATLAS, {8,4}, 
            {{DOWN,dm},{UP,um},{LEFT,lm},{RIGHT,rm}}, NPC);
        e->setAIType(CHARGER); e->setAIState(IDLE); e->setSpeed(80); e->setFrameSpeed(10);
        mEnemies.push_back(e); mEnemySpawnPositions.push_back(p);
    }

    // Patroller
    Vector2 pPos = {mapL + 40.5f*TILE_DIMENSION, mapT + 20.5f*TILE_DIMENSION};
    Entity* patroller = new Entity(pPos, {scale, scale}, "enemy1.png", ATLAS, {8,4}, 
            {{DOWN,dm},{UP,um},{LEFT,lm},{RIGHT,rm}}, NPC);
    patroller->setAIType(FOLLOWER); patroller->setAIState(WALKING); patroller->setSpeed(60); patroller->setFrameSpeed(12);
    mEnemies.push_back(patroller); mEnemySpawnPositions.push_back(pPos);

    // Portal
    mGameState.camera.zoom = 1.0f;
    mGameState.camera.offset = {500.0f, 300.0f};
    mGameState.camera.rotation = 0.0f;
    mGameState.camera.target = mGameState.player->getPosition();
    
    Vector2 portalPos = { mOrigin.x + (LEVEL3_WIDTH*TILE_DIMENSION)/2.0f - 4.5f*TILE_DIMENSION,
                          mOrigin.y + (LEVEL3_HEIGHT*TILE_DIMENSION)/2.0f - 4.5f*TILE_DIMENSION };
    std::vector<int> pAnim = {0,1,2,3,4,5};
    mPortal = new Entity(portalPos, {scale, scale}, "portal.png", ATLAS, {3,2},
                        {{DOWN,pAnim},{UP,pAnim},{LEFT,pAnim},{RIGHT,pAnim}}, NONE);
    mPortal->setFrameSpeed(10);
    mPortal->setDirection(DOWN);
}

void Level3::update(float deltaTime)
{
    mGameState.lives = gPlayerLives;
    if (mGameState.bgm.stream.buffer) UpdateMusicStream(mGameState.bgm);

    if (mGameState.player && mGameState.player->isAttacking() && !mWasAttacking) 
        PlaySound(mGameState.attackSound);
    mWasAttacking = (mGameState.player && mGameState.player->isAttacking());

    int active = 0;
    for (size_t i=0; i<mEnemies.size(); i++) {
        Entity* e = mEnemies[i];
        e->update(deltaTime, mGameState.player, mGameState.map, nullptr, 0);

        // Spawn Bullet
        if (e->shouldShoot()) {
            e->didShoot();
            Entity* b = new Entity(e->getPosition(), {20.0f, 20.0f}, "", PROJECTILE);
            b->setEntityType(PROJECTILE); b->setSpeed(150);
            Vector2 d = Vector2Subtract(mGameState.player->getPosition(), e->getPosition());
            float len = GetLength(d);
            if (len>0) { d.x/=len; d.y/=len; b->setMovement(d); }
            mBullets.push_back(b);
        }

        // Attack (Range 42)
        if (e->isActive() && mGameState.player->isAttacking()) {
            Vector2 p = mGameState.player->getPosition(), ep = e->getPosition();
            Direction d = mGameState.player->getDirection();
            float range = 62.0f; bool hit = false;
            if (d==UP && ep.y<p.y && ep.y>p.y-range && abs(ep.x-p.x)<40) hit=true;
            else if (d==DOWN && ep.y>p.y && ep.y<p.y+range && abs(ep.x-p.x)<40) hit=true;
            else if (d==LEFT && ep.x<p.x && ep.x>p.x-range && abs(ep.y-p.y)<40) hit=true;
            else if (d==RIGHT && ep.x>p.x && ep.x<p.x+range && abs(ep.y-p.y)<40) hit=true;
            if (hit) e->deactivate();
        }
        
        // Collision (Hurt)
        if (e->isActive() && mGameState.player->isColliding(e) && !mGameState.player->isAttacking()) {
            PlaySound(mGameState.hurtSound);
            gPlayerLives--;
            if (gPlayerLives > 0) {
                mGameState.player->setPosition(mPlayerStartPosition);
                for(auto* b:mBullets) delete b; mBullets.clear();
                for(size_t j=0; j<mEnemies.size(); j++) {
                    mEnemies[j]->setPosition(mEnemySpawnPositions[j]);
                    mEnemies[j]->activate();
                    AIType t = mEnemies[j]->getAIType();
                    mEnemies[j]->setAIState( (t==WATCHER||t==CHARGER) ? IDLE : WALKING );
                }
            } else {
                PlaySound(mGameState.playerDeathSound);
                mGameState.nextSceneID = -2;
            }
        }
        if (e->isActive()) active++;
    }

    // Bullets
    for (size_t i=0; i<mBullets.size(); i++) {
        Entity* b = mBullets[i];
        b->update(deltaTime, nullptr, mGameState.map, nullptr, 0);
        bool kill = false;
        
        // Check wall collision (Explicit map check + Flags)
        float xo, yo;
        if (b->isCollidingTop()||b->isCollidingBottom()||b->isCollidingLeft()||b->isCollidingRight()) kill=true;
        else if (mGameState.map->isSolidTileAt(b->getPosition(), &xo, &yo)) kill=true;

        if (b->isColliding(mGameState.player)) {
            kill=true;
            PlaySound(mGameState.hurtSound);
            gPlayerLives--;
            if (gPlayerLives <= 0) {
                PlaySound(mGameState.playerDeathSound);
                mGameState.nextSceneID = -2;
            } else {
                mGameState.player->setPosition(mPlayerStartPosition);
                for(auto* bu:mBullets) delete bu; mBullets.clear();
                for(size_t j=0; j<mEnemies.size(); j++) {
                    mEnemies[j]->setPosition(mEnemySpawnPositions[j]);
                    mEnemies[j]->activate();
                    AIType t = mEnemies[j]->getAIType();
                    mEnemies[j]->setAIState( (t==WATCHER||t==CHARGER) ? IDLE : WALKING );
                }
                break; 
            }
        }
        if (kill) { delete b; mBullets.erase(mBullets.begin()+i); i--; }
    }

    if (mPortal) mPortal->update(deltaTime, nullptr, nullptr, nullptr, 0);
    mGameState.player->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

    // Camera
    float mW = LEVEL3_WIDTH*TILE_DIMENSION, mH = LEVEL3_HEIGHT*TILE_DIMENSION;
    float sW=1000, sH=600;
    mGameState.camera.target = mGameState.player->getPosition();
    mGameState.camera.target.x = Clamp(mGameState.camera.target.x, mOrigin.x-mW/2+sW/2, mOrigin.x+mW/2-sW/2);
    mGameState.camera.target.y = Clamp(mGameState.camera.target.y, mOrigin.y-mH/2+sH/2, mOrigin.y+mH/2-sH/2);

    if (active==0 && mPortal && mGameState.player->isColliding(mPortal)) {
        mGameState.nextSceneID = -3;
        PlaySound(mGameState.levelCompleteSound);
    }
    
    if (gPlayerLives <= 0) mGameState.nextSceneID = -2;
}

void Level3::render()
{
    DrawRectangleRec({-10000,-10000,20000,20000}, BLACK);
    if (mGameState.map) mGameState.map->render();
    if (mPortal) mPortal->render();
    for (auto& b : mBullets) b->render();
    for (auto& e : mEnemies) e->render();
    if (mGameState.player) mGameState.player->render();
}

void Level3::renderUI()
{
    int active = 0;
    for (auto& e : mEnemies) if (e && e->isActive()) active++;
    char text[32]; snprintf(text, 32, "Enemies: %d", active);
    DrawText(text, 26, 71, 30, BLACK);
    DrawText(text, 25, 70, 30, WHITE);

    if (gPlayerLives <= 0) {
        const char* msg = "You Lose!";
        int size = 60;
        Vector2 dim = MeasureTextEx(GetFontDefault(), msg, size, 2);
        Vector2 pos = { 500 - dim.x/2, 300 - dim.y/2 };
        DrawTextEx(GetFontDefault(), msg, {pos.x+3, pos.y+3}, size, 2, BLACK);
        DrawTextEx(GetFontDefault(), msg, pos, size, 2, YELLOW);
    } else if (mGameState.nextSceneID == -3) {
        const char* msg = "You Win!";
        int size = 60;
        Vector2 dim = MeasureTextEx(GetFontDefault(), msg, size, 2);
        Vector2 pos = { 500 - dim.x/2, 300 - dim.y/2 };
        DrawTextEx(GetFontDefault(), msg, {pos.x+3, pos.y+3}, size, 2, BLACK);
        DrawTextEx(GetFontDefault(), msg, pos, size, 2, GREEN);
    }
}

void Level3::shutdown()
{
    delete mGameState.player;
    delete mGameState.map;
    if (mPortal) delete mPortal;
    for (auto& e : mEnemies) delete e; mEnemies.clear();
    for (auto& b : mBullets) delete b; mBullets.clear();
    UnloadMusicStream(mGameState.bgm);
    UnloadSound(mGameState.hurtSound);
    UnloadSound(mGameState.levelCompleteSound);
    UnloadSound(mGameState.playerDeathSound);
    UnloadSound(mGameState.attackSound);
}