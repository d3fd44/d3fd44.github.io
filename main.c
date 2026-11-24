#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define WAVE_COUNT 5
#define SAMPLES    1024

#define EXPAND     1
#define COLLAPSE   2
#define IDLE       3

typedef struct {
    Vector2 points[SAMPLES];
} wave;

wave waves[WAVE_COUNT] = { 0 };

float periods[WAVE_COUNT] = { -140, 45, -10, 160, -75 };  // negative values for negative rotation, not "negative period" actually
float frequencies[WAVE_COUNT] = { 1.3f, 2.1f, 1.7f, 2.6f, 1.1f };
float amplitudes[WAVE_COUNT] = { 42, 18, 70, 46, 62 };
float phase_offsets[WAVE_COUNT] = { 0.4f, 1.8f, 3.6f, 5.0f, 2.2f };

float scale_factor = 0;

#define REF_WIDTH             1920.0
#define DAMP_VIEWPORT         Clamp(GetScreenWidth() / REF_WIDTH, 0.35, 1)

#define SCALABLE_AMPLITUDE(A) ((A) * scale_factor)
#define EASE_OUT_DURATION     2
#define EASE_OUT_STRENGTH     250
#define K                     EASE_OUT_STRENGTH

int  direction = EXPAND;
bool show_combined = false;

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

bool is_cheating(double t)
{
    KeyboardKey key = GetKeyPressed();

    if (cheating_progress > 0 && t - last_keypress > 2)
        cheating_progress = 0;

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

        key = GetKeyPressed();
    }
    return false;
}

void update_frame(void)
{
    double t = GetTime();

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    Vector2 combined_wave[SAMPLES] = { 0 };

    float sea_level = (double)screen_height / 2;  // you need to join the dark side to control physics laws

    if (is_cheating(t) && direction == IDLE)
    {
        direction = COLLAPSE;
        st = t;
    }

    BeginDrawing();
    ClearBackground(MIDNIGHT);

    if (direction == EXPAND)
    {
        float elapsed = Clamp((t - st) / EASE_OUT_DURATION, 0, 1);
        scale_factor = log(1 + (K * elapsed)) / log(1 + K);

        if (elapsed >= 1)
            direction = IDLE;
    }
    else if (direction == COLLAPSE)
    {
        float elapsed = Clamp((t - st) / EASE_OUT_DURATION * 4, 0, 1);  // collapses 4 times faster
        scale_factor = 1 - log(1 + (K * elapsed)) / log(1 + K);

        if (elapsed >= 1)
        {
            show_combined = !show_combined;
            direction = EXPAND;
            st = t;
        }
    }

    for (int w = 0; w < WAVE_COUNT; w++)
    {
        for (int i = 0; i < SAMPLES; i++)
        {
            float initial_phase = 2 * PI * ((double)i / (SAMPLES - 1)) * frequencies[w];
            float angular_velocity = 2 * PI / periods[w];

            double vertical_displacement
              = SCALABLE_AMPLITUDE(amplitudes[w]) * sin(initial_phase - angular_velocity * t + phase_offsets[w]) * DAMP_VIEWPORT;
            double horizontal_displacement = ((double)i / (SAMPLES - 1)) * screen_width;

            waves[w].points[i].x = horizontal_displacement;
            waves[w].points[i].y = sea_level - vertical_displacement;

            combined_wave[i].y -= vertical_displacement;

            if (w > 0)
                continue;
            else
            {
                combined_wave[i].x = horizontal_displacement;
                combined_wave[i].y += sea_level;
            }
        }
        if (!show_combined)
            DrawSplineLinear(waves[w].points, SAMPLES, 2.5, wave_colors[w]);
    }
    if (show_combined)
        DrawSplineLinear(combined_wave, SAMPLES, 4.5, RED);

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
