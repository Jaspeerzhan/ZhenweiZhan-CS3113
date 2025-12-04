/**
 * Author: Zhenwwei Zhan
 * Assignment: Adventurer's Journey
 * Date due: 2025-11-08, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "Scene.h"
#include <vector>

#ifndef LEVEL2_H
#define LEVEL2_H

constexpr int LEVEL2_WIDTH = 48,
              LEVEL2_HEIGHT = 24;

class Level2 : public Scene {
private:
    Vector2 mPlayerStartPosition;
    Vector2 mEnemyStartPosition;
    
    std::vector<Entity*> mEnemies;
    std::vector<Vector2> mEnemySpawnPositions;
    
    // Portal (teleporter to next level)
    Entity* mPortal = nullptr;
    
    bool mWasAttacking = false; // Track previous frame attack state
    
    // Top-down map using tile.png (4 rows, 1 column)
    // 0 = empty, 1 = wall, 2 = grass
    // Map is 48x24, generated in initialise()
    unsigned int mLevelData[LEVEL2_WIDTH * LEVEL2_HEIGHT];

public:
    static constexpr float TILE_DIMENSION = 32.0f; // Smaller tiles for top-down view

    Level2();
    Level2(Vector2 origin, const char *bgHexCode);
    ~Level2();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif
