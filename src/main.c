#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define CUTE_TILED_IMPLEMENTATION
#include "cute_tiled.h"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"

#define GOAL_RADIUS 50.0f
#define PLAYER_RADIUS 15.0f

#include "stage_loader.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 896;
int screenHeight = 504;

StageData stage;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(); // Update and Draw one frame
void DrawMouseWidget(Vector2 pos, Color color);
void DrawBodies();
void UpdateBall();

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Momentum Primal");

    HideCursor();

    stage = LoadStage(1);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    FreeStage(&stage);
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame()
{

    // Update
    //----------------------------------------------------------------------------------
    UpdateBall();
    if (IsKeyPressed(KEY_R))
    {
        FreeStage(&stage);
        stage = LoadStage(stage.level);
    }
    if (IsKeyPressed(KEY_N))
    {
        FreeStage(&stage);
        stage = LoadStage(stage.level + 1);
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawFPS(screenWidth - 90, screenHeight - 30);

    DrawBodies();

    if (stage.goalReached)
    {
        DrawText("That was close!", screenWidth / 2, screenHeight / 2, 16, DARKGRAY);
    }

    DrawText("Press left mouse button to drag. Release to launch.", 10, 10, 12, DARKGRAY);
    DrawText("Press R to restart stage", 10, 25, 12, DARKGRAY);
    DrawText("Press N to jump to next stage", 10, 40, 12, DARKGRAY);

    if (stage.victory)
    {
        DrawText("VICTORY! NEW LEVELS IN THE INCOMING DLC!", 100, screenHeight / 2 - 20, 30, DARKGRAY);
    }
    else if (stage.level == 1)
    {
        DrawText("Reach the green ball", 100, screenHeight - 120, 20, DARKGRAY);
    }

    Vector2 mousePos = GetMousePosition();
    DrawMouseWidget(mousePos, DARKGRAY);

    EndDrawing();

    //----------------------------------------------------------------------------------
}

void UpdateBall()
{
    float dt = GetFrameTime();
    Vector2 ballPosition = {stage.ball.p.x, stage.ball.p.y};
    Vector2 mousePos = GetMousePosition();

    float speed = Vector2Length(stage.ballVelocity);
    if (speed == 0)
    {
        Vector2 directionVector = Vector2Subtract(ballPosition, mousePos);
        float maxDistanceLaunch = 100.0f;
        float lengthDirectionVector = Vector2Length(directionVector);

        if (IsMouseButtonDown(0))
        {
            Vector2 launchVector = directionVector;
            if (lengthDirectionVector > maxDistanceLaunch)
            {
                launchVector = Vector2Scale(Vector2Normalize(launchVector), maxDistanceLaunch);
            }

            Vector2 endPosition = Vector2Add(ballPosition, launchVector);
            // void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
            float greenComponent = Remap(Vector2Length(launchVector), 0, maxDistanceLaunch, 255, 0);
            Color launchColor = (Color){255, greenComponent, 0, 255};
            DrawLine(ballPosition.x, ballPosition.y, endPosition.x, endPosition.y, launchColor);
        }

        else if (IsMouseButtonReleased(0))
        {
            // calculate direction ball - mouse and power (distance)
            // shoot

            float launchSpeed = Remap(lengthDirectionVector, 0, maxDistanceLaunch, 0, 300);
            launchSpeed = Clamp(launchSpeed, 0, 300);

            directionVector = Vector2Normalize(directionVector);

            directionVector = Vector2Scale(directionVector, launchSpeed);

            stage.ballVelocity = directionVector;
            stage.launched = true;
        }
    }
    else
    {
        if (speed <= 0.5f)
        {
            stage.ballVelocity = (Vector2){0, 0};
        }
        else
        {
            Vector2 oppositeDirection = Vector2Negate(stage.ballVelocity);
            oppositeDirection = Vector2Normalize(oppositeDirection);
            float deceleration = dt * 40.0f; // a * t
            oppositeDirection = Vector2Scale(oppositeDirection, deceleration);

            stage.ballVelocity = Vector2Add(stage.ballVelocity, oppositeDirection);
        }
    }

    speed = Vector2Length(stage.ballVelocity);

    // Goal condition
    if (speed == 0 && Vector2Distance(ballPosition, stage.goalPosition) < GOAL_RADIUS)
    {
        stage.goalReached = true;
        stage.goalReachedAt = GetTime();
    }

    if (stage.launched)
    {
        if (speed == 0)
        {
            if (Vector2Distance(ballPosition, stage.goalPosition) < GOAL_RADIUS)
            {
                stage.goalReached = true;
                FreeStage(&stage);
                stage = LoadStage(stage.level + 1);
            }
            else
            {
                stage.ball.p.x = stage.initialPlayerPosition.x;
                stage.ball.p.y = stage.initialPlayerPosition.y;
            }
            stage.launched = false;
        }
    }

    stage.ball.p.x += stage.ballVelocity.x * dt;
    stage.ball.p.y += stage.ballVelocity.y * dt;

    for (int i = 0; i < stage.obstaclesCount; i++)
    {
        c2AABB obstacle = stage.obstacles[i];
        c2Manifold manifold;
        c2CircletoAABBManifold(stage.ball, obstacle, &manifold);
        if (manifold.count > 0)
        {
            if (manifold.n.x != 0)
            {
                stage.ballVelocity.x = -stage.ballVelocity.x;
                stage.ball.p.x += stage.ballVelocity.x * dt; // Easy, dirty and fake depenetration
            }
            if (manifold.n.y != 0)
            {
                stage.ballVelocity.y = -stage.ballVelocity.y;
                stage.ball.p.y += stage.ballVelocity.y * dt;
            }
            break;
        }
    }
}

void DrawBodies()
{

    DrawCircle(stage.goalPosition.x, stage.goalPosition.y, GOAL_RADIUS, GREEN);
    DrawCircleLines(stage.goalPosition.x, stage.goalPosition.y, GOAL_RADIUS, DARKGRAY);

    DrawCircle(stage.ball.p.x, stage.ball.p.y, stage.ball.r, GRAY);
    DrawCircleLines(stage.ball.p.x, stage.ball.p.y, stage.ball.r, DARKGRAY);

    for (int i = 0; i < stage.obstaclesCount; i++)
    {
        c2AABB *obstacle = &stage.obstacles[i];
        DrawRectangleLines(obstacle->min.x, obstacle->min.y, obstacle->max.x - obstacle->min.x,
                           obstacle->max.y - obstacle->min.y, DARKGRAY);
    }
}

void DrawMouseWidget(Vector2 pos, Color color)
{
    DrawPixel(pos.x + 1, pos.y, color);
    DrawPixel(pos.x + 2, pos.y, color);
    DrawPixel(pos.x + 3, pos.y, color);
    DrawPixel(pos.x + 4, pos.y, color);

    DrawPixel(pos.x - 1, pos.y, color);
    DrawPixel(pos.x - 2, pos.y, color);
    DrawPixel(pos.x - 3, pos.y, color);
    DrawPixel(pos.x - 4, pos.y, color);

    DrawPixel(pos.x, pos.y + 1, color);
    DrawPixel(pos.x, pos.y + 2, color);
    DrawPixel(pos.x, pos.y + 3, color);
    DrawPixel(pos.x, pos.y + 4, color);

    DrawPixel(pos.x, pos.y - 1, color);
    DrawPixel(pos.x, pos.y - 2, color);
    DrawPixel(pos.x, pos.y - 3, color);
    DrawPixel(pos.x, pos.y - 4, color);
}
