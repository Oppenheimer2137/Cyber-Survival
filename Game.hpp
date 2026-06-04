#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.hpp"
#include "Utils.hpp"
#include "Structs.hpp"
#include "Upgrades.hpp"

class Game {
public:
    Game();
    void run();
private:
    enum class State { Menu, Playing, LevelUp, GameOver };

    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;
    State            state = State::Menu;
    float            globalTime = 0.f;

    // Tło
    std::vector<Star> stars;

    // Przeciwnicy
    std::vector<Enemy>    enemies;
    std::vector<Bullet>   bullets;
    std::vector<Particle> particles;

    // Orby
    std::vector<XpOrb>    xpOrbs;
    std::vector<HpOrb>    hpOrbs;
    std::vector<BoostOrb> boostOrbs;

    // Timery orbów
    float xpSpawnTimer    = 0.f;
    float hpSpawnTimer    = 0.f;
    float boostSpawnTimer = 0.f;

    // Boosty tymczasowe
    float boostSpeedTimer    = 0.f;
    float boostFireTimer     = 0.f;
    float boostMagnetTimer   = 0.f;
    float boostDmgTimer      = 0.f;
    float boostGhostTimer    = 0.f;

    // Fale
    int       waveNumber        = 0;
    int       waveEnemiesLeft   = 0;
    int       waveEnemiesSpawn  = 0;
    float     waveSpawnTimer    = 0.f;
    float     waveSpawnInterval = 1.5f;
    float     waveClearTimer    = 0.f;
    enum class WaveState { Countdown, Spawning, WaitingClear };
    WaveState waveState = WaveState::Countdown;

    // Gracz
    float        fireTimer   = 0.f;
    float        invincTimer = 0.f;
    int          playerHp    = 100;
    int          playerXp    = 0;
    int          playerLevel = 1;
    int          xpToNext    = 10;
    sf::Vector2f playerPos   = {0.f, 0.f};
    sf::ConvexShape playerShape;

    // Dash
    sf::Vector2f dashDir;
    float dashTimer = 0.f;
    float dashCd    = 0.f;
    bool  dashing   = false;

    // Ulepszenia
    std::vector<Upgrade> upgradePool;
    std::vector<Upgrade*> upgradeChoices;
    int  pendingLevelUps = 0;

    void buildUpgradeChoices();
    void applyUpgrade(int choiceIndex);
    void drawLevelUp();

    // ── Metody ───────────────────────────────────────────────
    void loadFont();
    void buildStars();
    void buildPlayerShape();
    void handleEvents();
    void update(float dt);
    void render();
    void drawBG();
    void drawPlayer();
    void drawMenu();
    void drawHUD();
    void drawGameOver();

    void spawnEnemy();
    void spawnBoss();
    void startWave();
    void updateWave(float dt);
    void updateEnemies(float dt);
    void drawEnemies();
    void checkPlayerHit();
    void findAndShoot();
    void updateBullets(float dt);
    void drawBullets();

    void tryDash();
    void tickDash(float dt);

    void spawnXpOrb(sf::Vector2f pos, int value);
    void updateXpOrbs(float dt);
    void drawXpOrbs();
    void checkXpPickup();

    void spawnHpOrb(sf::Vector2f pos);
    void spawnBoostOrb(sf::Vector2f pos, int type);
    void updateHpOrbs(float dt);
    void updateBoostOrbs(float dt);
    void drawHpOrbs();
    void drawBoostOrbs();
    void checkHpPickup();
    void checkBoostPickup();
    void tickBoosts(float dt);

    void spawnParticles(sf::Vector2f pos, sf::Color col, int n);
    void updateParticles(float dt);
    void drawParticles();

    void resetGame();
};




