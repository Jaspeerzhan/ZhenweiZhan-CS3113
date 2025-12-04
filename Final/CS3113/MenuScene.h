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

#ifndef MENUSCENE_H
#define MENUSCENE_H

class MenuScene : public Scene {
private:
    Texture2D mMenuTexture;
    
public:
    MenuScene();
    MenuScene(Vector2 origin, const char *bgHexCode);
    ~MenuScene();
    
    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif

