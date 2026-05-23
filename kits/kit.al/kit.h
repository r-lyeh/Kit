// @todo OAL page 21
// "Buffers containing more than one channel of data will be played without 3D spatialization."

// @todo
// speaker_lerp(unsigned source, float *node_position[3], float smoothing = 2)
//   const float alpha = 1.0f - expf(-dt * smoothing);

// @todo: source: AL_CONE_INNER_ANGLE deg
// @todo: source: AL_CONE_OUTER_ANGLE deg
// @todo: source: AL_CONE_OUTER_GAIN
// @todo: source: AL_SEC_OFFSET
//
// @todo: alSourceQueueBuffers()
// @todo: AL_SOURCE_TYPE > AL_STATIC AL_STREAMING

#ifndef KIT_OPENAL_H
#define KIT_OPENAL_H "0.0.0"
#define AL_LIBTYPE_STATIC
#include "AL/al.h"
#include "AL/alc.h"

#if NDEBUG
#define AL_CHECK(...) do { __VA_ARGS__; } while(0)
#else
#define AL_CHECK(...) do { __VA_ARGS__; for(ALenum err = alGetError(); err != AL_NONE; err = AL_NONE) SDL_Log("OpenAL Error at %s (%s)! %s (%u)", __func__, #__VA_ARGS__, alGetString(err), (unsigned int)err); } while(0)
#endif

bool     al_open(void);
bool     al_close(void);

unsigned al_audio_new(const char *audiofile);
unsigned al_audio_duration(unsigned buffer); // millisecs
void     al_audio_free(unsigned *sample);

unsigned al_speaker_new(const float position[3]);
unsigned al_speaker_range(unsigned speaker, float mindistance, float maxdistance, float rollOff); // how and where sound attenuates over distance
unsigned al_speaker_relative(unsigned speaker, bool relative); // is position relative to listener
unsigned al_speaker_position(unsigned speaker, const float position[3]); // sets panning (audio2d) or position (audio3d)
unsigned al_speaker_velocity(unsigned speaker, const float velocity[3]);
unsigned al_speaker_direction(unsigned speaker, const float direction[3]);
unsigned al_speaker_loop(unsigned speaker, bool loop);
unsigned al_speaker_volume(unsigned speaker, float volume);
unsigned al_speaker_pitch(unsigned speaker, float pitch);
unsigned al_speaker_play(unsigned speaker, unsigned audio);
unsigned al_speaker_pause(unsigned speaker);
unsigned al_speaker_resume(unsigned speaker);
unsigned al_speaker_stop(unsigned speaker);
unsigned al_speaker_playing(unsigned speaker); // returns 0 if not playing, source otherwise (>0)
unsigned al_speaker_stopped(unsigned speaker); // returns 0 if not stopped, source otherwise (>0)
unsigned al_speaker_paused(unsigned speaker);  // returns 0 if not paused,  source otherwise (>0)
void     al_speaker_free(unsigned *speaker);

void     al_listener_volume(const float volume); // ~master volume
void     al_listener_position(const float position[3]);
void     al_listener_velocity(const float velocity[3]);
void     al_listener_orientation(const float normdir[3], const float normup[3]);
void     al_listener_doppler(float factor, float speed_of_sound_meters_sec); // 1.0 + 343.3 m/s
void     al_listener_model(const char *distance_model); // "none|inverse|linear|exponent"|"clamped"

#endif

#if KIT_CODE
#pragma once
#include "3rd_mojoal.c"

#if __has_include("3rd_dr_wav.h")
#define DR_WAV_IMPLEMENTATION
#include "3rd_dr_wav.h"
#endif

#if __has_include("3rd_dr_mp3.h")
#define DR_MP3_IMPLEMENTATION
#define DRMP3_MALLOC     SDL_malloc
#define DRMP3_REALLOC    SDL_realloc
#define DRMP3_FREE       SDL_free
#include "3rd_dr_mp3.h"
#endif

#if __has_include("3rd_dr_flac.h")
#define DR_FLAC_IMPLEMENTATION
#include "3rd_dr_flac.h"
#endif

#if __has_include("3rd_stb_vorbis.c")
//#define STB_VORBIS_HEADER_ONLY
#define error stbv_error
#include "3rd_stb_vorbis.c"
#undef error
#undef  L
#undef  R
#undef  C
#endif

static ALCdevice *device = NULL;
static ALCcontext *context = NULL;

