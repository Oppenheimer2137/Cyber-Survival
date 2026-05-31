#include <algorithm>
#include "Game.hpp"
#include <cstdlib>
#include <cmath>
#include <ctime>

// ── Konstruktor ───────────────────────────────────────────────
Game::Game() {
    std::srand((unsigned)std::time(nullptr));
    auto desktop = sf::VideoMode::getDesktopMode();
    GW = (float)desktop.width;
    GH = (float)desktop.height;

    window.create(desktop, "CyberSurvival", sf::Style::Fullscreen);
    window.setFramerateLimit(144);


    loadFont();
    buildStars();
    buildPlayerShape();
}

void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        handleEvents();
        update(dt);
        render();
    }
}

// ── Setup ─────────────────────────────────────────────────────
void Game::loadFont() {
    const char* paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        nullptr
    };
    for (int i = 0; paths[i]; ++i)
        if (font.loadFromFile(paths[i])) break;
}

void Game::buildStars() {
    stars.resize(STAR_COUNT);
    for (auto& s : stars) {
        s.worldPos = { frandr(-3000.f, 3000.f), frandr(-2000.f, 2000.f) };
        s.r        = frandr(0.5f, 2.5f);
        s.bright   = frandr(0.4f, 1.f);
        s.twSpd    = frandr(1.f, 3.f);
        s.twPhase  = frand() * GPI * 2.f;
    }
}

void Game::buildPlayerShape() {
    playerShape.setPointCount(5);
    playerShape.setPoint(0, {  0.f, -20.f });
    playerShape.setPoint(1, { 14.f,   8.f });
    playerShape.setPoint(2, {  9.f,  18.f });
    playerShape.setPoint(3, { -9.f,  18.f });
    playerShape.setPoint(4, {-14.f,   8.f });
    playerShape.setFillColor(sf::Color(0, 20, 30));
    playerShape.setOutlineColor(COL_CORE);
    playerShape.setOutlineThickness(2.f);
}

// ── Zdarzenia ─────────────────────────────────────────────────
void Game::handleEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) window.close();
        if (ev.type == sf::Event::KeyPressed) {
            if (ev.key.code == sf::Keyboard::Escape) window.close();
            if (ev.key.code == sf::Keyboard::Return && state == State::Menu)
                state = State::Playing;
        }
    }
}

// ── Update ────────────────────────────────────────────────────
void Game::update(float dt) {
    globalTime += dt;

    if (state == State::Playing) {
        // Ruch WASD
        sf::Vector2f dir(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
        playerPos += vnorm(dir) * PLAYER_SPD * dt;

        // Obróć statek w kierunku ruchu
        if (vlen(dir) > 0.01f) {
            float angle = std::atan2(dir.y, dir.x) * 180.f / GPI + 90.f;
            playerShape.setRotation(angle);
        }
        spawnTimer += dt;
        if (spawnTimer >= 2.f) {
                spawnTimer = 0.f;spawnEnemy();
        }
        updateEnemies(dt);
    }
}

// ── Render ────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color(2, 5, 12));
    drawBG();
    if (state == State::Playing) drawEnemies(); drawPlayer();
    if (state == State::Menu)    drawMenu();
    window.display();
}

void Game::drawBG() {
    sf::CircleShape dot;
    for (auto& s : stars) {
        sf::Vector2f scr = worldToScreen(s.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        float tw = s.bright * (0.6f + 0.4f * std::sin(globalTime * s.twSpd + s.twPhase));
        dot.setRadius(s.r);
        dot.setOrigin(s.r, s.r);
        dot.setPosition(scr);
        dot.setFillColor(sf::Color(200, 220, 255, (sf::Uint8)(tw * 220.f)));
        window.draw(dot);
    }
}

void Game::drawPlayer() {
    sf::Vector2f scr(GW/2.f, GH/2.f);

    // Poświata silnika
    float glow = 10.f + 5.f * std::sin(globalTime * 4.f);
    sf::CircleShape g(glow);
    g.setOrigin(glow, glow);
    g.setPosition(scr.x, scr.y + 10.f);
    g.setFillColor(sf::Color(0, 200, 255, 40));
    window.draw(g);

    playerShape.setPosition(scr);
    window.draw(playerShape);


}

void Game::drawMenu() {
    if (font.getInfo().family.empty()) return;
    sf::Text t;
    t.setFont(font);
    t.setString("CYBER SURVIVAL");
    t.setCharacterSize(72);
    t.setFillColor(COL_CORE);
    centerText(t, GW/2.f, GH*0.38f);
    window.draw(t);

    float blink = 0.5f + 0.5f * std::sin(globalTime * 3.f);
    t.setString("[ NACISNIJ ENTER ]");
    t.setCharacterSize(28);
    t.setFillColor(sf::Color(200, 220, 255, (sf::Uint8)(blink * 220.f)));
    centerText(t, GW/2.f, GH*0.58f);
    window.draw(t);
}

void Game::spawnEnemy() {
    float angle = frand() * 2.f * GPI;
    float dist  = 700.f + frand() * 200.f;
    Enemy e;
    e.worldPos = playerPos + sf::Vector2f(std::cos(angle)*dist, std::sin(angle)*dist);
    e.hp    = 30.f;
    e.speed = 80.f + frand() * 40.f;
    e.alive = true;
    enemies.push_back(e);
}

void Game::updateEnemies(float dt) {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        e.worldPos += vnorm(playerPos - e.worldPos) * e.speed * dt;
    }
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
                       [](const Enemy& e){ return !e.alive; }),
        enemies.end()
    );
}

void Game::drawEnemies() {
    sf::CircleShape shape(12.f);
    shape.setOrigin(12.f, 12.f);
    shape.setFillColor(sf::Color(180, 30, 30));
    shape.setOutlineColor(sf::Color(255, 60, 60));
    shape.setOutlineThickness(2.f);
    for (auto& e : enemies) {
        if (!e.alive) continue;
        sf::Vector2f scr = worldToScreen(e.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        shape.setPosition(scr);
        window.draw(shape);
    }
}
