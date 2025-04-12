#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdexcept>

class AudioClip {
public:
    virtual ~AudioClip() = default;
    virtual void play(int loops = 0) const = 0;
    virtual void setVolume(int volume) = 0;
};

class SoundEffect : public AudioClip {
public:
    SoundEffect(const char * path);
    ~SoundEffect();

    void play(int loops = 0) const override;
    void setVolume(int volume) override;

private:
    Mix_Chunk* chunk_;
};

class MusicTrack : public AudioClip {
public:
    MusicTrack(const char * path);
    ~MusicTrack();

    void play(int loops = -1) const override;
    void setVolume(int volume) override;
    static void pause();
    static void resume();
    static void stop();

private:
    Mix_Music* music_;
};

class AudioSystem {
public:
    static bool init(int frequency = 44100, Uint16 format = MIX_DEFAULT_FORMAT, int channels = 2, int chunksize = 2048);
    static void quit();

    AudioSystem() = delete;
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
};
