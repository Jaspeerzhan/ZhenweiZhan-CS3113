#include "CS3113/cs3113.h"
#include <math.h>

// Global Constants
constexpr int   SCREEN_WIDTH  = 1600,
                SCREEN_HEIGHT = 800,
                FPS           = 60;

//bar texture            //    
Texture2D p1BarTexture;
Texture2D p2BarTexture;
Texture2D WholeGroundTexture;
Vector2 p1BarPosition = {25, SCREEN_HEIGHT / 2};
Vector2 p2BarPosition = {SCREEN_WIDTH-25, SCREEN_HEIGHT / 2};
Vector2 p1BarVelocity = {0, 0};
Vector2 p2BarVelocity = {0, 0};
float barThickness = 200.0f;  
float barLength   = 100.0f; 
bool twoPlayer = true;
float     gPreviousTicks = 0.0f;
AppStatus gAppStatus     = RUNNING;
float ySpeed = 300;      
//ball structure 
struct Ball {
    Texture2D ballTexture;
    Vector2 ballPosition = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    Vector2 ballVelocity = {200, 200};
    float ballDiameter = 30.0f;
    bool isActive = false;
};
Ball balls[3];
bool p1Won = false;
bool p2Won = false;
int numBalls = 1;
// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions

bool isColliding(const Vector2 *positionA,  const Vector2 *scaleA, 
    const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}



void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Textures");
    p1BarTexture = LoadTexture("assets/ground.jpg");
    p2BarTexture = LoadTexture("assets/ground.jpg");
    for (int i = 0; i < 3; i++) {
        balls[i].ballTexture = LoadTexture("assets/ball.png");
        balls[i].ballVelocity.x = 200 * (i % 2 == 0 ? 1 : -1);
        balls[i].ballVelocity.y = 200 * (i == 1 ? -1 : 1);
        balls[i].isActive = false;
    }
    balls[0].isActive = true;
    SetTargetFPS(FPS);
}

void processInput() 
{
    p1BarVelocity = {0, 0};
    p2BarVelocity = {0, 0};
    if (WindowShouldClose()) gAppStatus = TERMINATED;
    if (IsKeyPressed(KEY_Q)) {
        gAppStatus = TERMINATED;
    }
    //processed input for key
    if (IsKeyPressed(KEY_T)) {
        twoPlayer = !twoPlayer;
    }
    if (IsKeyDown(KEY_W)) {
        p1BarVelocity.y = -1;
    }
    if (IsKeyDown(KEY_S)) {
        p1BarVelocity.y = 1;
    }
    if (twoPlayer) {
        if (IsKeyDown(KEY_UP)) {
            p2BarVelocity.y = -1;
        }
        if (IsKeyDown(KEY_DOWN)) {
            p2BarVelocity.y = 1;
        }
    }
   //pressed 1,2,3 to change the number of balls
    if (IsKeyPressed(KEY_ONE)) {
        balls[0].isActive = true;
        balls[1].isActive = false;
        balls[2].isActive = false;
    }
    if (IsKeyPressed(KEY_TWO)) {
        balls[0].isActive = true;
        balls[1].isActive = true;
        balls[2].isActive = false;
    }
    if (IsKeyPressed(KEY_THREE)) {
        balls[0].isActive = true;
        balls[1].isActive = true;
        balls[2].isActive = true;
    }
    
}

