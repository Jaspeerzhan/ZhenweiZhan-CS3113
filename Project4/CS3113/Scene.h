#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct GameState
{
    Entity *player;
    Entity *enemy;  // Enemy entity
    Map *map;

    Music bgm;                    // Background music (looping)
    Sound jumpSound;              // Jump sound effect
    Sound hurtSound;              // Player hurt sound effect (when hit by enemy)
    Sound levelCompleteSound;     // Level complete sound effect
    Sound playerDeathSound;       // Player death sound effect (when lives reach 0)

    Camera2D camera;

    int nextSceneID;
    int lives = 3;  // 玩家生命值（总共3条命）
};

class Scene 
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";
    
public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    
    GameState   getState()           const { return mGameState; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
    
    void setNextSceneID(int id) { mGameState.nextSceneID = id; }
    int getNextSceneID() const { return mGameState.nextSceneID; }
};

#endif