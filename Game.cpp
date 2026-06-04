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
    upgradePool = getUpgradePool();
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
            if (ev.key.code == sf::Keyboard::Return && state == State::GameOver) {
                resetGame();
                state = State::Menu;

            }
            if (ev.type == sf::Event::KeyPressed && state == State::LevelUp) {
                if (ev.key.code == sf::Keyboard::Num1) applyUpgrade(0);
                if (ev.key.code == sf::Keyboard::Num2) applyUpgrade(1);
                if (ev.key.code == sf::Keyboard::Num3) applyUpgrade(2);
            }
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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) tryDash();
        tickDash(dt);

        // Obróć sie w kierunku ruchu
        if (vlen(dir) > 0.01f) {
            float angle = std::atan2(dir.y, dir.x) * 180.f / GPI + 90.f;
            playerShape.setRotation(angle);
        }

        updateWave(dt);
        updateEnemies(dt);

        fireTimer -= dt;
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
        hpSpawnTimer += dt;
    if (hpSpawnTimer >= 20.f) {
        hpSpawnTimer = 0.f;
        sf::Vector2f p = playerPos + sf::Vector2f(frandr(-900.f,900.f), frandr(-700.f,700.f));
        spawnHpOrb(p);
    }
    boostSpawnTimer += dt;
    if (boostSpawnTimer >= 45.f) {
        boostSpawnTimer = 0.f;
        sf::Vector2f p = playerPos + sf::Vector2f(frandr(-900.f,900.f), frandr(-700.f,700.f));
        spawnBoostOrb(p, rand() % 5);
    }
    updateHpOrbs(dt);
    updateBoostOrbs(dt);
    checkHpPickup();
    checkBoostPickup();
    tickBoosts(dt);
    updateParticles(dt);
    }


}

// ── Render ────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color(2, 5, 12));
    drawBG();
    if (state == State::Playing) {drawParticles(); drawXpOrbs(); drawHpOrbs(); drawBoostOrbs(); drawEnemies(); drawBullets(); drawPlayer(); drawHUD();}
    if (state == State::Menu)    drawMenu();
    if (state == State::GameOver) drawGameOver();
    if (state == State::LevelUp) drawLevelUp();
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
    e.worldPos    = playerPos + sf::Vector2f(std::cos(angle)*dist, std::sin(angle)*dist);
    e.flashTimer  = 0.f;
    e.alive       = true;
    e.isBoss = false;
    e.zigzagTimer = 0.f;

    int maxType = (waveNumber < 3) ? 0 : (waveNumber < 6) ? 1 : 2;
    e.type = rand() % (maxType + 1);

    switch (e.type) {
        case 0: // Virus
            e.hp    = 30.f;
            e.maxHp = 30.f;
            e.speed = 90.f + waveNumber * 2.f;
            break;
        case 1: // Worm
            e.hp    = 15.f;
            e.maxHp = 15.f;
            e.speed = 160.f + waveNumber * 2.f;
            break;
        case 2: // Exploit
            e.hp    = 120.f;
            e.maxHp = 120.f;
            e.speed = 45.f + waveNumber * 1.f;
            break;
    }
    enemies.push_back(e);
}

