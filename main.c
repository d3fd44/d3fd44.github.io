#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define WAVE_COUNT 5
#define SAMPLES    1024

typedef struct {
    Vector2 points[SAMPLES];
} wave;

wave waves[WAVE_COUNT] = { 0 };

float periods[WAVE_COUNT] = { -140, 95, -110, 160, -75 };  // negative values for negative rotation, not "negative period" actually
float frequencies[WAVE_COUNT] = { 1.3f, 2.1f, 1.7f, 2.6f, 1.1f };
float amplitudes[WAVE_COUNT] = { 42, 18, 70, 46, 62 };
float phase_offsets[WAVE_COUNT] = { 0.4f, 1.8f, 3.6f, 5.0f, 2.2f };

float scale_factor = 0;

#define SCALABLE_AMPLITUDE(A) ((A) * scale_factor)
#define EASE_OUT_DURATION     2
#define EASE_OUT_STRENGHT     250
#define K                     EASE_OUT_STRENGHT

double st;

Color MIDNIGHT = { 11, 18, 32, 255 };
Color wave_colors[WAVE_COUNT] = {
    { 23, 40, 61, 200 },    // Navy
    { 66, 205, 224, 120 },  // Cyan
    { 66, 205, 224, 160 },  // Cyan
    { 255, 152, 92, 130 },  // Orange
    { 23, 40, 61, 220 }     // Navy
};

KeyboardKey konami_code[] = { KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_B, KEY_A };
double      last_keypress = 0;
int         cheating_progress = 0;

#define CHEAT_TIMEOUT   2.0
#define KONAMI_CODE_LEN (sizeof konami_code / sizeof konami_code[0])

bool cheated = false;

bool is_cheating(double t)
{
    KeyboardKey key = GetKeyPressed();
    bool        time_out = cheating_progress > 0 && t - last_keypress > 2;

    while (key != 0)
    {
        if (key == konami_code[cheating_progress])
        {
            cheating_progress++;
            last_keypress = t;

            if (cheating_progress == KONAMI_CODE_LEN)
            {
                cheating_progress = 0;
                return true;
            }
        }
        else
        {
            cheating_progress = key == konami_code[0] ? 1 : 0;
            last_keypress = t;
        }

        if (time_out)
            cheating_progress = 0;

        key = GetKeyPressed();
    }
    return false;
}

void update_frame(void)
{
    float t = GetTime();

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    float sea_level = (double)screen_height / 2;  // you need to join the dark side to control physics laws

    if (is_cheating(t))
    {
        cheated = !cheated;
    }

    BeginDrawing();
    ClearBackground(MIDNIGHT);

    if (scale_factor < 1)
    {
        float elapsed = Clamp((t - st) / EASE_OUT_DURATION, 0, 1);
        scale_factor = log(1 + (K * elapsed)) / log(1 + K);
    }

    for (int w = 0; w < WAVE_COUNT; w++)
    {
        for (int i = 0; i < SAMPLES; i++)
        {
            float initial_phase = 2 * PI * ((double)i / (SAMPLES - 1)) * frequencies[w];
            float angular_velocity = 2 * PI / periods[w];

            waves[w].points[i].x = (((double)i / (SAMPLES - 1)) * screen_width);
            waves[w].points[i].y
              = sea_level - (SCALABLE_AMPLITUDE(amplitudes[w]) * sin(initial_phase - angular_velocity * t + phase_offsets[w]));
        }
        DrawSplineLinear(waves[w].points, SAMPLES, 2.5, wave_colors[w]);
    }
    EndDrawing();
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(800, 600, "");
    SetWindowMinSize(600, 400);

    st = GetTime();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
        update_frame();
#endif

    CloseWindow();
    return 0;
}
