#include <atolla/source.h>
#include <stdio.h>
#include <math.h>

#if defined(HAVE_POSIX_SLEEP)
#include <unistd.h>
#define sleep_ms(ms) (usleep((ms) * 1000))
#elif defined(HAVE_WINDOWS_SLEEP)
#include <windows.h>
#define sleep_ms(ms) (Sleep((ms)))
#else
#error "No sleep function available"
#endif

const char* hostname = "brett.local";
const int port = 10042;
const int frame_length_ms = 30;
const int max_buffered_frames = 100;
const int simulation_length = 20; // amount of buffers to simulate

static void run_sink();
static void run_source();

int main(int argc, char* argv[])
{
    run_source();   
}

typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}


static void run_source()
{
    AtollaSourceSpec spec;
    spec.sink_hostname = hostname;
    spec.sink_port = port;
    spec.frame_duration_ms = frame_length_ms;
    spec.max_buffered_frames = max_buffered_frames;
    spec.retry_timeout_ms = 0; // 0 means pick a default value
    spec.disconnect_timeout_ms = 0; // 0 means pick a default value

    printf("Starting atolla source\n");

    AtollaSource source = atolla_source_make(&spec);

    int iterations = 0;
    while(atolla_source_state(source) != ATOLLA_SOURCE_STATE_OPEN)
    {
        if(atolla_source_state(source) == ATOLLA_SOURCE_STATE_ERROR) {
            printf("Borrow failed\n");
            return;
        }

        ++iterations;
        sleep_ms(1);
    }

    printf("Is open after %d iterations\n", iterations);

    static double t = 0.0;
    uint8_t frame[] = { 1, 2, 3, 4, 5, 6 };

    for(int i = 0; i < max_buffered_frames * simulation_length; ++i)
    {
        t += frame_length_ms / 1000.0;
        double poscos = (cos(t*10) + 1.0) / 2.0;
        uint8_t col = (uint8_t) (poscos * 255.0);

        HsvColor hsv = { (uint8_t) (t*50), 255, col };
        RgbColor rgb = HsvToRgb(hsv);

        frame[0] = rgb.r;
        frame[1] = rgb.g;
        frame[2] = rgb.b;

        HsvColor hsv2 = { (uint8_t) (t*50+128), 255, col };
        rgb = HsvToRgb(hsv2);

        frame[3] = rgb.r;
        frame[4] = rgb.g;
        frame[5] = rgb.b;

        atolla_source_put(source, frame, sizeof(frame) / sizeof(uint8_t));
    }
}