void Game::startWave() {
    waveNumber++;
    if (waveNumber % 5 == 0) {
        spawnBoss();
        waveEnemiesLeft  = 0;
        waveState        = WaveState::WaitingClear;
    } else {
        waveEnemiesSpawn  = 8 + waveNumber * 3;
        waveEnemiesLeft   = waveEnemiesSpawn;
        waveSpawnTimer    = 0.f;
        waveSpawnInterval = std::max(0.4f, 1.5f - waveNumber * 0.05f);
        waveState         = WaveState::Spawning;
    }
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
        if (e.type == 1) {
            e.zigzagTimer += dt;
            sf::Vector2f toPlayer = vnorm(playerPos - e.worldPos);
            sf::Vector2f perp(-toPlayer.y, toPlayer.x);
            float zigzag = std::sin(e.zigzagTimer * 5.f) * 0.6f;
            e.worldPos += (toPlayer + perp * zigzag) * e.speed * dt;
    }   else {
            e.worldPos += vnorm(playerPos - e.worldPos) * e.speed * dt;
    }}
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

        sf::Color col;

        if (e.isBoss) {
            float pulse = 26.f + 6.f * std::sin(globalTime * 3.f);
            shape.setRadius(pulse);
            shape.setOrigin(pulse, pulse);
            shape.setFillColor(e.flashTimer > 0.f ? sf::Color::White : sf::Color(255, 180, 0));
            shape.setOutlineColor(sf::Color(255, 220, 0));
            sf::Vector2f scr = worldToScreen(e.worldPos, playerPos);
            if (!isOnScreen(scr, 40.f)) continue;
            shape.setPosition(scr);
            window.draw(shape);

    // pasek HP nad bossem
            float bw = 80.f;
            float bf = bw * (e.hp / e.maxHp);
            sf::RectangleShape bbg({bw, 6.f});
            bbg.setOrigin(bw/2.f, 0.f);
            bbg.setPosition(scr.x, scr.y - pulse - 12.f);
            bbg.setFillColor(sf::Color(60, 0, 0));
            window.draw(bbg);
            sf::RectangleShape bbf({bf, 6.f});
            bbf.setOrigin(bw/2.f, 0.f);
            bbf.setPosition(scr.x, scr.y - pulse - 12.f);
            bbf.setFillColor(sf::Color(255, 180, 0));
            window.draw(bbf);
            continue;
            }

        switch(e.type) {
            case 0: col = sf::Color(180, 30,  30);  break; // Virus
            case 1: col = sf::Color(60,  220, 120); break; // Worm
            case 2: col = sf::Color(120, 120, 140); break; // Exploit
    }
shape.setRadius(e.type == 2 ? 18.f : 12.f);
shape.setOrigin(shape.getRadius(), shape.getRadius());
shape.setFillColor(e.flashTimer > 0.f ? sf::Color::White : col);

        sf::Vector2f scr = worldToScreen(e.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        shape.setPosition(scr);
        window.draw(shape);

    }
}

void Game::checkPlayerHit() {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float eRadius = e.isBoss ? 26.f : (e.type == 2 ? 18.f : 12.f);
        if (vlen(e.worldPos - playerPos) < eRadius + 14.f) {
            if (invincTimer <= 0.f) {
                playerHp -= 10;
                invincTimer = 1.f;
                if (playerHp <= 0) state = State::GameOver;
            }
        }
    }
}

