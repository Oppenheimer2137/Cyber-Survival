#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.hpp"
#include "Utils.hpp"
#include "Structs.hpp"
#include "Upgrades.hpp"
#include "PlayerClass.hpp"

class Game {
public:
    Game();
    void run();
private:
    enum class State { Menu, ClassSelect, Playing, LevelUp, GameOver, Paused };

    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;
    State            state = State::Menu;
    float            globalTime = 0.f;
    float            gameTime = 0.f;
    PlayerClass selectedClass = PlayerClass::None;
    int         hoveredClass  = -1;
    int              totalKills = 0;

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
    int   waveNumber    = 0;
    float waveTimer     = 0.f;
    float spawnTimer    = 0.f;
    float spawnInterval = 2.0f;
    bool  bossSpawned   = false;

    // Gracz
    float        fireTimer   = 0.f;
    float        invincTimer = 0.f;
    int          playerHp    = 100;
    int          playerXp    = 0;
    int          playerLevel = 1;
    int          xpToNext    = 5;
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

    // Statystyki gracza (modyfikowane przez power-upy)
    float playerAtk        = 10.f;
    float playerSpeedMult  = 1.f;
    float playerFireRate   = 3.2f;
    int   playerMaxHp      = 100;
    float playerMagnet     = 150.f;
    float playerCdr        = 0.f;
    float playerHpRegen    = 0.f;
    float baseSpeedMult  = 1.f;
    float baseFireRate   = 3.2f;
    float baseMagnet     = 150.f;

    // Stany aktywnych broni (odblokowane przez power-upy)
    bool  hasOrbitalNode   = false;
    float orbitalAngle     = 0.f;
    int   orbitalLevel     = 0;

    bool  hasChainShock    = false;
    float chainShockCd     = 0.f;
    int   chainShockLevel  = 0;

    bool  hasFrostBurst    = false;
    float frostBurstCd     = 0.f;
    int   frostBurstLevel  = 0;

    bool  hasVirusBomb     = false;
    float virusBombCd      = 0.f;
    int   virusBombLevel   = 0;

    bool  hasStaticStorm   = false;
    float staticStormCd    = 0.f;
    int   staticStormLevel = 0;

    bool  hasDataVortex    = false;
    float dataVortexCd     = 0.f;
    int   dataVortexLevel  = 0;

    bool  hasCoreDropWeapon = false;
    float coreDropCd       = 0.f;
    int   coreDropLevel    = 0;

    bool  hasCorruptionZone = false;
    float corruptionCd     = 0.f;
    int   corruptionLevel  = 0;

    bool  hasShockGrid     = false;
    float shockGridCd      = 0.f;
    int   shockGridLevel   = 0;

    bool  hasPacketFlood   = false;
    float packetFloodCd    = 0.f;
    int   packetFloodLevel = 0;

    bool  hasOverburn      = false;
    float overburnCd       = 0.f;
    int   overburnLevel    = 0;

    bool  hasCryoSweep     = false;
    float cryoSweepCd      = 0.f;
    int   cryoSweepLevel   = 0;

    bool  hasEmpFlash      = false;
    float empFlashCd       = 0.f;
    int   empFlashLevel    = 0;

    bool  hasLaserRay      = false;
    float laserRayCd       = 0.f;
    float laserRayTimer    = 0.f;
    bool  laserRayFiring   = false;
    int   laserRayLevel    = 0;

    // Sfera bossa
    bool  bossArenaActive  = false;
    float bossArenaRadius  = 0.f;
    sf::Vector2f bossArenaCenter = {0.f, 0.f};

    // Pauza
    bool  paused           = false;
    int   pauseSelected    = 0;

    // Kolor pocisków gracza
    sf::Color bulletColor  = sf::Color(0, 240, 180);

    float bgPulse          = 0.f;

    // Wizualne efekty broni
    struct WeaponEffect {
        sf::Vector2f worldPos;
        float radius, alpha, lifetime;
        sf::Color color;
        bool alive;
    };
    std::vector<WeaponEffect> weaponEffects;

    // Strefa korupcji na mapie
    struct CorruptionZoneObj {
        sf::Vector2f worldPos;
        float radius, lifetime, tickTimer;
        bool alive;
    };
    std::vector<CorruptionZoneObj> corruptionZones;

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

    void tickWeapons(float dt);
    void drawWeapons();
    void hitEnemiesInRadius(sf::Vector2f pos, float radius, float dmg, sf::Color col, bool freeze = false);

    void resetGame();
    void drawMinimap();
    void drawClassSelect();
    void applyClass(PlayerClass pc);
    void drawPauseMenu();
    void drawBossArena();
    void updateBossArena();
};