bool al_close(void) {
    if( context ) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        context = NULL;
    }

    if( device ) {
        alcCloseDevice(device);
        device = NULL;
    }

    return true;
}

bool al_open(void) {
    al_close();

    if( !device ) {
        device = alcOpenDevice(NULL);
        if( !device ) {
            SDL_Log("Couldn't open OpenAL default device.");
            return false;
        }
    }

    if( !context ) {
        context = alcCreateContext(device, NULL);
        if( !context ) {
            SDL_Log("Couldn't create OpenAL context.");
            return false;
        }

        alcMakeContextCurrent(context);
    }

//    if (alGetString(AL_VERSION))    SDL_Log("OpenAL version: %s\n", alGetString(AL_VERSION));
//    if (alGetString(AL_EXTENSIONS)) SDL_Log("OpenAL extensions: %s\n", alGetString(AL_EXTENSIONS));

    return true;
}

// by default, we keep memory requirements small:
// - we decode to 16-bits (we can upscale to float later).
// - we leave channels as-is (we can upscale to stereo later).
// - we leave frequency as-is (we can upscale to 44khz later).
unsigned al_audio_new(const char *pathfile) {
    SDL_AudioSpec spec = {0};
    uint8_t *buf = NULL;
    uint32_t buflen = 0;
    ALuint bid = 0, invalid = 0; // assumes valid bids are never 0

#ifdef DR_WAV_IMPLEMENTATION
    //if( !buf ) for( drwav w = {0}, *wav = &w; wav && drwav_init_memory(wav, bin, len, NULL); wav = 0 ) {
    if( !buf ) for( drwav w = {0}, *wav = &w; wav && drwav_init_file(wav, pathfile, NULL); wav = 0 ) {
        spec.channels = wav->channels;
        spec.freq = wav->sampleRate;
        spec.format = SDL_AUDIO_S16;
        buflen = wav->totalPCMFrameCount * sizeof(short) * spec.channels;
        buf = SDL_malloc(buflen);
        drwav_read_pcm_frames_s16(wav, wav->totalPCMFrameCount, (short*)buf);
        drwav_uninit(wav);
    }
#else
    if( !buf ) if( SDL_LoadWAV(pathfile, &spec, &buf, &buflen) ) {
        // ok...
    }
#endif

#ifdef STB_VORBIS_INCLUDE_STB_VORBIS_H
    //if( !buf ) for( void *oss = stb_vorbis_open_memory( (unsigned char *)data, size, NULL, NULL); oss; stb_vorbis_close(oss), oss = 0) {
    if( !buf ) for( void *oss = stb_vorbis_open_filename(pathfile, NULL, NULL); oss; stb_vorbis_close(oss), oss = 0) {
        stb_vorbis_info info = stb_vorbis_get_info(oss);

        spec.channels = info.channels;
        spec.format = SDL_AUDIO_S16;
        spec.freq = info.sample_rate;

        buflen = sizeof(short) * (stb_vorbis_stream_length_in_samples(oss) * info.channels);
        buf = SDL_malloc(buflen);

        stb_vorbis_get_samples_short_interleaved(oss, info.channels, (short*)buf, buflen / 2);
    }
#endif        

#ifdef DR_MP3_IMPLEMENTATION
    drmp3_config mp3_cfg = { 2, 44100 };
    drmp3_uint64 mp3_framecount;
    //if( !buf ) for( short* mp3 = drmp3_open_memory_and_read_pcm_frames_s16(bin, len, &mp3_cfg, &mp3_framecount, NULL); mp3 ; mp3 = 0 ) {
    if( !buf ) for( short* mp3 = drmp3_open_file_and_read_pcm_frames_s16(pathfile, &mp3_cfg, &mp3_framecount, NULL); mp3; mp3 = 0 ) {
        spec.channels = mp3_cfg.channels;
        spec.freq = mp3_cfg.sampleRate;
        spec.format = SDL_AUDIO_S16;
        buflen = mp3_framecount * mp3_cfg.channels * sizeof(short);
        buf = (uint8_t*)mp3;
    }
#endif

#ifdef DR_FLAC_IMPLEMENTATION
    if( !buf ) for( drflac* flac = drflac_open_file(pathfile, NULL); flac; drflac_close(flac), flac = 0 ) {
        spec.channels = flac->channels;
        spec.freq = flac->sampleRate;
        spec.format = SDL_AUDIO_S16;
        buflen = flac->totalPCMFrameCount * sizeof(short) * flac->channels;
        buf = SDL_malloc(buflen);
        drflac_read_pcm_frames_s16(flac, flac->totalPCMFrameCount, (short*)buf); // _f32
    }
#endif

    if( !buf ) {
        SDL_Log("Loading '%s' failed! %s", pathfile, SDL_GetError());
        return invalid;
    }

    ALenum fmt = AL_NONE;
    /**/ if( spec.format == SDL_AUDIO_U8  && spec.channels == 1 ) fmt = AL_FORMAT_MONO8;
    else if( spec.format == SDL_AUDIO_U8  && spec.channels == 2 ) fmt = AL_FORMAT_STEREO8;
    else if( spec.format == SDL_AUDIO_S16 && spec.channels == 1 ) fmt = AL_FORMAT_MONO16;
    else if( spec.format == SDL_AUDIO_S16 && spec.channels == 2 ) fmt = AL_FORMAT_STEREO16;
    else if( spec.format == SDL_AUDIO_S32 && spec.channels == 1 && alIsExtensionPresent("AL_EXT_32bit_formats") ) fmt = alGetEnumValue("AL_FORMAT_MONO_I32");
    else if( spec.format == SDL_AUDIO_S32 && spec.channels == 2 && alIsExtensionPresent("AL_EXT_32bit_formats") ) fmt = alGetEnumValue("AL_FORMAT_STEREO_I32");
    else if( spec.format == SDL_AUDIO_F32 && spec.channels == 1 && alIsExtensionPresent("AL_EXT_FLOAT32") )       fmt = alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
    else if( spec.format == SDL_AUDIO_F32 && spec.channels == 2 && alIsExtensionPresent("AL_EXT_FLOAT32") )       fmt = alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");

    if (fmt == AL_NONE) {
        SDL_Log("Can't queue '%s', format not supported by the AL.", pathfile);
        SDL_free(buf);
        return invalid;
    }

    alGenBuffers(1, &bid);
    if (alGetError()) {
        SDL_free(buf);
        return invalid;
    }

    AL_CHECK(alBufferData(bid, fmt, buf, buflen, spec.freq));
    SDL_free(buf);

    return bid;
}
unsigned al_audio_duration(unsigned buffer) { // in ms
    if( buffer ) {
        ALint size, bits, channels, freq;

        alGetBufferi(buffer, AL_SIZE, &size);
        alGetBufferi(buffer, AL_BITS, &bits);
        alGetBufferi(buffer, AL_CHANNELS, &channels);
        alGetBufferi(buffer, AL_FREQUENCY, &freq);

        if( alGetError() == AL_NO_ERROR )
            return (1000.0 * ((ALuint)size/channels/(bits/8))) / (ALfloat)(freq + !freq);
    }
    return 0;
}
void al_audio_free(unsigned *sample) {
    if( sample ) {
        ALuint bid = *sample;
        AL_CHECK(alDeleteBuffers(1, &bid));
        *sample = 0;
    }
}

