#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>

#define DEFAULT_WINDOW_WIDTH  800
#define DEFAULT_WINDOW_HEIGHT 600
#define ZOOM_SPEED            20
#define MIN_ZOOM              0.5
#define MAX_ZOOM              20
#define PAN_SPEED             100
#define FPS_CAP               60
#define DEFAULT_FG_COLOR      0x282828FF
#define DEFAULT_BG_COLOR      0xFBF1C7FF
#define STROKE_RADIUS         3
#define INIT_POINTS_CAPACITY  128
#define INIT_STROKES_CAPACITY 32

#define COLOR_FROM_U32(c)                                                     \
    (Color)                                                                   \
    {                                                                         \
        (unsigned char)((c >> 24) & 0xFF), (unsigned char)((c >> 16) & 0xFF), \
            (unsigned char)((c >> 8) & 0xFF), (unsigned char)(c & 0xFF)       \
    }

const static Color DEFAULT_BG           = COLOR_FROM_U32(DEFAULT_BG_COLOR);
const static Color DEFAULT_FG           = COLOR_FROM_U32(DEFAULT_FG_COLOR);
const static unsigned char CURSOR_PNG[] = {
#embed "cursor.png"
};

typedef struct Stroke {
    Vector2* points;
    int n_points;
    int capacity_points;

    float radius;
    Color color;
} Stroke;

typedef struct WorldState {
    Stroke* strokes;
    int n_strokes;
    int capacity_strokes;

    bool drawing;
} WorldState;

void unloadWorldState(WorldState* w)
{
    for (int i = 0; i < w->n_strokes; ++i) {
        Stroke* s = &w->strokes[i];
        free(s->points);
        printf("Freed stroke: id=%d\n", i);
    }

    free(w->strokes);
    printf("Freed world\n");
}

void new_stroke(WorldState* w)
{
    printf("Started new stroke: id=%d\n", w->n_strokes);

    if (w->n_strokes >= w->capacity_strokes) {
        w->capacity_strokes *= 2;
        w->strokes = realloc(w->strokes, sizeof(Stroke) * w->capacity_strokes);
        printf("Grew strokes: new_size=%d\n", w->capacity_strokes);
    }

    w->strokes[w->n_strokes++] = (Stroke) {
        .points          = malloc(sizeof(Vector2) * INIT_POINTS_CAPACITY),
        .n_points        = 0,
        .capacity_points = INIT_POINTS_CAPACITY,
        .radius          = STROKE_RADIUS,
        .color           = DEFAULT_FG,
    };
}

void addPointToStroke(WorldState* w, Vector2 p)
{
    Stroke* s = &w->strokes[w->n_strokes - 1];

    if (s->n_points >= s->capacity_points) {
        s->capacity_points *= 2;
        s->points = realloc(s->points, sizeof(Vector2) * s->capacity_points);
        printf("Grew points: id=%d, new_size=%d\n",
            w->n_strokes - 1,
            s->capacity_points);
    }

    s->points[s->n_points++] = p;
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetTargetFPS(FPS_CAP);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "scratchrat");
    Camera2D c = {
        .zoom = 1.0,
    };

    Image cursorImage       = LoadImageFromMemory(".png", CURSOR_PNG, sizeof(CURSOR_PNG));
    Texture2D cursorTexture = LoadTextureFromImage(cursorImage);

    RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    int lastScreenWidth    = GetScreenWidth();
    int lastScreenHeight   = GetScreenHeight();

    WorldState w = {
        .strokes          = malloc(INIT_STROKES_CAPACITY * sizeof(Stroke)),
        .n_strokes        = 0,
        .capacity_strokes = INIT_STROKES_CAPACITY,
        .drawing          = false,
    };

    HideCursor();

    while (!WindowShouldClose()) {
        float dt      = GetFrameTime();
        Vector2 mouse = GetMousePosition();

        // apply render target resize
        if (GetScreenWidth() != lastScreenWidth || GetScreenHeight() != lastScreenHeight) {
            UnloadRenderTexture(target);
            target           = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
            lastScreenWidth  = GetScreenWidth();
            lastScreenHeight = GetScreenHeight();
        }

        // apply zoom
        float old_zoom = c.zoom;
        c.zoom += GetMouseWheelMoveV().y * dt * ZOOM_SPEED;
        c.zoom = Clamp(c.zoom, MIN_ZOOM, MAX_ZOOM);

        Vector2 world_before = Vector2Add(Vector2Scale(mouse, 1.0f / old_zoom), c.target);
        Vector2 world_after  = Vector2Add(Vector2Scale(mouse, 1.0f / c.zoom), c.target);
        c.target             = Vector2Add(c.target, Vector2Subtract(world_before, world_after));

        // apply pan
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 m = GetMouseDelta();
            m         = Vector2Scale(m, 1 / c.zoom);
            c.target  = Vector2Subtract(c.target, m);
        }

        // apply draw
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (!w.drawing) {
                new_stroke(&w);
                w.drawing = true;
            }
            Vector2 point_pos = GetScreenToWorld2D(mouse, c);
            addPointToStroke(&w, point_pos);
        }

        // quit drawing
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            w.drawing = false;
        }

        // check undo
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)) {
            if (w.n_strokes >= 1) {
                printf("Undid stroke: id=%d\n", w.n_strokes - 1);
                w.n_strokes -= 1;
            }
        }

        BeginTextureMode(target);
        ClearBackground(DEFAULT_BG);
        DrawTextureV(cursorTexture, mouse, WHITE);

        BeginMode2D(c);

        for (int i = 0; i < w.n_strokes; ++i) {
            Stroke s = w.strokes[i];
            DrawSplineBasis(s.points, s.n_points, s.radius, s.color);
        }

        EndMode2D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(PINK);
        DrawTextureRec(
            target.texture,
            (Rectangle) {
                0, 0, (float)target.texture.width, -(float)target.texture.height },
            (Vector2) { 0, 0 },
            WHITE);
        EndDrawing();
    }

    unloadWorldState(&w);
    CloseWindow();
}
