#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

#define CUTE_TILED_IMPLEMENTATION
#include "cute_tiled.h"

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

    // Initialize physics and default physics bodies
    InitPhysics();
    SetPhysicsTimeStep(1.0 / 60.0 / 100 * 1000); // 0.16ms

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
    ClosePhysics(); // Unitialize physics
    CloseWindow();  // Close window and OpenGL context
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
    UpdatePhysics(); // Update physics system
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
    else if(stage.level == 1){
        DrawText("Reach the green ball", 100, screenHeight - 120, 20, DARKGRAY);
    }

    Vector2 mousePos = GetMousePosition();
    DrawMouseWidget(mousePos, DARKGRAY);

    EndDrawing();

    //----------------------------------------------------------------------------------
}

void UpdateBall()
{
    PhysicsBody ball = stage.ball;
    Vector2 mousePos = GetMousePosition();

    Vector2 ballPosition = ball->position;
    Vector2 velocity = ball->velocity;
    float speed = Vector2Length(velocity);
    if (speed == 0)
    {
        Vector2 directionVector = {(ball->position.x - mousePos.x), (ball->position.y - mousePos.y)};
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

            float launchSpeed = Remap(lengthDirectionVector, 0, maxDistanceLaunch, 0, 0.5f);
            launchSpeed = Clamp(launchSpeed, 0, 0.5f);

            directionVector = Vector2Normalize(directionVector);

            directionVector = Vector2Scale(directionVector, launchSpeed);

            ball->velocity = directionVector;
            stage.launched = true;
        }
    }
    else
    {
        if (speed <= 0.005f)
        {
            ball->velocity = (Vector2){0, 0};
        }
        else
        {
            Vector2 oppositeDirection = velocity;
            oppositeDirection.x = -oppositeDirection.x;
            oppositeDirection.y = -oppositeDirection.y;
            oppositeDirection = Vector2Normalize(oppositeDirection);
            float deceleration = GetFrameTime() * 0.06f; // a * t
            oppositeDirection = Vector2Scale(oppositeDirection, deceleration);

            ball->velocity = Vector2Add(ball->velocity, oppositeDirection);
        }
    }

    speed = Vector2Length(ball->velocity);

    // Goal condition
    if (speed == 0 && Vector2Distance(ball->position, stage.goalPosition) < GOAL_RADIUS)
    {
        stage.goalReached = true;
        stage.goalReachedAt = GetTime();
    }

    // Load next stage 1 second after goal condition
    /*if (goalReached && GetTime() - goalReachedAt > 1)
    {
        goalReached = false;
        FreeStage(&stage);
        LoadStage(stage.level + 1);
    }*/
    if (stage.launched)
    {
        if (speed == 0)
        {
            if (Vector2Distance(ball->position, stage.goalPosition) < GOAL_RADIUS)
            {
                stage.goalReached = true;
                FreeStage(&stage);
                stage = LoadStage(stage.level + 1);
            }
            else
            {
                ball->position = stage.initialPlayerPosition;
            }
            stage.launched = false;
        }
    }
}

void DrawBodies()
{

    DrawCircle(stage.goalPosition.x, stage.goalPosition.y, GOAL_RADIUS, GREEN);
    DrawCircleLines(stage.goalPosition.x, stage.goalPosition.y, GOAL_RADIUS, DARKGRAY);

    DrawCircle(stage.ball->position.x, stage.ball->position.y, PLAYER_RADIUS, GRAY);

    int bodiesCount = GetPhysicsBodiesCount();
    for (int i = 0; i < bodiesCount; i++)
    {
        PhysicsBody body = GetPhysicsBody(i);

        if (body != NULL)
        {
            int vertexCount = GetPhysicsShapeVerticesCount(i);
            for (int j = 0; j < vertexCount; j++)
            {
                // Get physics bodies shape vertices to draw lines
                // Note: GetPhysicsShapeVertex() already calculates rotation transformations
                Vector2 vertexA = GetPhysicsShapeVertex(body, j);

                int jj = (((j + 1) < vertexCount) ? (j + 1) : 0); // Get next vertex or first to close the shape
                Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

                DrawLineV(vertexA, vertexB, DARKGRAY); // Draw a line between two vertex positions
            }
        }
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
