#include "Entity.h"
#include <string>
#include <cstring>

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
                   mTexture {0}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT}, 
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f}, 
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
    mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
    mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}}, 
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
    mAngle {0.0f}, mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction, 
        std::vector<int>> animationAtlas, EntityType entityType) : 
        mPosition {position}, mVelocity {0.0f, 0.0f}, 
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)}, 
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, 
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        // STEP 1: For every entity that our player can collide with...
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {
            // STEP 2: Calculate the distance between its centre and our centre
            //         and use that to calculate the amount of overlap between
            //         both bodies.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                              (collidableEntity->mColliderDimensions.y / 2.0f));
            
            // STEP 3: "Unclip" ourselves from the other entity, and zero our
            //         vertical velocity.
            if (mVelocity.y > 0) 
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0) 
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {            
            // When standing on a platform, we're always slightly overlapping
            // it vertically due to gravity, which causes false horizontal
            // collision detections. So the solution I dound is only resolve X
            // collisions if there's significant Y overlap, preventing the 
            // platform we're standing on from acting like a wall.
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            // Skip if barely touching vertically (standing on platform)
            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;

                // Collision!
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
 
                // Collision!
                mIsCollidingLeft = true;
            }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    float halfWidth = mColliderDimensions.x / 2.0f;
    float halfHeight = mColliderDimensions.y / 2.0f;
    
    Vector2 topCentreProbe    = { mPosition.x, mPosition.y - halfHeight };
    Vector2 topLeftProbe      = { mPosition.x - halfWidth, mPosition.y - halfHeight };
    Vector2 topRightProbe     = { mPosition.x + halfWidth, mPosition.y - halfHeight };

    Vector2 bottomCentreProbe = { mPosition.x, mPosition.y + halfHeight };
    Vector2 bottomLeftProbe   = { mPosition.x - halfWidth, mPosition.y + halfHeight };
    Vector2 bottomRightProbe  = { mPosition.x + halfWidth, mPosition.y + halfHeight };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ABOVE (top-down: moving up)
    if (mVelocity.y < 0.0f) {
        bool colliding = false;
        float maxOverlap = 0.0f;
        
        if (map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) && xOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, yOverlap);
        }
        if (map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap) && xOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, yOverlap);
        }
        if (map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap) && xOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, yOverlap);
        }
        
        if (colliding) {
            mPosition.y += maxOverlap * 1.01f;   // push down
        mVelocity.y  = 0.0f;
        mIsCollidingTop = true;
    }
    }

    // COLLISION BELOW (moving down)
    if (mVelocity.y > 0.0f) {
        bool colliding = false;
        float maxOverlap = 0.0f;
        
        if (map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) && xOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, yOverlap);
        }
        if (map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap) && xOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, yOverlap);
        }
        if (map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap) && xOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, yOverlap);
        }
        
        if (colliding) {
            mPosition.y -= maxOverlap * 1.01f;   // push up
        mVelocity.y  = 0.0f;
        mIsCollidingBottom = true;
        }
    } 
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    // Check multiple points along the collision box for better detection
    float halfWidth = mColliderDimensions.x / 2.0f;
    float halfHeight = mColliderDimensions.y / 2.0f;
    
    Vector2 leftTopProbe    = { mPosition.x - halfWidth, mPosition.y - halfHeight };
    Vector2 leftCentreProbe  = { mPosition.x - halfWidth, mPosition.y };
    Vector2 leftBottomProbe  = { mPosition.x - halfWidth, mPosition.y + halfHeight };
    
    Vector2 rightTopProbe   = { mPosition.x + halfWidth, mPosition.y - halfHeight };
    Vector2 rightCentreProbe = { mPosition.x + halfWidth, mPosition.y };
    Vector2 rightBottomProbe = { mPosition.x + halfWidth, mPosition.y + halfHeight };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ON RIGHT (moving right) - check multiple points
    if (mVelocity.x > 0.0f) {
        bool colliding = false;
        float maxOverlap = 0.0f;
        
        if (map->isSolidTileAt(rightTopProbe, &xOverlap, &yOverlap) && yOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, xOverlap);
        }
        if (map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap) && yOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, xOverlap);
        }
        if (map->isSolidTileAt(rightBottomProbe, &xOverlap, &yOverlap) && yOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, xOverlap);
        }
        
        if (colliding) {
            mPosition.x -= maxOverlap * 1.01f;   // push left
        mVelocity.x  = 0.0f;
        mIsCollidingRight = true;
    }
    }

    // COLLISION ON LEFT (moving left) - check multiple points
    if (mVelocity.x < 0.0f) {
        bool colliding = false;
        float maxOverlap = 0.0f;
        
        if (map->isSolidTileAt(leftTopProbe, &xOverlap, &yOverlap) && yOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, xOverlap);
        }
        if (map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap) && yOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, xOverlap);
        }
        if (map->isSolidTileAt(leftBottomProbe, &xOverlap, &yOverlap) && yOverlap >= 0.5f) {
            colliding = true;
            maxOverlap = fmaxf(maxOverlap, xOverlap);
        }
        
        if (colliding) {
            mPosition.x += maxOverlap * 1.01f;   // push right
        mVelocity.x  = 0.0f;
        mIsCollidingLeft = true;
        }
    }
}

