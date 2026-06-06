#pragma once
#include <SFML/Graphics.hpp>

struct Star {
    sf::Vector2f worldPos;
    float r, bright, twSpd, twPhase;
};

struct Enemy {
    sf::Vector2f worldPos;
    float hp, maxHp, speed, flashTimer;
    float frozenTimer = 0.f;
    bool  alive, isBoss;
    int   type;
    float zigzagTimer;
};

struct XpOrb {
    sf::Vector2f worldPos;
    int   value;
    int   orbSize;
    bool  alive;
};

struct HpOrb {
    sf::Vector2f worldPos;
    bool alive;
};

struct BoostOrb {
    sf::Vector2f worldPos;
    bool alive;
    int  type; // 0=SpeedCore, 1=Overclock, 2=DataSurge, 3=Overload, 4=GhostProtocol
};

struct Particle {
    sf::Vector2f worldPos, vel;
    float radius, lifetime, maxLife;
    sf::Color color;
    bool alive;
};

struct Bullet {
    sf::Vector2f worldPos, dir;
    float speed, lifetime;
    bool  alive;
};
