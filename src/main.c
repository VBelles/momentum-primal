/*******************************************************************************************
 *
 *   raylib [core] example - Basic window (adapted for HTML5 platform)
 *
 *   This example is prepared to compile for PLATFORM_WEB, PLATFORM_DESKTOP and PLATFORM_RPI
 *   As you will notice, code structure is slightly diferent to the other examples...
 *   To compile it for PLATFORM_WEB just uncomment #define PLATFORM_WEB at beginning
 *
 *   This example has been created using raylib 1.3 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
 *
 *   Copyright (c) 2015 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

#define CUTE_TILED_IMPLEMENTATION
#include "cute_tiled.h"

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 896;
int screenHeight = 504;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(); // Update and Draw one frame
void CreateObstacles();
void DrawMouseWidget();
int bouncingDemo();
//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // Initialize physics and default physics bodies
    InitPhysics();

    // Load level physics
    cute_tiled_map_t *map = cute_tiled_load_map_from_file("resources/level1.json", NULL);
    float ratio = screenWidth / (float)map->width;

    cute_tiled_layer_t *layer;
    for (layer = map->layers; layer != NULL; layer = layer->next)
    {
        cute_tiled_object_t *object;
        for (object = layer->objects; object != NULL; object = object->next)
        {
            PhysicsBody body = CreatePhysicsBodyRectangle((Vector2){0, 0}, object->width, object->height, 10.0f);

            if (object->rotation != 0)
            {
                // body->position.x -= object->width / 2.0f;
            }

            SetPhysicsBodyRotation(body, object->rotation * DEG2RAD);

            body->position.x += object->x + object->width / 2.0f;
            body->position.y += object->y + object->height / 2.0f;
            body->enabled = false;
        }
    }

    InitWindow(screenWidth, screenHeight, "Momentun Primal");

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

    // Physics body creation inputs
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        CreatePhysicsBodyPolygon(GetMousePosition(), (float)GetRandomValue(20, 80), GetRandomValue(3, 8), 10);
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        CreatePhysicsBodyCircle(GetMousePosition(), (float)GetRandomValue(10, 45), 10);

    // Destroy falling physics bodies
    int bodiesCount = GetPhysicsBodiesCount();
    for (int i = bodiesCount - 1; i >= 0; i--)
    {
        PhysicsBody body = GetPhysicsBody(i);
        if (body != NULL && (body->position.y > screenHeight * 2))
            DestroyPhysicsBody(body);
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(BLACK);

    DrawFPS(screenWidth - 90, screenHeight - 30);

    // Draw created physics bodies
    bodiesCount = GetPhysicsBodiesCount();
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

                DrawLineV(vertexA, vertexB, GREEN); // Draw a line between two vertex positions
            }
        }
    }

    DrawText("Left mouse button to create a polygon", 10, 10, 10, WHITE);
    DrawText("Right mouse button to create a circle", 10, 25, 10, WHITE);
    EndDrawing();

    //----------------------------------------------------------------------------------
}

int bouncingDemo(void){
    
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    
    InitWindow(screenWidth, screenHeight, "Momentum Primal");
    
    // Initialize physics and default physics bodies
    InitPhysics();
    
    SetPhysicsGravity(0, 0);
    
    Vector2 initialPosition = {(float)screenWidth/2.0f, (float)screenHeight * 0.670f};//screen goes from top to bottom
    //PhysicsBody ball = CreateBall((Vector2)initialPosition, (float)screenHeight*0.03f, (float)1f);
    PhysicsBody ball = CreatePhysicsBodyCircle(initialPosition, screenHeight*0.03f, 0.1f);
    
    
    ball->staticFriction = 0.0f;           // Friction when the body has not movement (0 to 1)
    ball->dynamicFriction = 0.0f;          // Friction when the body has movement (0 to 1)
    ball->restitution = 1.0f;              // Restitution coefficient of the body (0 to 1)
    ball->useGravity = false;                // Apply gravity force to dynamics
    ball->freezeOrient = false;              // Physics rotation constraint

    CreateObstacles(); 
    
    
    HideCursor();
    
    SetPhysicsTimeStep(1.0/60.0/100*1000);//0.16ms
    //SetPhysicsTimeStep(GetFrameTime()/100*1000);
    SetTargetFPS(60);
    
    while(!WindowShouldClose()){
        
        UpdatePhysics();
        
        
        BeginDrawing();
        
            ClearBackground((Color){0, 0, 0, 255});
            
            DrawFPS(screenWidth - 90, screenHeight - 30);            
            
            Vector2 mousePos = GetMousePosition();
            DrawMouseWidget(mousePos, RAYWHITE);

            Vector2 velocity = ball->velocity;
            float speed = Vector2Length(velocity);
            if(speed == 0)
            {
                if(IsMouseButtonDown(0))
                {  
                    //feedback - draw arrow
                    
                }

                if (IsMouseButtonReleased(0))
                {
                    //calculate direction ball - mouse and power (distance)
                    //shoot
                    
                    Vector2 directionVector = {(ball->position.x - mousePos.x), (ball->position.y - mousePos.y)};
                    float length = Vector2Length(directionVector);
                    
                    float launchSpeed =  Remap(length, 0, 400.0f, 0, 2.0f);
                    launchSpeed = Clamp(launchSpeed, 0, 0.5f);
                    
                    directionVector = Vector2Normalize(directionVector);
                    
                    directionVector = Vector2Scale(directionVector, launchSpeed);;

                    ball->velocity = directionVector;
                }
            }
            else
            {
                printf("starting frame speed = %f\n", speed);
                if(speed <= 0.005f)
                {
                    ball->velocity = (Vector2){0, 0};
                }
                else
                {
                    Vector2 oppositeDirection = velocity;
                    oppositeDirection.x = -oppositeDirection.x;
                    oppositeDirection.y = -oppositeDirection.y;
                    oppositeDirection = Vector2Normalize(oppositeDirection);
                    float deceleration = GetFrameTime() * 0.06f;//a * t
                    oppositeDirection = Vector2Scale(oppositeDirection, deceleration); 

                    ball->velocity = Vector2Add(ball->velocity, oppositeDirection);
                }

                speed = Vector2Length(ball->velocity);
                printf("Ending frame speed = %f\n", speed);
            }
            
            // Draw created physics bodies
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

                        int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
                        Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

                        DrawLineV(vertexA, vertexB, GREEN);     // Draw a line between two vertex positions
                    }
                }
            }
        
        
        EndDrawing();
    }//end game loop
    
     // De-Initialization
    //--------------------------------------------------------------------------------------
    ClosePhysics();       // Unitialize physics

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void DrawMouseWidget(Vector2 pos, Color color)
{   
    DrawPixel(pos.x+1, pos.y, color);
    DrawPixel(pos.x+2, pos.y, color);
    DrawPixel(pos.x+3, pos.y, color);
    DrawPixel(pos.x+4, pos.y, color);
    
    DrawPixel(pos.x-1, pos.y, color);
    DrawPixel(pos.x-2, pos.y, color);
    DrawPixel(pos.x-3, pos.y, color);
    DrawPixel(pos.x-4, pos.y, color);
    
    DrawPixel(pos.x, pos.y+1, color);
    DrawPixel(pos.x, pos.y+2, color);
    DrawPixel(pos.x, pos.y+3, color);
    DrawPixel(pos.x, pos.y+4, color);
    
    DrawPixel(pos.x, pos.y-1, color);
    DrawPixel(pos.x, pos.y-2, color);
    DrawPixel(pos.x, pos.y-3, color);
    DrawPixel(pos.x, pos.y-4, color);
}

void CreateObstacles()
{
    //here we should import the stage  
    
    float density = 10;
    float thickness = screenWidth * 0.1f;
    float restitution = 1.0f;
    
    PhysicsBody wallBottom = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2.0f, (float)screenHeight }, screenWidth, thickness, density);
    wallBottom->enabled = false;         // Disable body state to convert it to static (no dynamics, but collisions)
    wallBottom->restitution = restitution;
    
    PhysicsBody wallTop = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2.0f, 0 }, screenWidth, thickness, density);
    wallTop->enabled = false;
    wallTop->restitution = restitution;
    
    PhysicsBody wallLeft = CreatePhysicsBodyRectangle((Vector2){ 0, screenHeight/2.0f }, thickness, screenHeight, density);
    wallLeft->enabled = false;
    wallLeft->restitution = restitution;
    
    PhysicsBody wallRight = CreatePhysicsBodyRectangle((Vector2){ screenWidth, screenHeight/2.0f }, thickness, screenHeight, density);
    wallRight->enabled = false;
    wallRight->restitution = restitution;
}