bool Entity::isColliding(Entity *other) const 
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) - 
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) - 
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::animate(float deltaTime)
{
    if (mAnimationAtlas.find(mDirection) == mAnimationAtlas.end()) return;

    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    // mFrameSpeed is frames per second, so time per frame is 1.0f / mFrameSpeed
    float timePerFrame = 1.0f / (float)mFrameSpeed;

    if (mAnimationTime >= timePerFrame)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::AIWander() { moveLeft(); }

void Entity::AIFollow(Entity *target)
{
    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 300.0f) 
            mAIState = WALKING;
        break;

    case WALKING:
    {
        // Top-down following: move towards player in both X and Y
        Vector2 targetPos = target->getPosition();
        float dx = targetPos.x - mPosition.x;
        float dy = targetPos.y - mPosition.y;
        
        // Move in the direction with larger difference
        if (fabs(dx) > fabs(dy))
        {
            if (dx > 0) moveRight();
            else        moveLeft();
        }
        else
        {
            if (dy > 0) moveDown();
            else        moveUp();
        }
        
        // If too far, go back to idle
        if (Vector2Distance(mPosition, targetPos) > 400.0f)
            mAIState = IDLE;
        break;
    }
    
    default:
        break;
    }
}

void Entity::AIFly(Entity *target)
{
    // Fly left and right in the air
    if (mPatrolLeftBound == 0.0f && mPatrolRightBound == 0.0f)
    {
        // Initialize patrol bounds if not set
        mPatrolLeftBound = mPosition.x - 300.0f;
        mPatrolRightBound = mPosition.x + 300.0f;
    }
    
    // Change direction at boundaries
    if (mPosition.x >= mPatrolRightBound)
    {
        mPatrolDirection = false; // Move left
    }
    else if (mPosition.x <= mPatrolLeftBound)
    {
        mPatrolDirection = true; // Move right
    }
    
    // Move in current direction
    if (mPatrolDirection) moveRight();
    else moveLeft();
}

