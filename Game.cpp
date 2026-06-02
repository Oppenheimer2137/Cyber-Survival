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
            if (ev.key.code == sf::Keyboard::Return && state == State::GameOver)
                state = State::Menu;
        }
    }
}

// ── Update ────────────────────────────────────────────────────
void Game::update(float dt) {
    globalTime += dt;

    if (state == State::Playing) {
        // Ruch WASD
        sf::Vector2f dir(0.f, 0.f);
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))) dir.y -= 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::S))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))) dir.y += 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))) dir.x -= 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) dir.x += 1.f;
        playerPos += vnorm(dir) * PLAYER_SPD * dt;

        // Obróć sie w kierunku ruchu
        if (vlen(dir) > 0.01f) {
            float angle = std::atan2(dir.y, dir.x) * 180.f / GPI + 90.f;
            playerShape.setRotation(angle);
        }

        updateWave(dt);
        updateEnemies(dt);

        findAndShoot();
        updateBullets(dt);
        if (invincTimer > 0.f) invincTimer -= dt;
        checkPlayerHit();
        xpSpawnTimer += dt;
        if (xpSpawnTimer >= 3.f) {
            xpSpawnTimer = 0.f;
            sf::Vector2f spawnPos = playerPos + sf::Vector2f(
            frandr(-800.f, 800.f), frandr(-600.f, 600.f));
            spawnXpOrb(spawnPos, 1);
}
        updateXpOrbs(dt);
        checkXpPickup();
    }


}

// ── Render ────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color(2, 5, 12));
    drawBG();
    if (state == State::Playing) {drawXpOrbs(); drawEnemies(); drawBullets(); drawPlayer(); drawHUD();}
    if (state == State::Menu)    drawMenu();
    if (state == State::GameOver) drawGameOver();
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

    // Poświata
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
    e.worldPos   = playerPos + sf::Vector2f(std::cos(angle)*dist, std::sin(angle)*dist);
    e.hp         = 30.f;
    e.speed      = 80.f + frand() * 40.f + waveNumber * 2.f;
    e.flashTimer = 0.f;
    e.alive      = true;
    enemies.push_back(e);
}

void Game::startWave() {
    waveNumber++;
    waveEnemiesSpawn  = 8 + waveNumber * 3;
    waveEnemiesLeft   = waveEnemiesSpawn;
    waveSpawnTimer    = 0.f;
    waveSpawnInterval = std::max(0.4f, 1.5f - waveNumber * 0.05f);
    waveState         = WaveState::Spawning;
}

void Game::updateWave(float dt) {
    switch (waveState) {
        case WaveState::Countdown:
            waveClearTimer += dt;
            if (waveClearTimer >= (waveNumber == 0 ? 1.f : 3.f)) {
                waveClearTimer = 0.f;
                startWave();
            }
            break;

        case WaveState::Spawning:
            waveSpawnTimer += dt;
            if (waveSpawnTimer >= waveSpawnInterval && waveEnemiesLeft > 0) {
                waveSpawnTimer = 0.f;
                waveEnemiesLeft--;
                spawnEnemy();
            }
            if (waveEnemiesLeft == 0) waveState = WaveState::WaitingClear;
            break;

        case WaveState::WaitingClear:
            if (enemies.empty()) {
                waveClearTimer = 0.f;
                waveState      = WaveState::Countdown;
            }
            break;
    }
}

void Game::updateEnemies(float dt) {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (e.flashTimer > 0.f) e.flashTimer -= dt;
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
    shape.setOutlineColor(sf::Color(255, 60, 60));
    shape.setOutlineThickness(2.f);
    for (auto& e : enemies) {
        if (!e.alive) continue;
        shape.setFillColor(e.flashTimer > 0.f ? sf::Color::White : sf::Color(180, 30, 30));
        sf::Vector2f scr = worldToScreen(e.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        shape.setPosition(scr);
        window.draw(shape);

    }
}

void Game::checkPlayerHit() {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (vlen(e.worldPos - playerPos) < 12.f + 14.f) {
            if (invincTimer <= 0.f) {
                playerHp -= 10;
                invincTimer = 1.f;
                if (playerHp <= 0) state = State::GameOver;
            }
        }
    }
}

void Game::findAndShoot() {
    fireTimer -= 0.016f;
    if (fireTimer > 0.f) return;

    Enemy* target = nullptr;
    float  bestDist = 1e9f;
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float d = vlen(e.worldPos - playerPos);
        if (d < bestDist) { bestDist = d; target = &e; }
    }
    if (!target) return;

    fireTimer = 3.2f;
    Bullet b;
    b.worldPos = playerPos;
    b.dir      = vnorm(target->worldPos - playerPos);
    b.speed    = 500.f;
    b.lifetime = 2.f;
    b.alive    = true;
    bullets.push_back(b);
}

void Game::updateBullets(float dt) {
    for (auto& b : bullets) {
        if (!b.alive) continue;
        b.worldPos += b.dir * b.speed * dt;
        b.lifetime -= dt;
        if (b.lifetime <= 0.f) { b.alive = false; continue; }

        for (auto& e : enemies) {
            if (!e.alive) continue;
            if (vlen(b.worldPos - e.worldPos) < 5.f + 12.f) {
                e.hp -= 10.f;
                e.flashTimer = 0.1f;
                b.alive = false;
                if (e.hp <= 0.f) {
                e.alive = false;
                if (frand() < 0.2f) spawnXpOrb(e.worldPos, 1);
}
                break;
            }
        }
    }
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
                       [](const Bullet& b){ return !b.alive; }),
        bullets.end()
    );
}