void Game::findAndShoot() {
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
                    if (e.type == 2 && frand() < 0.15f) spawnBoostOrb(e.worldPos, rand() % 5);
                    sf::Color col = e.isBoss ? sf::Color(255,180,0) :
                    e.type == 1 ? sf::Color(60,220,120) :
                    e.type == 2 ? sf::Color(120,120,140) :
                                  sf::Color(180,30,30);
                    spawnParticles(e.worldPos, col, e.isBoss ? 20 : 8);
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

    float dashBarW  = 80.f;
    float dashFill  = dashBarW * (1.f - std::min(dashCd / 2.0f, 1.f));
    sf::RectangleShape dashBg({dashBarW, 8.f});
    dashBg.setPosition(x + barW + 16.f, y);
    dashBg.setFillColor(sf::Color(20, 20, 60));
    window.draw(dashBg);
    sf::RectangleShape dashFillBar({dashFill, 8.f});
    dashFillBar.setPosition(x + barW + 16.f, y);
    dashFillBar.setFillColor(dashing ? sf::Color(255,255,255) : sf::Color(100, 180, 255));
    window.draw(dashFillBar);

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
                pendingLevelUps++;
                if (pendingLevelUps > 0) {
                    buildUpgradeChoices();
                    state = State::LevelUp;
                }
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

void Game::spawnHpOrb(sf::Vector2f pos) {
    HpOrb o; o.worldPos = pos; o.alive = true;
    hpOrbs.push_back(o);
}

void Game::spawnBoostOrb(sf::Vector2f pos, int type) {
    BoostOrb o; o.worldPos = pos; o.alive = true; o.type = type;
    boostOrbs.push_back(o);
}

void Game::updateHpOrbs(float dt) {
    hpOrbs.erase(std::remove_if(hpOrbs.begin(), hpOrbs.end(),
        [](const HpOrb& o){ return !o.alive; }), hpOrbs.end());
}

void Game::updateBoostOrbs(float dt) {
    boostOrbs.erase(std::remove_if(boostOrbs.begin(), boostOrbs.end(),
        [](const BoostOrb& o){ return !o.alive; }), boostOrbs.end());
}

void Game::checkHpPickup() {
    for (auto& o : hpOrbs) {
        if (!o.alive) continue;
        if (vlen(o.worldPos - playerPos) < 20.f) {
            o.alive   = false;
            playerHp  = std::min(playerHp + 25, 100);
        }
    }
}

void Game::checkBoostPickup() {
    for (auto& o : boostOrbs) {
        if (!o.alive) continue;
        if (vlen(o.worldPos - playerPos) < 20.f) {
            o.alive = false;
            switch (o.type) {
                case 0: boostSpeedTimer  = 15.f; break; // SpeedCore
                case 1: boostFireTimer   = 12.f; break; // Overclock
                case 2: boostMagnetTimer = 20.f; break; // DataSurge
                case 3: boostDmgTimer    =  8.f; break; // Overload
                case 4: boostGhostTimer  =  5.f; break; // GhostProtocol
            }
        }
    }
}

void Game::tickBoosts(float dt) {
    if (boostSpeedTimer  > 0.f) boostSpeedTimer  -= dt;
    if (boostFireTimer   > 0.f) boostFireTimer   -= dt;
    if (boostMagnetTimer > 0.f) boostMagnetTimer -= dt;
    if (boostDmgTimer    > 0.f) boostDmgTimer    -= dt;
    if (boostGhostTimer  > 0.f) boostGhostTimer  -= dt;
}

void Game::drawHpOrbs() {
    sf::CircleShape shape(8.f);
    shape.setOrigin(8.f, 8.f);
    shape.setFillColor(sf::Color(220, 60, 60));
    shape.setOutlineColor(sf::Color(255, 120, 120));
    shape.setOutlineThickness(1.5f);
    for (auto& o : hpOrbs) {
        if (!o.alive) continue;
        sf::Vector2f scr = worldToScreen(o.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        shape.setPosition(scr);
        window.draw(shape);
    }
}

void Game::drawBoostOrbs() {
    sf::CircleShape shape(9.f);
    shape.setOrigin(9.f, 9.f);
    shape.setOutlineThickness(1.5f);
    for (auto& o : boostOrbs) {
        if (!o.alive) continue;
        sf::Vector2f scr = worldToScreen(o.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        switch (o.type) {
            case 0: shape.setFillColor(sf::Color(255,200,0));   shape.setOutlineColor(sf::Color(255,240,100)); break;
            case 1: shape.setFillColor(sf::Color(255,100,0));   shape.setOutlineColor(sf::Color(255,160,80));  break;
            case 2: shape.setFillColor(sf::Color(0,200,255));   shape.setOutlineColor(sf::Color(100,230,255)); break;
            case 3: shape.setFillColor(sf::Color(220,0,220));   shape.setOutlineColor(sf::Color(255,100,255)); break;
            case 4: shape.setFillColor(sf::Color(220,220,220)); shape.setOutlineColor(sf::Color(255,255,255)); break;
        }
        shape.setPosition(scr);
        window.draw(shape);
    }
}

void Game::tryDash() {
    if (dashCd > 0.f || dashing) return;

    sf::Vector2f dir(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dir.x += 1.f;

    if (vlen(dir) < 0.01f) return; // brak kierunku — nie dashuj

    dashing   = true;
    dashDir   = vnorm(dir);
    dashTimer = 0.17f;
    dashCd    = 2.0f;
    invincTimer = 0.17f; // nietykalny podczas dashu
}

void Game::tickDash(float dt) {
    if (dashCd > 0.f) dashCd -= dt;

    if (!dashing) return;
    dashTimer -= dt;
    playerPos += dashDir * 750.f * dt;

    if (dashTimer <= 0.f) dashing = false;
}

void Game::resetGame() {
    playerHp       = 100;
    playerXp       = 0;
    playerLevel    = 1;
    xpToNext       = 10;
    playerPos      = {0.f, 0.f};
    fireTimer      = 0.f;
    invincTimer    = 0.f;
    dashCd         = 0.f;
    dashTimer      = 0.f;
    dashing        = false;
    waveNumber     = 0;
    waveEnemiesLeft   = 0;
    waveEnemiesSpawn  = 0;
    waveSpawnTimer    = 0.f;
    waveClearTimer    = 0.f;
    waveState      = WaveState::Countdown;
    xpSpawnTimer   = 0.f;
    hpSpawnTimer   = 0.f;
    boostSpawnTimer = 0.f;
    boostSpeedTimer = 0.f;
    boostFireTimer  = 0.f;
    boostMagnetTimer = 0.f;
    boostDmgTimer   = 0.f;
    boostGhostTimer = 0.f;
    enemies.clear();
    bullets.clear();
    xpOrbs.clear();
    hpOrbs.clear();
    boostOrbs.clear();
    playerShape.setRotation(0.f);
    waveSpawnInterval = 1.5f;
}

void Game::spawnBoss() {
    float angle = frand() * 2.f * GPI;
    float dist  = 600.f;

    Enemy e;
    e.worldPos    = playerPos + sf::Vector2f(std::cos(angle)*dist, std::sin(angle)*dist);
    e.flashTimer  = 0.f;
    e.alive       = true;
    e.isBoss      = true;
    e.zigzagTimer = 0.f;
    e.type        = 0;

    // statystyki rosną z każdym bossem
    int bossIndex = waveNumber / 5;
    e.hp    = 300.f + bossIndex * 150.f;
    e.maxHp = e.hp;
    e.speed = 55.f  + bossIndex * 5.f;

    enemies.push_back(e);
}

void Game::spawnParticles(sf::Vector2f pos, sf::Color col, int n) {
    for (int i = 0; i < n; ++i) {
        Particle p;
        p.worldPos = pos;
        float angle = frand() * 2.f * GPI;
        float spd   = frandr(60.f, 220.f);
        p.vel       = sf::Vector2f(std::cos(angle)*spd, std::sin(angle)*spd);
        p.radius    = frandr(2.f, 5.f);
        p.lifetime  = frandr(0.3f, 0.8f);
        p.maxLife   = p.lifetime;
        p.color     = col;
        p.alive     = true;
        particles.push_back(p);
    }
}

void Game::updateParticles(float dt) {
    for (auto& p : particles) {
        if (!p.alive) continue;
        p.worldPos += p.vel * dt;
        p.vel      *= 0.92f;
        p.lifetime -= dt;
        if (p.lifetime <= 0.f) p.alive = false;
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const Particle& p){ return !p.alive; }),
        particles.end()
    );
}

void Game::drawParticles() {
    sf::CircleShape shape;
    for (auto& p : particles) {
        if (!p.alive) continue;
        sf::Vector2f scr = worldToScreen(p.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        float alpha = p.lifetime / p.maxLife;
        sf::Color col = p.color;
        col.a = (sf::Uint8)(alpha * 255.f);
        shape.setRadius(p.radius * alpha);
        shape.setOrigin(p.radius * alpha, p.radius * alpha);
        shape.setFillColor(col);
        shape.setPosition(scr);
        window.draw(shape);
    }
    }

 void Game::buildUpgradeChoices() {
    upgradeChoices.clear();

    // Suma wag
    float totalWeight = 0.f;
    for (auto& u : upgradePool)
        if (u.level < u.maxLevel) totalWeight += u.weight;

    if (totalWeight <= 0.f) return;

    // Losuj 3 unikalne
    std::vector<Upgrade*> available;
    for (auto& u : upgradePool)
        if (u.level < u.maxLevel) available.push_back(&u);

    for (int i = 0; i < 3 && !available.empty(); ++i) {
        float roll = frand() * totalWeight;
        float acc  = 0.f;
        for (int j = 0; j < (int)available.size(); ++j) {
            acc += available[j]->weight;
            if (roll <= acc) {
                upgradeChoices.push_back(available[j]);
                totalWeight -= available[j]->weight;
                available.erase(available.begin() + j);
                break;
            }
        }
    }
}

void Game::applyUpgrade(int idx) {
    if (idx >= (int)upgradeChoices.size()) return;

    Upgrade* chosen = upgradeChoices[idx];
    chosen->level++;
    chosen->weight += 5.f; // częściej się pojawia po wybraniu

    // Reset wagi dla zignorowanych
    for (auto* u : upgradeChoices)
        if (u != chosen) u->weight = u->baseWeight;

    upgradeChoices.clear();
    pendingLevelUps--;

    if (pendingLevelUps > 0) {
        buildUpgradeChoices();
    } else {
        state = State::Playing;
    }
}

void Game::drawLevelUp() {
    // Przyciemnione tło
    sf::RectangleShape overlay({GW, GH});
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    if (font.getInfo().family.empty()) return;

    sf::Text title;
    title.setFont(font);
    title.setString("LEVEL UP!");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color(40, 160, 255));
    centerText(title, GW/2.f, GH*0.18f);
    window.draw(title);

    sf::Text hint;
    hint.setFont(font);
    hint.setString("[ 1 ]          [ 2 ]          [ 3 ]");
    hint.setCharacterSize(20);
    hint.setFillColor(sf::Color(160, 160, 160));
    centerText(hint, GW/2.f, GH*0.88f);
    window.draw(hint);

    // 3 karty
    float cardW = 280.f, cardH = 340.f;
    float startX = GW/2.f - cardW*1.5f - 20.f;

    for (int i = 0; i < (int)upgradeChoices.size(); ++i) {
        Upgrade* u = upgradeChoices[i];
        float cx = startX + i * (cardW + 20.f);
        float cy = GH/2.f - cardH/2.f;

        // Tło karty
        sf::RectangleShape card({cardW, cardH});
        card.setPosition(cx, cy);
        card.setFillColor(sf::Color(10, 15, 30, 220));
        card.setOutlineColor(u->color);
        card.setOutlineThickness(2.f);
        window.draw(card);

        // Nazwa
        sf::Text name;
        name.setFont(font);
        name.setString(u->name);
        name.setCharacterSize(22);
        name.setFillColor(u->color);
        centerText(name, cx + cardW/2.f, cy + 30.f);
        window.draw(name);

        // Poziom
        sf::Text lvl;
        lvl.setFont(font);
        lvl.setString("LVL " + std::to_string(u->level) + " / " + std::to_string(u->maxLevel));
        lvl.setCharacterSize(16);
        lvl.setFillColor(sf::Color(140, 140, 140));
        centerText(lvl, cx + cardW/2.f, cy + 65.f);
        window.draw(lvl);

        // Opis
        sf::Text desc;
        desc.setFont(font);
        desc.setString(u->desc);
        desc.setCharacterSize(16);
        desc.setFillColor(sf::Color(200, 200, 200));
        desc.setPosition(cx + 10.f, cy + 100.f);
        window.draw(desc);

        // Rzadkość
        std::string rarStr;
        sf::Color rarCol;
        switch(u->rarity) {
            case Rarity::Common:    rarStr="COMMON";    rarCol=sf::Color(180,180,180); break;
            case Rarity::Rare:      rarStr="RARE";      rarCol=sf::Color(40,160,255);  break;
            case Rarity::Epic:      rarStr="EPIC";      rarCol=sf::Color(180,80,255);  break;
            case Rarity::Legendary: rarStr="LEGENDARY"; rarCol=sf::Color(255,180,0);   break;
        }
        sf::Text rar;
        rar.setFont(font);
        rar.setString(rarStr);
        rar.setCharacterSize(14);
        rar.setFillColor(rarCol);
        centerText(rar, cx + cardW/2.f, cy + cardH - 30.f);
        window.draw(rar);
    }
}

