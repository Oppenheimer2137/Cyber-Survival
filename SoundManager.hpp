#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <vector>

class SoundManager {
public:
    void loadAll();
    void play(const std::string& name);
    void playLoop(const std::string& name);
    void stopLoop(const std::string& name);
    bool isLoopPlaying(const std::string& name);

    void playMusic(const std::string& name, bool loop = true);
    void stopMusic();

    void setSfxVolume(float v);
    void setMusicVolume(float v);
    float getSfxVolume() const { return sfxVolume; }
    float getMusicVolume() const { return musicVolume; }

private:
    std::map<std::string, sf::SoundBuffer> buffers;

    std::vector<sf::Sound> sndPool;
    static const int POOL_SIZE = 32;


    std::map<std::string, sf::Sound> loopSounds;

    sf::Music music;
    std::string currentMusic;

    float sfxVolume   = 70.f;
    float musicVolume = 50.f;
};