void Game::drawBullets() {
    sf::CircleShape shape(5.f);
    shape.setOrigin(5.f, 5.f);
    shape.setFillColor(sf::Color(0, 240, 180));
    for (auto& b : bullets) {
        if (!b.alive) continue;
        sf::Vector2f scr = worldToScreen(b.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        shape.setPosition(scr);
        window.draw(shape);
    }
}

void Game::drawHUD() {
    float barW   = 512.f;
    float barH   = 24.f;
    float x      = 20.f;
    float y      = GH - 30.f;
    float filled = barW * (playerHp / 100.f);

    sf::RectangleShape bgBar({barW, barH});
    bgBar.setPosition(x, y);
    bgBar.setFillColor(sf::Color(60, 0, 0));
    window.draw(bgBar);

    sf::RectangleShape hpBar({filled, barH});
    hpBar.setPosition(x, y);
    hpBar.setFillColor(sf::Color(0, 220, 80));
    window.draw(hpBar);

    float xpFilled = barW * ((float)playerXp / (float)xpToNext);
    sf::RectangleShape xpBg({barW, 10.f});
    xpBg.setPosition(x, y - 16.f);
    xpBg.setFillColor(sf::Color(0, 30, 60));
    window.draw(xpBg);
    sf::RectangleShape xpBar({xpFilled, 10.f});
    xpBar.setPosition(x, y - 16.f);
    xpBar.setFillColor(sf::Color(40, 160, 255));
    window.draw(xpBar);

    if (font.getInfo().family.empty()) return;
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(16);

    t.setString("HP: " + std::to_string(playerHp) + " / 100");
    t.setFillColor(sf::Color(180, 255, 200));
    t.setPosition(x, y - 20.f);
    window.draw(t);

    t.setString("LVL " + std::to_string(playerLevel) + "   XP: " + std::to_string(playerXp) + " / " + std::to_string(xpToNext));
    t.setFillColor(sf::Color(100, 180, 255));
    t.setPosition(x, y - 38.f);
    window.draw(t);

    t.setCharacterSize(22);
    t.setString("WAVE " + std::to_string(waveNumber));
    t.setFillColor(COL_CORE);
    t.setPosition(GW - 160.f, 16.f);
    window.draw(t);

    int alive = (int)enemies.size();
    t.setCharacterSize(16);
    t.setString("Wrogowie: " + std::to_string(alive));
    t.setFillColor(sf::Color(200, 200, 200));
    t.setPosition(GW - 160.f, 44.f);
    window.draw(t);

    if (waveState == WaveState::Countdown && waveNumber > 0) {
        float timeLeft = (waveNumber == 0 ? 1.f : 3.f) - waveClearTimer;
        t.setCharacterSize(32);
        t.setString("FALA " + std::to_string(waveNumber + 1) + " ZA " + std::to_string((int)timeLeft + 1) + "s");
        t.setFillColor(sf::Color(255, 200, 0));
        centerText(t, GW / 2.f, GH * 0.25f);
        window.draw(t);
    }
}

void Game::drawGameOver() {
    if (font.getInfo().family.empty()) return;
    sf::Text t;
    t.setFont(font);
    t.setString("GAME OVER");
    t.setCharacterSize(80);
    t.setFillColor(sf::Color(220, 40, 40));
    centerText(t, GW/2.f, GH*0.38f);
    window.draw(t);

    float blink = 0.5f + 0.5f * std::sin(globalTime * 3.f);
    t.setString("[ ENTER - wroc do menu ]");
    t.setCharacterSize(28);
    t.setFillColor(sf::Color(200, 200, 200, (sf::Uint8)(blink * 220.f)));
    centerText(t, GW/2.f, GH*0.55f);
    window.draw(t);
}

void Game::spawnXpOrb(sf::Vector2f pos, int value) {
    XpOrb orb;
    orb.worldPos = pos;
    orb.value    = value;
    orb.alive    = true;
    xpOrbs.push_back(orb);
}

void Game::updateXpOrbs(float dt) {
    xpOrbs.erase(
        std::remove_if(xpOrbs.begin(), xpOrbs.end(),
                       [](const XpOrb& o){ return !o.alive; }),
        xpOrbs.end()
    );
}

void Game::checkXpPickup() {
    const float pickupRadius = 30.f;
    const float magnetRadius = 150.f;
    for (auto& orb : xpOrbs) {
        if (!orb.alive) continue;
        float d = vlen(orb.worldPos - playerPos);
        if (d < magnetRadius)
            orb.worldPos += vnorm(playerPos - orb.worldPos) * 200.f * 0.016f;
        if (d < pickupRadius) {
            orb.alive = false;
            playerXp += orb.value;
            if (playerXp >= xpToNext) {
                playerXp  -= xpToNext;
                playerLevel++;
                xpToNext   = playerLevel * 10;
                // miejsce na power-up
            }
        }
    }
}

void Game::drawXpOrbs() {
    sf::CircleShape shape(6.f);
    shape.setOrigin(6.f, 6.f);
    shape.setFillColor(sf::Color(40, 160, 255));
    shape.setOutlineColor(sf::Color(100, 200, 255));
    shape.setOutlineThickness(1.f);
    for (auto& orb : xpOrbs) {
        if (!orb.alive) continue;
        sf::Vector2f scr = worldToScreen(orb.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        shape.setPosition(scr);
        window.draw(shape);
    }
}