void Entity::AICharger(Entity *target)
{
    switch (mAIState)
    {
    case IDLE:
        mFrameSpeed = 10;
        // Check if player is within detection range (e.g. 300 pixels)
        if (Vector2Distance(mPosition, target->getPosition()) < 300.0f)
        {
            mAIState = PREPARING_CHARGE;
            mAIStateTimer = 1.0f; // 1 second preparation time
            mFrameSpeed = 15; // Faster flap when preparing
            // Face the player
            if (target->getPosition().x < mPosition.x) mDirection = LEFT;
            else mDirection = RIGHT;
        }
        break;

    case PREPARING_CHARGE:
        mAIStateTimer -= GetFrameTime();
        
        // Update direction to face player during prep
        if (target->getPosition().x < mPosition.x) mDirection = LEFT;
        else mDirection = RIGHT;

        if (mAIStateTimer <= 0.0f)
        {
            mAIState = CHARGING;
            mAIStateTimer = 2.0f; // Max charge duration 2 seconds
            mFrameSpeed = 20; // Fast flap during charge
            
            // Lock in charge direction towards player
            mChargeDirection = Vector2Subtract(target->getPosition(), mPosition);
            float length = GetLength(mChargeDirection);
            if (length > 0)
            {
                mChargeDirection.x /= length;
                mChargeDirection.y /= length;
            }
        }
        break;

    case CHARGING:
        mAIStateTimer -= GetFrameTime();
        
        // Move rapidly in charge direction
        // Charger speed multiplier
        mMovement = mChargeDirection;
        mSpeed = 200; // Reduced to 200
        
        // Stop charging if hit wall (checked via collision flags) or timer runs out
        if (mIsCollidingLeft || mIsCollidingRight || mIsCollidingTop || mIsCollidingBottom || mAIStateTimer <= 0.0f)
        {
            mAIState = CHARGING_COOLDOWN;
            mAIStateTimer = 2.0f; // 2 seconds cooldown
            mSpeed = 80; // Reset to slower normal speed
            mMovement = {0.0f, 0.0f}; // Stop moving
            mFrameSpeed = 8; // Slow flap during cooldown
        }
        break;

    case CHARGING_COOLDOWN:
        mAIStateTimer -= GetFrameTime();
        if (mAIStateTimer <= 0.0f)
        {
            mAIState = IDLE;
            mFrameSpeed = 10; // Back to normal
        }
        break;
        
    default:
        break;
    }
}

void Entity::AIWatcher(Entity *target)
{
    Vector2 targetPos = target->getPosition();
    float distance = Vector2Distance(mPosition, targetPos);
    
    switch (mAIState)
    {
    case IDLE:
    case WALKING:
        mMovement = {0.0f, 0.0f};
        
        if (distance < 220.0f)
        {
            // Move away to keep distance
            Vector2 away = Vector2Subtract(mPosition, targetPos);
            float len = GetLength(away);
            if (len > 0.0f) {
                away.x /= len;
                away.y /= len;
                mMovement = away;
            }
        }
        else if (distance > mWatcherRange)
        {
            // Move closer to stay within range
            Vector2 towards = Vector2Subtract(targetPos, mPosition);
            float len = GetLength(towards);
            if (len > 0.0f) {
                towards.x /= len;
                towards.y /= len;
                mMovement = towards;
            }
        }
        
        if (distance < mWatcherRange)
        {
            mAIState = AIMING;
            mAIStateTimer = 0.8f;
            mWatcherDamageApplied = false;
        }
        break;
        
    case AIMING:
        mMovement = {0.0f, 0.0f};
        mAIStateTimer -= GetFrameTime();
        
        // Face player
        if (fabs(targetPos.x - mPosition.x) > fabs(targetPos.y - mPosition.y))
            mDirection = (targetPos.x < mPosition.x) ? LEFT : RIGHT;
        else
            mDirection = (targetPos.y < mPosition.y) ? UP : DOWN;
        
        if (mAIStateTimer <= 0.0f)
        {
            mAIState = FIRING;
            mAIStateTimer = 0.25f;
            mIsAttacking = true;
            mWatcherDamageApplied = false;
        }
        break;
        
    case FIRING:
        mMovement = {0.0f, 0.0f};
        mAIStateTimer -= GetFrameTime();
        
        // Shoot projectile once at the start of FIRING state
        if (!mWatcherDamageApplied) {
            mShouldShoot = true;
            mWatcherDamageApplied = true;
        }
        
        if (mAIStateTimer <= 0.0f)
        {
            mIsAttacking = false;
            mAIState = COOLDOWN;
            mAIStateTimer = 1.2f;
        }
        break;
        
    case COOLDOWN:
        mMovement = {0.0f, 0.0f};
        mAIStateTimer -= GetFrameTime();
        if (mAIStateTimer <= 0.0f)
        {
            mAIState = IDLE;
        }
        break;
        
    default:
        break;
    }
}

void Entity::AIActivate(Entity *target)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander();
        break;

    case FOLLOWER:
        AIFollow(target);
        break;
    
    case FLYER:
        AIFly(target);
        break;

    case CHARGER:
        AICharger(target);
        break;
        
    case WATCHER:
        AIWatcher(target);
        break;
    
    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map, 
    Entity *collidableEntities, int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;
    
    if (mEntityType == NPC) AIActivate(player);

    resetColliderFlags();

    // Update attack system
    if (mAttackCooldown > 0.0f) mAttackCooldown -= deltaTime;
    if (mAttackDuration > 0.0f) 
    {
        mAttackDuration -= deltaTime;
        if (mAttackDuration <= 0.0f) mIsAttacking = false;
    }

    // Top-down movement: set velocity from movement in both X and Y
    mVelocity.x = mMovement.x * mSpeed;
    mVelocity.y = mMovement.y * mSpeed;

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    // Top-down movement: update position directly
    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionX(map);

    // here i using path to switch the texture
        if (mEntityType == PLAYER && mTextureType == ATLAS)
    {
        bool isMoving = GetLength(mMovement) > 0.01f;
        std::string basePath = "player/";
        std::string animFolder;
        std::string directionStr;
        
        // Determine animation folder
        if (mIsAttacking)
        {
            animFolder = "ATTACK 1/";
        }
        else if (isMoving)
        {
            animFolder = "RUN/";
        }
        else
        {
            animFolder = "IDLE/";
        }
        
        // Determine direction string
        switch (mDirection)
        {
            case UP: directionStr = "up"; break;
            case DOWN: directionStr = "down"; break;
            case LEFT: directionStr = "left"; break;
            case RIGHT: directionStr = "right"; break;
            default: directionStr = "down"; break;
        }
        
        // Construct full path
        std::string fullPath = basePath + animFolder;
        if (mIsAttacking)
        {
            fullPath += "attack1_" + directionStr + ".png";
        }
        else if (isMoving)
        {
            fullPath += "run_" + directionStr + ".png";
        }
        else
        {
            fullPath += "idle_" + directionStr + ".png";
        }
        
        static std::string lastPath = "";
        if (lastPath != fullPath)
        {
            if (mTexture.id != 0) UnloadTexture(mTexture);
            mTexture = LoadTexture(fullPath.c_str());
            lastPath = fullPath;
            // Reset animation when texture changes
            mCurrentFrameIndex = 0;
            mAnimationTime = 0.0f;
        }
    }
    
    // Animate sprite sheet (for top-down, always animate if ATLAS type)
    if (mTextureType == ATLAS) 
    {
        // Special case: Charger preparing charge locks to first frame (frame 0)
        if (mAIType == CHARGER && mAIState == PREPARING_CHARGE) {
            mCurrentFrameIndex = 0;
        } else {
            animate(deltaTime);
        }
    }
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    if (mEntityType == PROJECTILE) {
        DrawCircle(mPosition.x, mPosition.y, mScale.x / 2.0f, RED);
        return;
    }

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            // Whole texture (UV coordinates)
            textureArea = {
                // top-left corner
                0.0f, 0.0f,

                // bottom-right corner (of texture)
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture, 
                mAnimationIndices[mCurrentFrameIndex], 
                (int)mSpriteSheetDimensions.y,  // rows
                (int)mSpriteSheetDimensions.x    // cols
            );
        
        default: break;
    }

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    // Visual indicator for charging state - blinking effect
    Color tint = WHITE;
    if (mAIState == PREPARING_CHARGE && mEntityType == NPC) {
        float blinkSpeed = 8.0f;
        float time = GetTime();
        float offset = mPosition.x + mPosition.y;
        int blinkState = (int)((time + offset * 0.01f) * blinkSpeed) % 2;
        tint = (blinkState == 0) ? RED : WHITE;
    }
    else if (mAIType == WATCHER && (mAIState == AIMING || mAIState == FIRING)) {
        tint = (mAIState == AIMING) ? ORANGE : SKYBLUE;
    }

    // Render the texture on screen
    DrawTexturePro(
        mTexture, 
        textureArea, destinationArea, originOffset,
        mAngle, tint
    );

}