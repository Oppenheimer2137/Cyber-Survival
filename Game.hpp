#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.hpp"
#include "Utils.hpp"

struct Star { sf::Vector2f worldPos; float r, bright, twSpd, twPhase; };

struct Enemy {sf::Vector2f worldPos; float hp, speed, flashTimer; bool alive; };

struct XpOrb { sf::Vector2f worldPos; int value; bool alive; };

class Game {
public:
    Game();
    void run();
private:
    enum class State { Menu, Playing, GameOver };


    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;
    State            state = State::Menu;
    float            globalTime = 0.f;


    // Tło
    std::vector<Star> stars;

    // Przeciwnicy
    std::vector<Enemy> enemies;

    std::vector<XpOrb> xpOrbs;
    float xpSpawnTimer = 0.f;
    int   playerXp     = 0;
    int   playerLevel  = 1;
    int   xpToNext     = 10;

    // Fale
    int       waveNumber        = 0;
    int       waveEnemiesLeft   = 0;
    int       waveEnemiesSpawn  = 0;
    float     waveSpawnTimer    = 0.f;
    float     waveSpawnInterval = 1.5f;
    float     waveClearTimer    = 0.f;
    enum class WaveState { Countdown, Spawning, WaitingClear };
    WaveState waveState         = WaveState::Countdown;

    struct Bullet { sf::Vector2f worldPos, dir; float speed, lifetime; bool alive; };
    std::vector<Bullet> bullets;
    float fireTimer = 0.f;
    float invincTimer = 0.f;
    int   playerHp    = 100;

    // Gracz
    sf::Vector2f    playerPos = {0.f, 0.f};
    sf::ConvexShape playerShape;

    void loadFont();
    void buildStars();
    void buildPlayerShape();

    void handleEvents();
    void update(float dt);
    void render();
    void drawBG();
    void drawPlayer();
    void drawMenu();
    void spawnEnemy();
    void startWave();
    void updateWave(float dt);
    void updateEnemies(float dt);
    void drawEnemies();
    void checkPlayerHit();
    void updateBullets(float dt);
    void drawBullets();
    void findAndShoot();
    void drawHUD();
    void drawGameOver();
    void spawnXpOrb(sf::Vector2f pos, int value);
    void updateXpOrbs(float dt);
    void drawXpOrbs();
    void checkXpPickup();
};