// Controls how and where sound attenuates over distance
unsigned al_speaker_range(unsigned source, float mindistance, float maxdistance, float rollOff) {
    // Distance at which attenuation begins.
    // At or closer than this distance, the sound plays at full volume.
    alSourcef(source, AL_REFERENCE_DISTANCE, mindistance);

    // Maximum distance for attenuation (used in clamped distance models: AL_INVERSE_DISTANCE_CLAMPED, AL_LINEAR_DISTANCE_CLAMPED; ignored otherwise).
    // Beyond this distance, the volume stops decreasing (it does NOT go silent).
    alSourcef(source, AL_MAX_DISTANCE, maxdistance);

    // Controls how quickly the sound fades with distance.
    // Higher = faster fade, lower = slower fade, 0 = no attenuation.
    alSourcef(source, AL_ROLLOFF_FACTOR, rollOff);
    return source;
}
unsigned al_speaker_relative(unsigned source, bool relative_pos) { return alSourcei(source, AL_SOURCE_RELATIVE, relative_pos), source; }
unsigned al_speaker_position(unsigned source, const float position[3]) { return alSourcefv(source, AL_POSITION, position), source; }
unsigned al_speaker_velocity(unsigned source, const float velocity[3]) { return alSourcefv(source, AL_VELOCITY, velocity), source; }
unsigned al_speaker_direction(unsigned source, const float direction[3]) { return alSourcefv(source, AL_DIRECTION, direction), source; }
unsigned al_speaker_loop(unsigned source, bool on) { return alSourcei( source, AL_LOOPING, !!on), source; }
unsigned al_speaker_volume(unsigned source, float volume) { return alSourcef( source, AL_GAIN, volume <= 0 ? 1e-5f : volume >= 1 ? 1.f : volume ), source; }
unsigned al_speaker_pitch(unsigned source,  float pitch) { return alSourcef( source, AL_PITCH, pitch <= 0 ? 1e-5f : pitch >= 10 ? 10.f : pitch ), source; }

