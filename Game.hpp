#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.hpp"
#include "Utils.hpp"

struct Star { sf::Vector2f worldPos; float r, bright, twSpd, twPhase; };

class Game {
public:
    Game();
    void run();
private:
    enum class State { Menu, Playing };

    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;
    State            state = State::Menu;
    float            globalTime = 0.f;

    // Tło
    std::vector<Star> stars;

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
};
