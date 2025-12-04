/**
 * Author: Zhenwwei Zhan
 * Assignment: Adventurer's Journey
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include <vector>
#include "Scene.h"

#ifndef LEVEL3_H
#define LEVEL3_H

constexpr int LEVEL3_WIDTH = 64,
              LEVEL3_HEIGHT = 32;

class Level3 : public Scene {
private:
    Vector2 mPlayerStartPosition;
    
    std::vector<Entity*> mEnemies;
    std::vector<Vector2> mEnemySpawnPositions;
    std::vector<Entity*> mBullets;
    
    // Portal (teleporter to next level)
    Entity* mPortal = nullptr;
    bool mWasAttacking = false;
    
    // Top-down map
    unsigned int mLevelData[LEVEL3_WIDTH * LEVEL3_HEIGHT];

public:
    static constexpr float TILE_DIMENSION = 32.0f;

    Level3();
    Level3(Vector2 origin, const char *bgHexCode);
    ~Level3();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif

