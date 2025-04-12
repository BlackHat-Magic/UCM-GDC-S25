#include "audio.h"

bool AudioSystem::init(int frequency, Uint16 format, int channels, int chunksize) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Failed to initialize SDL audio: %s", SDL_GetError());
        return false;
    }

    if (Mix_OpenAudio(frequency, format, channels, chunksize) < 0) {
        SDL_Log("Failed to open SDL_mixer audio: %s", Mix_GetError());
        return false;
    }

    Mix_AllocateChannels(32); // 32 channels is arbitrary, adjust as needed
    
    return true;
}

void AudioSystem::quit() {
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

SoundEffect::SoundEffect(const char * path) {
    chunk_ = Mix_LoadWAV(path);
    if (!chunk_) {
        throw std::runtime_error(std::string("Failed to load sound effect: ") + Mix_GetError());
    }
}

SoundEffect::~SoundEffect() {
    if (chunk_) {
        Mix_FreeChunk(chunk_);
    }
}

void SoundEffect::play(int loops) const {
    if (Mix_PlayChannel(-1, chunk_, loops) == -1) {
        SDL_Log("Failed to play sound effect: %s", Mix_GetError());
    }
}

void SoundEffect::setVolume(int volume) {
    Mix_VolumeChunk(chunk_, volume);
}

MusicTrack::MusicTrack(const char * path) {
    music_ = Mix_LoadMUS(path);
    if (!music_) {
        throw std::runtime_error(std::string("Failed to load music track: ") + Mix_GetError());
    }
}

MusicTrack::~MusicTrack() {
    if (music_) {
        Mix_FreeMusic(music_);
    }
}

void MusicTrack::play(int loops) const {
    if (Mix_PlayMusic(music_, loops) == -1) {
        SDL_Log("Failed to play music: %s", Mix_GetError());
    }
}

void MusicTrack::setVolume(int volume) {
    Mix_VolumeMusic(volume);
}

void MusicTrack::pause() {
    Mix_PauseMusic();
}

void MusicTrack::resume() {
    Mix_ResumeMusic();
}

void MusicTrack::stop() {
    Mix_HaltMusic();
}