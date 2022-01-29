typedef struct StageData
{
    int level;
    Vector2 initialPlayerPosition;
    Vector2 goalPosition;
    cute_tiled_map_t *map;
} StageData;

StageData LoadStage(int level)
{
    StageData stage;
    stage.level = level;
    char *stagePath = (char *)malloc(21 * sizeof(char));
    sprintf(stagePath, "resources/level%d.json", level);
    stage.map = cute_tiled_load_map_from_file(stagePath, NULL);
    free(stagePath);
    cute_tiled_layer_t *layer;
    for (layer = stage.map->layers; layer != NULL; layer = layer->next)
    {
        cute_tiled_object_t *object;
        for (object = layer->objects; object != NULL; object = object->next)
        {

            if (object->ellipse)
            {
                stage.goalPosition = (Vector2){object->x + GOAL_RADIUS / 2, object->y + GOAL_RADIUS / 2};
            }
            else
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
                body->restitution = 1.0f;
            }
        }
    }
    return stage;
}

void FreeStage(StageData *stage)
{
    ResetPhysics();
    cute_tiled_free_map(stage->map);
}