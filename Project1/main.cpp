#include "CS3113/cs3113.h"
#include <math.h>

// Global Constants
constexpr int   SCREEN_WIDTH  = 1200,
                SCREEN_HEIGHT = 800,
                FPS           = 60;
// Simple planet rotation settings
float speed = 2.0f;      // How fast it spins
constexpr float radius = 100.0f;

// Day/night cycle variables
float dayNightCycle = 0.0f;  // 0 = day, PI = night
float cycleSpeed = 2.0f;     // How fast the cycle changes

// Rock variables
float rockTime = 0.0f;
float rockSpeed = 0.3f;
Vector2 rockPosition = { 0.0f, 0.0f };
float rockScale = 1.5f;

// Star variables (heartbeat + waving)
float starTime = 0.0f;
float starSpeed = 2.0f;
Vector2 starPosition = { 0.0f, 0.0f };
float starScale = 1.0f;

// Heartbeat variables
float gPulseTime = 0.0f;
constexpr float PULSE_INCREMENT = 0.1f;
constexpr float BASE_SIZE = 1.0f;
constexpr float MAX_AMPLITUDE = 0.3f;
constexpr float PULSE_SPEED = 2.0f;

Vector2 ORIGIN = { radius + 80, radius + 80 };

// Global Variables
AppStatus gAppStatus     = RUNNING;
Texture2D planetTexture;
Texture2D backgroundTexture;
Texture2D rockTexture;
Texture2D starTexture;
Vector2   planetPosition = { 0.0f, 0.0f };
float     gAngle         = 0.0f;
float     gPreviousTicks = 0.0f;
        
// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Textures");

    planetTexture = LoadTexture("assets/planet.png");
    backgroundTexture = LoadTexture("assets/sky.jpg");
    rockTexture = LoadTexture("assets/rock.png");
    starTexture = LoadTexture("assets/star.png");
    
    SetTargetFPS(FPS);
}

void processInput() 
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Calculate delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    
    // Update day/night cycle
    dayNightCycle += cycleSpeed * deltaTime;
    if (dayNightCycle > 2 * PI) dayNightCycle = 0.0f;
    
    // Update rock movement 
    rockTime += rockSpeed * deltaTime;
    if (rockTime > 1.0f) rockTime = 0.0f; // Reset when reaches bottom-left
    // Linear movement 
    rockPosition.x = SCREEN_WIDTH - rockTime * SCREEN_WIDTH;
    rockPosition.y = rockTime * SCREEN_HEIGHT; 
    
    rockScale = 1.0f - rockTime * 0.9; 
    
    // Update star movement (heartbeat + waving)
    starTime += starSpeed * deltaTime;
    if (starTime > 2 * PI) starTime = 0.0f; // Reset cycle
    

    gPulseTime += PULSE_INCREMENT * deltaTime * 60.0f; 
    starScale = BASE_SIZE + MAX_AMPLITUDE * sin(gPulseTime / PULSE_SPEED);
    // https://mathworld.wolfram.com/HeartCurve.html found the heart function here
    float heartX = 16 * pow(sin(starTime), 3);
    float heartY = -(13 * cos(starTime) - 5 * cos(2*starTime) - 2 * cos(3*starTime) - cos(4*starTime)); 
    // Scale  to make heart bigger
    float heartScale = 8.0f; 
    starPosition.x = (SCREEN_WIDTH - 200) + heartX * heartScale;
    starPosition.y = (SCREEN_HEIGHT - 200) + heartY * heartScale;
    
    // Make the planet spin around
    gAngle += speed * deltaTime;
    planetPosition.x = ORIGIN.x + radius * cos(gAngle);
    planetPosition.y = ORIGIN.y + radius * sin(gAngle);
}

void render()
{
    BeginDrawing();
    // zero to 1 
    float brightness = (cos(dayNightCycle) + 1.0f) / 2.0f;
    ClearBackground(BLACK);
    Color skyTint = { 
        (unsigned char)(255 * brightness),     
        (unsigned char)(255 * brightness),      
        (unsigned char)(255 * brightness),     
        255                                     
    };
    DrawTexture(backgroundTexture, 0, 0, skyTint);

    Rectangle source = { 0, 0, (float)planetTexture.width, (float)planetTexture.height };
    Rectangle dest = { planetPosition.x, planetPosition.y, 80, 80 };
    Vector2 origin = { 40, 40 }; // Center of the planet
    
    DrawTexturePro(planetTexture, source, dest, origin, gAngle * 30, WHITE);

    // Draw the rock
    Rectangle rockSource = { 0, 0, (float)rockTexture.width, (float)rockTexture.height };
    Rectangle rockDest = { 
        rockPosition.x, 
        rockPosition.y, 
        150 * rockScale, 
        150 * rockScale 
    };
    Vector2 rockOrigin = { 75 * rockScale, 75 * rockScale };
    DrawTexturePro(rockTexture, rockSource, rockDest, rockOrigin, 0, WHITE);

    // Draw the star with heartbeat and waving effects
    Rectangle starSource = { 0, 0, (float)starTexture.width, (float)starTexture.height };
    Rectangle starDest = { 
        starPosition.x, 
        starPosition.y, 
        60 * starScale, 
        60 * starScale 
    };
    Vector2 starOrigin = { 30 * starScale, 30 * starScale };
    DrawTexturePro(starTexture, starSource, starDest, starOrigin, starTime * 20, WHITE);

    EndDrawing();
}

void shutdown() { CloseWindow(); }

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}