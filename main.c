#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define WAVE_COUNT 6

#define EXPAND     1
#define COLLAPSE   2
#define IDLE       3

float scale_factor = 0;

#define REF_WIDTH             1920.0
#define WIDTH_THRESHOLD       0.8
#define SPACING               (REF_WIDTH / 1023)  // distance between two points in pixels (1024 points)
#define MAX_SAMPLES           4096

#define SCALABLE_AMPLITUDE(A) ((A) * scale_factor)
#define EASE_OUT_DURATION     2
#define EASE_OUT_STRENGTH     250

int  direction = EXPAND;
bool show_combined = false;

double st;

typedef struct {
    Vector2 points[MAX_SAMPLES];
} wave;

static wave    waves[WAVE_COUNT] = { 0 };
static Vector2 combined_wave[MAX_SAMPLES] = { 0 };

float periods[WAVE_COUNT] = { -170, 65, -10, 160, -75, 18 };  // negative values for negative rotation
float frequencies[WAVE_COUNT] = { 0.9f, 2.0f, 0.7f, 0.6f, 1.1f, 3.1f };
float amplitudes[WAVE_COUNT] = { 60, 20, 50, 34, 56, 12 };
float phase_offsets[WAVE_COUNT] = { 0.3f, 1.1f, 1.9f, 3.0f, 4.2f, 5.3f };

Color MIDNIGHT = { 11, 18, 32, 255 };
Color wave_colors[WAVE_COUNT] = {
    { 23, 40, 61, 170 },    // Navy
    { 66, 205, 224, 90 },   // Cyan
    { 66, 205, 224, 160 },  // Cyan
    { 255, 152, 92, 80 },   // Orange
    { 23, 40, 61, 215 },    // Navy
    { 170, 235, 240, 65 },  // Pale Cyan
};

const KeyboardKey konami_code[] = { KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_B, KEY_A };
double            last_keypress = 0;
int               cheating_progress = 0;

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
        scale_factor = log(1 + (EASE_OUT_STRENGTH * elapsed)) / log(1 + EASE_OUT_STRENGTH);

        if (elapsed >= 1)
            direction = IDLE;
    }
    else if (direction == COLLAPSE)
    {
        float elapsed = Clamp((t - st) / EASE_OUT_DURATION * 4, 0, 1);  // collapses 4 times faster
        scale_factor = 1 - log(1 + (EASE_OUT_STRENGTH * elapsed)) / log(1 + EASE_OUT_STRENGTH);

        if (elapsed >= 1)
        {
            show_combined = !show_combined;
            direction = EXPAND;
            st = t;
        }
    }

    float wave_spread = fmax(screen_width, REF_WIDTH * WIDTH_THRESHOLD);
    float response_factor = fmin(wave_spread / REF_WIDTH, 1);
    int   n = Clamp(ceil(screen_width / SPACING) + 1, 2, MAX_SAMPLES);  // one every SPACING pixels so the density is the same on every
                                                                        // screen

    for (int w = 0; w < WAVE_COUNT; w++)
    {
        float angular_velocity = 2 * PI / periods[w];

        float  amplitude = SCALABLE_AMPLITUDE(amplitudes[w]) * response_factor;
        float  phase_per_px = 2 * PI * frequencies[w] / wave_spread;
        double time_phase = phase_offsets[w] - angular_velocity * t;

        for (int i = 0; i < n; i++)
        {
            double horizontal_displacement = ((double)i / (n - 1)) * screen_width;
            double vertical_displacement = amplitude * sin(phase_per_px * horizontal_displacement + time_phase);

            waves[w].points[i] = (Vector2){ horizontal_displacement, sea_level - vertical_displacement };

            if (w == 0)
                combined_wave[i] = (Vector2){ horizontal_displacement, sea_level };

            combined_wave[i].y -= vertical_displacement;
        }
        if (!show_combined)
            DrawSplineLinear(waves[w].points, n, 4 * response_factor, wave_colors[w]);
    }
    if (show_combined)
        DrawSplineLinear(combined_wave, n, 6 * response_factor, RED);

    EndDrawing();
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(800, 600, "");

    st = GetTime();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    SetWindowMinSize(600, 400);
    SetTargetFPS(60);

    while (!WindowShouldClose())
        update_frame();
#endif

    CloseWindow();
    return 0;
}
