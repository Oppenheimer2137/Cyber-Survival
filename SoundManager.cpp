#include "SoundManager.hpp"

void SoundManager::loadAll() {
    const char* names[] = {
        "move_loop", "dash", "dash_ready", "shoot", "hit_enemy",
        "enemy_death", "wave_start", "upgrade_select", "unpause",
        "pause", "ui_select", "ui_hover", "laser_ray", "emp_flash",
        "cryo_sweep", "corruption_zone", "enemy_shoot", "core_drop",
        "data_vortex", "static_strike", "virus_bomb", "frost_burst",
        "chain_shock", "orbital_hit", "teleport", "split_enemy",
        "boss_spawn", "boost_pickup", "hp_pickup", "xp_pickup",
        "level_up", "game_over", "low_hp_alarm", "player_hit",
        "boss_death"
    };
    for (auto n : names) {
        sf::SoundBuffer buf;
        std::string path = "assets/sfx/" + std::string(n) + ".wav";
        if (buf.loadFromFile(path)) {
            buffers[n] = buf;
        }
    }

    sndPool.resize(POOL_SIZE);

    // Przypisz buffery do dedykowanych pêtli
    if (buffers.count("move_loop")) {
        loopSounds["move_loop"].setBuffer(buffers["move_loop"]);
        loopSounds["move_loop"].setLoop(true);
    }
    if (buffers.count("low_hp_alarm")) {
        loopSounds["low_hp_alarm"].setBuffer(buffers["low_hp_alarm"]);
        loopSounds["low_hp_alarm"].setLoop(true);
    }
    if (buffers.count("corruption_zone")) {
        loopSounds["corruption_zone"].setBuffer(buffers["corruption_zone"]);
        loopSounds["corruption_zone"].setLoop(true);
    }

    setSfxVolume(sfxVolume);
    setMusicVolume(musicVolume);
}

void SoundManager::play(const std::string& name) {
    if (!buffers.count(name)) return;
    for (auto& s : sndPool) {
        if (s.getStatus() != sf::Sound::Playing) {
            s.setBuffer(buffers[name]);
            s.setVolume(sfxVolume);
            s.play();
            return;
        }
    }

    sndPool[0].setBuffer(buffers[name]);
    sndPool[0].setVolume(sfxVolume);
    sndPool[0].play();
}

void SoundManager::playLoop(const std::string& name) {
    if (!loopSounds.count(name)) return;
    if (loopSounds[name].getStatus() != sf::Sound::Playing) {
        loopSounds[name].setVolume(sfxVolume);
        loopSounds[name].play();
    }
}

void SoundManager::stopLoop(const std::string& name) {
    if (!loopSounds.count(name)) return;
    loopSounds[name].stop();
}

bool SoundManager::isLoopPlaying(const std::string& name) {
    if (!loopSounds.count(name)) return false;
    return loopSounds[name].getStatus() == sf::Sound::Playing;
}

void SoundManager::playMusic(const std::string& name, bool loop) {
    if (currentMusic == name && music.getStatus() == sf::Music::Playing) return;
    std::string path = "assets/music/" + name;
    if (music.openFromFile(path)) {
        music.setLoop(loop);
        music.setVolume(musicVolume);
        music.play();
        currentMusic = name;
    }
}

void SoundManager::stopMusic() {
    music.stop();
    currentMusic.clear();
}

void SoundManager::setSfxVolume(float v) {
    sfxVolume = v;
    for (auto& s : sndPool) s.setVolume(v);
    for (auto& [k, s] : loopSounds) s.setVolume(v);
}

void SoundManager::setMusicVolume(float v) {
    musicVolume = v;
    music.setVolume(v);
}
