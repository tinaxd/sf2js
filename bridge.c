#define TSF_IMPLEMENTATION
#include "TinySoundFont/tsf.h"

#include <math.h>
#include <assert.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <emscripten.h>

tsf *load_filename(const char *filename)
{
    return tsf_load_filename(filename);
}

void init_output(tsf *f)
{
    tsf_set_output(f, TSF_MONO, 44100, 0);
}

void note_on(tsf *f, int chan, int key, int velocity)
{
    tsf_note_on(f, chan, key, velocity);
}

void render_float(tsf *f, float *buf, int samples)
{
    tsf_render_float(f, buf, samples, 0);
}

#define BUF_COUNT 2
ALuint sources[1];
ALuint buffers[BUF_COUNT];

void fill_buffer(ALuint buffer)
{
    const int audioFreq = 441;
    const int sampleRate = 44100;
    const int bufLen = sampleRate / 2;
    signed char buf[bufLen];

    for (int i = 0; i < bufLen; i++)
    {
        float t = i / (float)sampleRate;
        buf[i] = (signed char)(sinf(2 * M_PI * audioFreq * t) * 30.0);
    }

    alBufferData(buffer, AL_FORMAT_MONO8, buf, sizeof(buf), sampleRate);
}

void audio_loop(void)
{
    ALuint source = sources[0];

    int processed;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
    printf("%d\n", processed);
    while (processed > 0)
    {
        ALuint buffer;
        alSourceUnqueueBuffers(source, 1, &buffer);

        fill_buffer(buffer);
        alSourceQueueBuffers(source, 1, &buffer);

        processed--;
    }
}

int main()
{
    int major, minor;
    alcGetIntegerv(NULL, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(NULL, ALC_MINOR_VERSION, 1, &minor);

    assert(major == 1);

    printf("ALC version: %i.%i\n", major, minor);
    printf("Default device: %s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));

    ALCdevice *device = alcOpenDevice(NULL);
    ALCcontext *context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    printf("OpenAL version: %s\n", alGetString(AL_VERSION));
    printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
    printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

    ALfloat listenerPos[] = {0.0, 0.0, 1.0};
    ALfloat listenerVel[] = {0.0, 0.0, 0.0};
    ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
    ALfloat listenerGain = 0.1;

    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);
    alListenerf(AL_GAIN, listenerGain);

    // prepare buffers
    alGenBuffers(BUF_COUNT, buffers);

    // prepare sources
    alGenSources(1, sources);
    assert(alIsSource(sources[0]));

    // initial data
    ALuint source = sources[0];
    for (int i = 0; i < BUF_COUNT; i++)
    {
        ALuint buffer = buffers[i];
        fill_buffer(buffer);
    }
    alSourceQueueBuffers(source, 2, buffers);

    alSourcePlay(sources[0]);

    emscripten_set_main_loop(audio_loop, 15, 0);
}