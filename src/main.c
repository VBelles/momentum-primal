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