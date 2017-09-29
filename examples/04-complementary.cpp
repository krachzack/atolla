#include <atolla/source.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const int frame_length_ms = 17;
const int max_buffered_frames = 50;
const int run_time_ms = 10000;

static void run_source(const char* sink_hostname, int port);
static void paint(AtollaSource source);

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


static void paint(AtollaSource source)
{
    static double t = 0.0;
    uint8_t frame[] = { 1, 2, 3, 4, 5, 6 };

    for(int i = 0; (i*frame_length_ms) < run_time_ms; ++i)
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

static void run_source(const char* sink_hostname, int port)
{
    AtollaSourceSpec spec;
    spec.sink_hostname = sink_hostname;
    spec.sink_port = port;
    spec.frame_duration_ms = frame_length_ms;
    spec.max_buffered_frames = max_buffered_frames;
    spec.retry_timeout_ms = 0; // 0 means pick a default value
    spec.disconnect_timeout_ms = 0; // 0 means pick a default value
    spec.async_make = false;

    printf("Starting atolla source\n");

    AtollaSource source = atolla_source_make(&spec);
    AtollaSourceState state = atolla_source_state(source);

    if(state == ATOLLA_SOURCE_STATE_OPEN) {
        printf("Atolla source received lent\n");
        paint(source);
    } else {
        printf("Error occurred lending\n");
    }
}

int main(int argc, const char* argv[])
{
    if(argc < 3) {
        run_source("localhost", 10042);
    } else {
        const char* sink_hostname = argv[1];
        int port = atoi(argv[2]);
        run_source(sink_hostname, port);
    }

    return EXIT_SUCCESS;
}