unsigned al_speaker_new(const float position[3]) {
    ALuint sid = 0;
    alGenSources(1, &sid);
    if (alGetError()) {
        return 0;
    }
    AL_CHECK( alSourcef(sid, AL_MIN_GAIN, 0.0f) );
    AL_CHECK( alSourcef(sid, AL_MAX_GAIN, 1.0f) );
    bool audio3d = position && (fabsf(position[0]) > 1 || fabsf(position[1]) > 1 || fabsf(position[2]) > 1);
    al_speaker_relative(sid, audio3d ? 0 : 1);
    if( position ) al_speaker_position(sid, position);
    return sid; // assumes valid sids are never 0
}

unsigned al_speaker_stop(unsigned source) {
    AL_CHECK(alSourceStop(source));
    AL_CHECK(alSourcei(source, AL_BUFFER, 0)); // unbind
    return source;
}

unsigned al_speaker_play(unsigned source, unsigned sample) {
    al_speaker_stop(source);
    if( !sample ) return source;

    AL_CHECK(alSourcei(source, AL_BUFFER, sample)); // bind
    AL_CHECK(alSourcePlay(source));

    return source;
}
unsigned al_speaker_pause(unsigned source) { return alSourcePause(source), source; }
unsigned al_speaker_resume(unsigned source) { return alSourcePlay(source), source; }
unsigned al_speaker_playing(unsigned source) { ALint state; return alGetSourcei( source, AL_SOURCE_STATE, &state ), source * (state == AL_PLAYING); }
unsigned al_speaker_stopped(unsigned source) { ALint state; return alGetSourcei( source, AL_SOURCE_STATE, &state ), source * (state == AL_STOPPED); }
unsigned al_speaker_paused(unsigned source)  { ALint state; return alGetSourcei( source, AL_SOURCE_STATE, &state ), source * (state == AL_PAUSED); }
void     al_speaker_free(unsigned *source) { if( !source ) return; al_speaker_stop(*source); ALuint sid = *source; AL_CHECK(alDeleteSources(1, &sid)); *source = 0; }

void al_listener_volume(const float volume) { AL_CHECK(alListenerf( AL_GAIN, volume <= 0 ? 1e-5f : volume >= 1 ? 1.f : volume )); }
void al_listener_position(const float position[3]) { AL_CHECK(alListenerfv( AL_POSITION, position )); }
void al_listener_velocity(const float velocity[3]) { AL_CHECK(alListenerfv( AL_VELOCITY, velocity )); }
void al_listener_orientation(const float normdir[3], const float normup[3]) { float orientation6[] = { normdir[0], normdir[1], normdir[2], -normup[0], -normup[1], -normup[2] }; AL_CHECK(alListenerfv( AL_ORIENTATION, orientation6 )); }
void al_listener_doppler(float factor, float speed_of_sound_meters_sec) { AL_CHECK(alSpeedOfSound(speed_of_sound_meters_sec)); AL_CHECK(alDopplerFactor(factor < 0 ? 1e-5f : factor )); } // 1.0,343.3 by default
void al_listener_model(const char *distance_model) {
    const char *m = distance_model ? distance_model : "none"; // "inverse|clamped";
    ALenum value = AL_NONE;
    /**/ if( strstri(m, "lin" ) ) value = strstri(m, "clamp") ? AL_LINEAR_DISTANCE_CLAMPED   : AL_LINEAR_DISTANCE;
    else if( strstri(m, "inv" ) ) value = strstri(m, "clamp") ? AL_INVERSE_DISTANCE_CLAMPED  : AL_INVERSE_DISTANCE;
    else if( strstri(m, "exp" ) ) value = strstri(m, "clamp") ? AL_EXPONENT_DISTANCE_CLAMPED : AL_EXPONENT_DISTANCE;
    AL_CHECK( alDistanceModel( value ) );
} // "none|inverse|linear|exponent"|"clamped"

#endif
