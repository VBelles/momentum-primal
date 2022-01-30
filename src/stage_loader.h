#ifndef STAGE_LOADER_H_
#define STAGE_LOADER_H_

typedef struct StageData
{
    int level;
    Vector2 initialPlayerPosition;
    Vector2 goalPosition;
    cute_tiled_map_t *map;

    bool goalReached;
    double goalReachedAt;
    bool launched;

    c2Circle ball;
    Vector2 ballVelocity;
    c2AABB obstacles[100];
    int obstaclesCount;

    bool victory;

} StageData;

StageData LoadStage(int level)
{
    StageData stage = {0};

    if (level >= 4)
    {
        stage.victory = true;
        level = 1;
    }

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
            if (object->property_count > 0) // Player
            {
                stage.initialPlayerPosition.x = object->x;
                stage.initialPlayerPosition.y = object->y;
            }
            else if (object->ellipse) // Goal
            {
                stage.goalPosition = (Vector2){object->x + GOAL_RADIUS / 2, object->y + GOAL_RADIUS / 2};
            }
            else // Obstacles
            {
                c2AABB *obstacle = &stage.obstacles[stage.obstaclesCount++];
                obstacle->min = (c2v){object->x, object->y};
                obstacle->max = (c2v){object->x + object->width, object->y + object->height};
            }
        }
    }

    // Create ball
    stage.ball.r = PLAYER_RADIUS;
    stage.ball.p = (c2v){stage.initialPlayerPosition.x, stage.initialPlayerPosition.y};

    return stage;
}

void FreeStage(StageData *stage)
{
    cute_tiled_free_map(stage->map);
}

#endif