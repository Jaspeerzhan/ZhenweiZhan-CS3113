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

#ifndef LEVEL1_H
#define LEVEL1_H

constexpr int LEVEL1_WIDTH = 48, 
              LEVEL1_HEIGHT = 24; 

class Level1 : public Scene {
private:
    Vector2 mPlayerStartPosition;
    
    std::vector<Entity*> mEnemies;
    std::vector<Vector2> mEnemySpawnPositions;
    float mEnemyPatrolRange = 200.0f;
    float mEnemyMaxChaseDistance = 300.0f;
    
    // Portal (teleporter to next level)
    Entity* mPortal = nullptr;
    
    bool mWasAttacking = false; // Track previous frame attack state
    
    // Top-down map using tile.png (4 rows, 1 column)
    // 0 = empty, 1-4 = different tiles from tile.png
    // Map is 64x32, generated in initialise()
    // Outer border is wall (1), inner area is grass (2)
    unsigned int mLevelData[LEVEL1_WIDTH * LEVEL1_HEIGHT];

public:
    static constexpr float TILE_DIMENSION = 32.0f; // Smaller tiles for top-down view

    Level1();
    Level1(Vector2 origin, const char *bgHexCode);
    ~Level1();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void renderUI() override;
    void shutdown() override;
};

#endif