void update() 
{   
    if (p1Won || p2Won ) {
        return;
    }
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    float halfHeight = barThickness * 0.5f;
    float topLimit   = halfHeight;
    float bottomLimit = SCREEN_HEIGHT - halfHeight;

    float newY1 = p1BarPosition.y + p1BarVelocity.y * ySpeed * deltaTime;
    if (newY1 <= topLimit)
        p1BarPosition.y = topLimit;
    else if (newY1 >= bottomLimit)
        p1BarPosition.y = bottomLimit;
    else
        p1BarPosition.y = newY1;

    if (twoPlayer) {
        float newY2 = p2BarPosition.y + p2BarVelocity.y * ySpeed * deltaTime;
        if (newY2 <= topLimit)
            p2BarPosition.y = topLimit;
        else if (newY2 >= bottomLimit)
            p2BarPosition.y = bottomLimit;
        else
            p2BarPosition.y = newY2;
    } else {
        static float dir = 1.0f; 
        p2BarPosition.y += dir * ySpeed  * deltaTime;
    
        if (p2BarPosition.y <= topLimit) {
            p2BarPosition.y = topLimit;
            dir = 1.0f;
        } else if (p2BarPosition.y >= bottomLimit) {
            p2BarPosition.y = bottomLimit;
            dir = -1.0f;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (balls[i].isActive) {
            balls[i].ballPosition.x += balls[i].ballVelocity.x * deltaTime;
            balls[i].ballPosition.y += balls[i].ballVelocity.y * deltaTime;
            //check wins
            if (balls[i].ballPosition.x < p1BarPosition.x + barLength*0.5f) { 
                p2Won = true;
            }
            if (balls[i].ballPosition.x > p2BarPosition.x - barLength*0.5f) { 
                p1Won = true;
            }
            if (balls[i].ballPosition.y <= balls[i].ballDiameter/2 || balls[i].ballPosition.y > SCREEN_HEIGHT - balls[i].ballDiameter/2) {
                balls[i].ballVelocity.y = -balls[i].ballVelocity.y;
            }
            Vector2 ballScale = {balls[i].ballDiameter, balls[i].ballDiameter};
            Vector2 barScale = {barLength, barThickness};
            if (isColliding(&balls[i].ballPosition, &ballScale, &p1BarPosition, &barScale)) {
                balls[i].ballVelocity.x = -balls[i].ballVelocity.x * 1.1f;
            }
            if (isColliding(&balls[i].ballPosition, &ballScale, &p2BarPosition, &barScale)) {
                balls[i].ballVelocity.x = -balls[i].ballVelocity.x * 1.1f;
            }

        }
    }

}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);
    if (p1Won) {
        DrawText("PLAYER 1 WINS!", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 40, RED);
    }
    if (p2Won) {
        DrawText("PLAYER 2 WINS!", SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 40, BLUE);
    }
    Rectangle p1Source = {
        0.0f,
        (float)p1BarTexture.height * 4.0f / 6.0f,
        (float)p1BarTexture.width,
        (float)p1BarTexture.height / 6.0f
    };
    Rectangle p2Source = {
        0.0f,
        (float)p2BarTexture.height * 5.0f / 6.0f,
        (float)p2BarTexture.width,
        (float)p2BarTexture.height / 6.0f
    };

    Rectangle p1Dest = { p1BarPosition.x, p1BarPosition.y, barThickness, barLength };
    Vector2   p1Origin = { p1Dest.width * 0.5f, p1Dest.height * 0.5f };
    Rectangle p2Dest = { p2BarPosition.x, p2BarPosition.y, barThickness, barLength };
    Vector2   p2Origin = { p2Dest.width * 0.5f, p2Dest.height * 0.5f };
    DrawTexturePro(p1BarTexture, p1Source, p1Dest, p1Origin, 90.0f, WHITE);
    DrawTexturePro(p2BarTexture, p2Source, p2Dest, p2Origin, 270.0f, WHITE);

    //draw ball
    for (int i = 0; i < 3; i++) {
        if (balls[i].isActive) {
            Rectangle ballDest = { balls[i].ballPosition.x, balls[i].ballPosition.y, balls[i].ballDiameter, balls[i].ballDiameter };
            Rectangle ballSource = { 0.0f, 0.0f, (float)balls[i].ballTexture.width, (float)balls[i].ballTexture.height };
            Vector2 ballOrigin = { balls[i].ballDiameter * 0.5f, balls[i].ballDiameter * 0.5f };
            DrawTexturePro(balls[i].ballTexture, ballSource, ballDest, ballOrigin, 0.0f, WHITE);
        }
    }
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