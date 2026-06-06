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
        gameTime += dt;
        // Ruch WASD
        sf::Vector2f dir(0.f, 0.f);
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))) dir.y -= 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::S))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))) dir.y += 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))) dir.x -= 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) dir.x += 1.f;
        playerPos += vnorm(dir) * PLAYER_SPD * playerSpeedMult * dt;

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

        if (playerHpRegen > 0.f) playerHp = std::min(playerMaxHp, playerHp + (int)(playerHpRegen * dt));

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
    tickWeapons(dt);
    }


}

// ── Render ────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color(2, 5, 12));
    drawBG();
    if (state == State::Playing) {drawParticles(); drawXpOrbs(); drawHpOrbs(); drawBoostOrbs(); drawEnemies(); drawBullets(); drawWeapons(); drawPlayer(); drawHUD();}
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

    int maxType;
    if      (waveNumber < 3)  maxType = 0;
    else if (waveNumber < 8)  maxType = 1;
    else if (waveNumber < 15) maxType = 2;
    else if (waveNumber < 25) maxType = 3;
    else if (waveNumber < 35) maxType = 4;
    else if (waveNumber < 45) maxType = 5;
    else                      maxType = 6;
    e.type = rand() % (maxType + 1);
    float hpMult  = 1.f + waveNumber * 0.08f;
    float spdMult = 1.f + waveNumber * 0.03f;

    switch (e.type) {
    case 0: e.hp = 30.f*hpMult;  e.maxHp=e.hp; e.speed=80.f*spdMult;  break;
    case 1: e.hp = 15.f*hpMult;  e.maxHp=e.hp; e.speed=150.f*spdMult; break;
    case 2: e.hp = 200.f*hpMult; e.maxHp=e.hp; e.speed=40.f*spdMult;  break;
    case 3: e.hp = 60.f*hpMult;  e.maxHp=e.hp; e.speed=70.f*spdMult;  break;
    case 4: e.hp = 40.f*hpMult;  e.maxHp=e.hp; e.speed=55.f*spdMult;  break;
    case 5: e.hp = 25.f*hpMult;  e.maxHp=e.hp; e.speed=0.f;           break;
    case 6: e.hp = 10.f*hpMult;  e.maxHp=e.hp; e.speed=200.f*spdMult; break;
}
    enemies.push_back(e);
}

void Game::updateWave(float dt) {
    waveTimer  += dt;
    spawnTimer += dt;

    if (waveTimer >= 30.f && waveNumber < 60) {
        waveTimer = 0.f;
        waveNumber++;
        bossSpawned = false;
        if (waveNumber % 5 == 0) {
            spawnBoss();
            bossSpawned = true;
        }
    }

    spawnInterval = std::max(0.3f, 2.0f - waveNumber * 0.025f);
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.f;
        int count = 1 + waveNumber / 10;
        for (int i = 0; i < count; ++i) spawnEnemy();
    }
}

void Game::updateEnemies(float dt) {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (e.flashTimer > 0.f) e.flashTimer -= dt;
        if (e.frozenTimer > 0.f) { e.frozenTimer -= dt; continue; }
        if (e.type == 1) {
            e.zigzagTimer += dt;
            sf::Vector2f toPlayer = vnorm(playerPos - e.worldPos);
            sf::Vector2f perp(-toPlayer.y, toPlayer.x);
            float zigzag = std::sin(e.zigzagTimer * 5.f) * 0.6f;
            e.worldPos += (toPlayer + perp * zigzag) * e.speed * dt;
}       else if (e.type == 4) {
            float d = vlen(playerPos - e.worldPos);
            float spd = d > 200.f ? e.speed : e.speed * 0.3f;
            e.worldPos += vnorm(playerPos - e.worldPos) * spd * dt;
}       else if (e.type == 5) {
            e.zigzagTimer += dt;
            if (e.zigzagTimer >= 2.f) {
                e.zigzagTimer = 0.f;
                float a = frand() * 2.f * GPI;
                float d = 80.f + frand() * 60.f;
                e.worldPos = playerPos + sf::Vector2f(std::cos(a)*d, std::sin(a)*d);
                e.flashTimer = 0.3f;
    }
}       else {
            e.worldPos += vnorm(playerPos - e.worldPos) * e.speed * dt;
}}

    for (int i = 0; i < (int)enemies.size(); ++i) {
        if (!enemies[i].alive) continue;
        float ri = enemies[i].isBoss ? 26.f : (enemies[i].type == 2 ? 18.f : 12.f);
        for (int j = i+1; j < (int)enemies.size(); ++j) {
            if (!enemies[j].alive) continue;
            float rj = enemies[j].isBoss ? 26.f : (enemies[j].type == 2 ? 18.f : 12.f);
            float minDist = ri + rj;
            sf::Vector2f diff = enemies[i].worldPos - enemies[j].worldPos;
            float d = vlen(diff);
            if (d < minDist && d > 0.01f) {
                sf::Vector2f push = vnorm(diff) * (minDist - d) * 0.5f;
                enemies[i].worldPos += push;
                enemies[j].worldPos -= push;
            }
        }
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
            case 0: col = sf::Color(180, 30,  30);  break;
            case 1: col = sf::Color(60,  220, 120); break;
            case 2: col = sf::Color(120, 120, 140); break;
            case 3: col = sf::Color(200, 120,  40); break;
            case 4: col = sf::Color(40,  160, 255); break;
            case 5: col = sf::Color(120,   0, 220); break;
            case 6: col = sf::Color(80,  180, 255); break;
            default:col = sf::Color(180,  30,  30); break;
}
    float eRad = e.isBoss ? 26.f : (e.type == 2 ? 18.f : e.type == 6 ? 8.f : 12.f);
    shape.setRadius(eRad);
    shape.setOrigin(eRad, eRad);
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
        float eRadius = e.isBoss ? 26.f : (e.type == 2 ? 18.f : e.type == 6 ? 8.f : 12.f);
        if (vlen(e.worldPos - playerPos) < eRadius + 14.f) {
            if (invincTimer <= 0.f && boostGhostTimer <= 0.f) {
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

    float effectiveCd = playerFireRate * (1.f - playerCdr);
    if (effectiveCd < 0.3f) effectiveCd = 0.3f;
    fireTimer = effectiveCd;
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
                float dmg = playerAtk * (boostDmgTimer > 0.f ? 2.f : 1.f);
                e.hp -= dmg;
                e.flashTimer = 0.1f;
                b.alive = false;
                if (e.hp <= 0.f) {
                    e.alive = false;

                    if (e.type == 3) {
                        for (int s = 0; s < 2; ++s) {
                        float a = frand() * 2.f * GPI;
                        Enemy baby;
                        baby.worldPos  = e.worldPos + sf::Vector2f(std::cos(a)*20.f, std::sin(a)*20.f);
                        baby.hp = baby.maxHp = 15.f;
                        baby.speed = 90.f;
                        baby.type = 0; baby.isBoss = false;
                        baby.alive = true; baby.flashTimer = 0.f;
                        baby.frozenTimer = 0.f; baby.zigzagTimer = 0.f;
                        enemies.push_back(baby);
                    }
                }

                    int xpVal = e.isBoss ? 50 : (e.type == 2 ? 15 : 1);
                    spawnXpOrb(e.worldPos, xpVal);
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

    int gm = (int)gameTime;
    t.setString(std::to_string(gm/60) + ":" + (gm%60 < 10 ? "0" : "") + std::to_string(gm%60));
    t.setCharacterSize(20);
    t.setFillColor(sf::Color(200, 200, 200));
    centerText(t, GW/2.f, 20.f);
    window.draw(t);

    if (font.getInfo().family.empty()) return;
    sf::Text bt;
    bt.setFont(font);
    bt.setCharacterSize(14);
    float bx = GW - 200.f;
    float by = 80.f;
    if (boostSpeedTimer  > 0.f) { bt.setString("SPEED CORE  " + std::to_string((int)boostSpeedTimer)  + "s"); bt.setFillColor(sf::Color(255,200,0));   bt.setPosition(bx,by);    window.draw(bt); by+=20.f; }
    if (boostFireTimer   > 0.f) { bt.setString("OVERCLOCK   " + std::to_string((int)boostFireTimer)   + "s"); bt.setFillColor(sf::Color(255,100,0));   bt.setPosition(bx,by);    window.draw(bt); by+=20.f; }
    if (boostMagnetTimer > 0.f) { bt.setString("DATA SURGE  " + std::to_string((int)boostMagnetTimer) + "s"); bt.setFillColor(sf::Color(0,200,255));   bt.setPosition(bx,by);    window.draw(bt); by+=20.f; }
    if (boostDmgTimer    > 0.f) { bt.setString("OVERLOAD    " + std::to_string((int)boostDmgTimer)    + "s"); bt.setFillColor(sf::Color(220,0,220));   bt.setPosition(bx,by);    window.draw(bt); by+=20.f; }
    if (boostGhostTimer  > 0.f) { bt.setString("GHOST PROTO " + std::to_string((int)boostGhostTimer)  + "s"); bt.setFillColor(sf::Color(220,220,220)); bt.setPosition(bx,by);    window.draw(bt); by+=20.f; }
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
    if      (value >= 50) orb.orbSize = 3;
    else if (value >= 15) orb.orbSize = 2;
    else if (value >= 4)  orb.orbSize = 1;
    else                  orb.orbSize = 0;
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
    const float magnetRadius = playerMagnet;
    for (auto& orb : xpOrbs) {
        if (!orb.alive) continue;
        float d = vlen(orb.worldPos - playerPos);
        if (d < magnetRadius)
            orb.worldPos += vnorm(playerPos - orb.worldPos) * 200.f * 0.016f;
        if (d < pickupRadius) {
            orb.alive = false;
            if (orb.orbSize == 3) playerHp = std::min(playerMaxHp, playerHp + 20);
            playerXp += orb.value;
            if (playerXp >= xpToNext) {
                playerXp  -= xpToNext;
                playerLevel++;
                xpToNext = 5 * std::pow(1.15f, playerLevel - 1);
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
    sf::CircleShape shape;
    for (auto& orb : xpOrbs) {
        if (!orb.alive) continue;
        sf::Vector2f scr = worldToScreen(orb.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        float r; sf::Color fill, out;
        switch (orb.orbSize) {
            case 0: r=5.f;  fill=sf::Color(40,160,255);  out=sf::Color(100,200,255); break;
            case 1: r=8.f;  fill=sf::Color(255,200,40);  out=sf::Color(255,230,100); break;
            case 2: r=11.f; fill=sf::Color(255,100,180); out=sf::Color(255,160,210); break;
            case 3: r=14.f; fill=sf::Color(60,220,80);   out=sf::Color(120,255,140); break;
            default:r=5.f;  fill=sf::Color(40,160,255);  out=sf::Color(100,200,255); break;
        }
        shape.setRadius(r);
        shape.setOrigin(r, r);
        shape.setPosition(scr);
        shape.setFillColor(fill);
        shape.setOutlineColor(out);
        shape.setOutlineThickness(1.5f);
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
    if (boostSpeedTimer  > 0.f) { boostSpeedTimer  -= dt; playerSpeedMult = 1.f + 0.5f; }
    else                          playerSpeedMult = 1.f;

    if (boostFireTimer   > 0.f) { boostFireTimer   -= dt; playerFireRate = 1.6f; }
    else                          playerFireRate = 3.2f;

    if (boostMagnetTimer > 0.f) { boostMagnetTimer -= dt; playerMagnet = 150.f + 300.f; }
    else                          playerMagnet = 150.f;

    if (boostDmgTimer    > 0.f) { boostDmgTimer    -= dt; }
    if (boostGhostTimer  > 0.f) { boostGhostTimer  -= dt; }
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
    xpToNext       = 5;
    playerPos      = {0.f, 0.f};
    fireTimer      = 0.f;
    invincTimer    = 0.f;
    dashCd         = 0.f;
    dashTimer      = 0.f;
    dashing        = false;
    waveNumber    = 0;
    waveTimer     = 0.f;
    spawnTimer    = 0.f;
    spawnInterval = 2.0f;
    bossSpawned   = false;
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
    gameTime = 0.f;
    playerAtk       = 10.f;
    playerSpeedMult = 1.f;
    playerFireRate  = 3.2f;
    playerMaxHp     = 100;
    playerMagnet    = 150.f;
    playerCdr       = 0.f;
    playerHpRegen   = 0.f;
    upgradePool     = getUpgradePool();
    pendingLevelUps = 0;
    upgradeChoices.clear();
    hasOrbitalNode = hasChainShock = hasFrostBurst = hasVirusBomb = false;
    hasStaticStorm = hasDataVortex = hasCoreDropWeapon = hasCorruptionZone = false;
    hasShockGrid   = hasPacketFlood = hasOverburn = hasCryoSweep = false;
    hasEmpFlash    = hasLaserRay = false;
    orbitalLevel = chainShockLevel = frostBurstLevel = virusBombLevel = 0;
    staticStormLevel = dataVortexLevel = coreDropLevel = corruptionLevel = 0;
    shockGridLevel = packetFloodLevel = overburnLevel = cryoSweepLevel = 0;
    empFlashLevel = laserRayLevel = 0;
    weaponEffects.clear();
    corruptionZones.clear();
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
    chosen->weight += 5.f;

    // Reset wagi zignorowanych
    for (auto* u : upgradeChoices)
        if (u != chosen) u->weight = u->baseWeight;

    // ── Faktyczne efekty ─────────────────────────────────
    const std::string& id = chosen->id;

    if (id == "ghost_process") {
        playerSpeedMult += 0.08f;               // +8% prędkości
    } else if (id == "pulse_bolt") {
        playerAtk *= 1.10f;                     // +10% obrażeń
    } else if (id == "firewall_shield") {
        playerMaxHp += 15;
        playerHp = std::min(playerHp + 15, playerMaxHp);
    } else if (id == "encryption_ring") {
        playerHpRegen += 1.f;                   // +1 HP/s
    } else if (id == "data_bolt") {
        // wielokrotne pociski — obsłużymy przy strzelaniu
        // na razie zwiększamy obrażenia jako kompensata
        playerAtk *= 1.15f;
    } else if (id == "chain_shock") {
        hasChainShock = true; chainShockLevel++;
    } else if (id == "frost_burst") {
        hasFrostBurst = true; frostBurstLevel++;
    } else if (id == "virus_bomb") {
        hasVirusBomb = true; virusBombLevel++;
    } else if (id == "orbital_node") {
        hasOrbitalNode = true; orbitalLevel++;
    } else if (id == "static_storm") {
        hasStaticStorm = true; staticStormLevel++;
    } else if (id == "data_vortex") {
        hasDataVortex = true; dataVortexLevel++;
        playerMagnet += 40.f;
    } else if (id == "core_drop") {
        hasCoreDropWeapon = true; coreDropLevel++;
    } else if (id == "corruption_zone") {
        hasCorruptionZone = true; corruptionLevel++;
    } else if (id == "shock_grid") {
        hasShockGrid = true; shockGridLevel++;
    } else if (id == "packet_flood") {
        hasPacketFlood = true; packetFloodLevel++;
    } else if (id == "overburn") {
        hasOverburn = true; overburnLevel++;
    } else if (id == "cryo_sweep") {
        hasCryoSweep = true; cryoSweepLevel++;
    } else if (id == "emp_flash") {
        hasEmpFlash = true; empFlashLevel++;
    } else if (id == "laser_ray") {
        hasLaserRay = true; laserRayLevel++;
    }

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

void Game::hitEnemiesInRadius(sf::Vector2f pos, float radius, float dmg,
                               sf::Color col, bool freeze) {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (vlen(e.worldPos - pos) < radius) {
            e.hp -= dmg;
            e.flashTimer = 0.15f;
            if (freeze) e.frozenTimer = 1.5f + frostBurstLevel * 0.3f;
            if (e.hp <= 0.f) {
                e.alive = false;
                spawnXpOrb(e.worldPos, e.isBoss ? 50 : (e.type == 2 ? 15 : 1));
                spawnParticles(e.worldPos, col, 6);
            }
        }
    }
    // Efekt wizualny
    WeaponEffect ef;
    ef.worldPos = pos;
    ef.radius   = 0.f;
    ef.alpha    = 200.f;
    ef.lifetime = 0.3f;
    ef.color    = col;
    ef.alive    = true;
    weaponEffects.push_back(ef);
}

void Game::tickWeapons(float dt) {

    // ── Orbital Node — krąży wokół gracza ────────────────
    if (hasOrbitalNode) {
        orbitalAngle += (1.5f + orbitalLevel * 0.2f) * dt;
        float orbitR = 80.f + orbitalLevel * 10.f;
        sf::Vector2f orbPos = playerPos + sf::Vector2f(
            std::cos(orbitalAngle) * orbitR,
            std::sin(orbitalAngle) * orbitR);
        // zadaje obrażenia dotykając wrogów
        for (auto& e : enemies) {
            if (!e.alive) continue;
            if (vlen(e.worldPos - orbPos) < 18.f) {
                e.hp -= (playerAtk * 0.5f) * dt;
                e.flashTimer = 0.1f;
                if (e.hp <= 0.f) {
                    e.alive = false;
                    spawnXpOrb(e.worldPos, 1);
                    spawnParticles(e.worldPos, sf::Color(255,180,0), 6);
                }
            }
        }
    }

    // ── Chain Shock — łańcuch co 2s ──────────────────────
    if (hasChainShock) {
        chainShockCd -= dt;
        if (chainShockCd <= 0.f) {
            chainShockCd = 2.0f - chainShockLevel * 0.15f;
            // znajdź najbliższego, potem łańcuch
            Enemy* first = nullptr; float bd = 1e9f;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                float d = vlen(e.worldPos - playerPos);
                if (d < 400.f && d < bd) { bd = d; first = &e; }
            }
            if (first) {
                float dmg = playerAtk * (1.f + chainShockLevel * 0.2f);
                first->hp -= dmg; first->flashTimer = 0.15f;
                // łańcuch do następnych
                sf::Vector2f lastPos = first->worldPos;
                int chains = 2 + chainShockLevel;
                for (int c = 0; c < chains; ++c) {
                    Enemy* next = nullptr; float nd = 1e9f;
                    for (auto& e : enemies) {
                        if (!e.alive || &e == first) continue;
                        float d = vlen(e.worldPos - lastPos);
                        if (d < 200.f && d < nd) { nd = d; next = &e; }
                    }
                    if (!next) break;
                    next->hp -= dmg * 0.7f; next->flashTimer = 0.15f;
                    if (next->hp <= 0.f) {
                        next->alive = false;
                        spawnXpOrb(next->worldPos, 1);
                        spawnParticles(next->worldPos, sf::Color(255,220,0), 5);
                    }
                    lastPos = next->worldPos;
                }
                if (first->hp <= 0.f) {
                    first->alive = false;
                    spawnXpOrb(first->worldPos, 1);
                    spawnParticles(first->worldPos, sf::Color(255,220,0), 5);
                }
            }
        }
    }

    // ── Frost Burst — AoE zamrożenie co 4s ───────────────
    if (hasFrostBurst) {
        frostBurstCd -= dt;
        if (frostBurstCd <= 0.f) {
            frostBurstCd = 4.0f - frostBurstLevel * 0.3f;
            float r = 120.f + frostBurstLevel * 20.f;
            hitEnemiesInRadius(playerPos, r,
                playerAtk * (1.f + frostBurstLevel * 0.3f),
                sf::Color(100, 220, 255), true);
        }
    }

    // ── Virus Bomb — eksplozja przy najbliższym wrogu co 3s ──
    if (hasVirusBomb) {
        virusBombCd -= dt;
        if (virusBombCd <= 0.f) {
            virusBombCd = 3.0f - virusBombLevel * 0.2f;
            Enemy* t = nullptr; float bd = 1e9f;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                float d = vlen(e.worldPos - playerPos);
                if (d < bd) { bd = d; t = &e; }
            }
            if (t) {
                float r = 80.f + virusBombLevel * 15.f;
                hitEnemiesInRadius(t->worldPos, r,
                    playerAtk * (1.5f + virusBombLevel * 0.3f),
                    sf::Color(255, 60, 120));
            }
        }
    }

    // ── Static Storm — losowe pioruny co 0.8s ────────────
    if (hasStaticStorm) {
        staticStormCd -= dt;
        if (staticStormCd <= 0.f) {
            staticStormCd = 0.8f - staticStormLevel * 0.05f;
            int strikes = 1 + staticStormLevel;
            for (int i = 0; i < strikes; ++i) {
                sf::Vector2f spos = playerPos + sf::Vector2f(
                    frandr(-250.f, 250.f), frandr(-250.f, 250.f));
                hitEnemiesInRadius(spos, 40.f,
                    playerAtk * (0.8f + staticStormLevel * 0.15f),
                    sf::Color(200, 100, 255));
            }
        }
    }

    // ── Data Vortex — przyciąga + obrażenia co 5s ────────
    if (hasDataVortex) {
        dataVortexCd -= dt;
        // ciągłe przyciąganie
        for (auto& e : enemies) {
            if (!e.alive || e.frozenTimer > 0.f) continue;
            float d = vlen(e.worldPos - playerPos);
            if (d < 200.f + dataVortexLevel * 30.f && d > 30.f) {
                e.worldPos += vnorm(playerPos - e.worldPos) * 60.f * dt;
            }
        }
        if (dataVortexCd <= 0.f) {
            dataVortexCd = 5.0f;
            hitEnemiesInRadius(playerPos, 80.f,
                playerAtk * (1.f + dataVortexLevel * 0.25f),
                sf::Color(255, 220, 0));
        }
    }

    // ── Core Drop — spada na losowego wroga co 3.5s ──────
    if (hasCoreDropWeapon) {
        coreDropCd -= dt;
        if (coreDropCd <= 0.f) {
            coreDropCd = 3.5f - coreDropLevel * 0.2f;
            if (!enemies.empty()) {
                int idx = rand() % enemies.size();
                if (enemies[idx].alive) {
                    hitEnemiesInRadius(enemies[idx].worldPos,
                        60.f + coreDropLevel * 10.f,
                        playerAtk * (2.f + coreDropLevel * 0.4f),
                        sf::Color(255, 100, 0));
                }
            }
        }
    }

    // ── Corruption Zone — strefa obrażeń co 6s ───────────
    if (hasCorruptionZone) {
        corruptionCd -= dt;
        if (corruptionCd <= 0.f) {
            corruptionCd = 6.f;
            CorruptionZoneObj cz;
            cz.worldPos  = playerPos;
            cz.radius    = 70.f + corruptionLevel * 15.f;
            cz.lifetime  = 4.f + corruptionLevel * 0.5f;
            cz.tickTimer = 0.f;
            cz.alive     = true;
            corruptionZones.push_back(cz);
        }
        // tick istniejących stref
        for (auto& cz : corruptionZones) {
            if (!cz.alive) continue;
            cz.lifetime  -= dt;
            cz.tickTimer -= dt;
            if (cz.lifetime <= 0.f) { cz.alive = false; continue; }
            if (cz.tickTimer <= 0.f) {
                cz.tickTimer = 0.5f;
                for (auto& e : enemies) {
                    if (!e.alive) continue;
                    if (vlen(e.worldPos - cz.worldPos) < cz.radius) {
                        e.hp -= playerAtk * 0.4f;
                        e.flashTimer = 0.1f;
                        if (e.hp <= 0.f) {
                            e.alive = false;
                            spawnXpOrb(e.worldPos, 1);
                            spawnParticles(e.worldPos, sf::Color(120,0,220), 5);
                        }
                    }
                }
            }
        }
        corruptionZones.erase(
            std::remove_if(corruptionZones.begin(), corruptionZones.end(),
                [](const CorruptionZoneObj& c){ return !c.alive; }),
            corruptionZones.end());
    }

    // ── Shock Grid — strefa wokół gracza ciągłe DMG ──────
    if (hasShockGrid) {
        shockGridCd -= dt;
        if (shockGridCd <= 0.f) {
            shockGridCd = 0.6f;
            float r = 80.f + shockGridLevel * 12.f;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                if (vlen(e.worldPos - playerPos) < r) {
                    e.hp -= playerAtk * 0.3f;
                    e.flashTimer = 0.08f;
                    if (e.hp <= 0.f) {
                        e.alive = false;
                        spawnXpOrb(e.worldPos, 1);
                        spawnParticles(e.worldPos, sf::Color(0,220,255), 4);
                    }
                }
            }
        }
    }

    // ── Packet Flood — fala pocisków co 2.5s ─────────────
    if (hasPacketFlood) {
        packetFloodCd -= dt;
        if (packetFloodCd <= 0.f) {
            packetFloodCd = 2.5f - packetFloodLevel * 0.15f;
            int count = 5 + packetFloodLevel * 2;
            for (int i = 0; i < count; ++i) {
                float a = (float)i / count * 2.f * GPI;
                Bullet b;
                b.worldPos = playerPos;
                b.dir      = sf::Vector2f(std::cos(a), std::sin(a));
                b.speed    = 400.f;
                b.lifetime = 1.2f;
                b.alive    = true;
                bullets.push_back(b);
            }
        }
    }

    // ── Overburn — ogień za graczem podczas ruchu ─────────
    if (hasOverburn) {
        overburnCd -= dt;
        sf::Vector2f dir(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
        if (vlen(dir) > 0.01f && overburnCd <= 0.f) {
            overburnCd = 0.3f;
            hitEnemiesInRadius(playerPos, 35.f + overburnLevel * 8.f,
                playerAtk * 0.4f, sf::Color(255, 80, 0));
        }
    }

    // ── Cryo Sweep — fala we wszystkich kierunkach co 5s ──
    if (hasCryoSweep) {
        cryoSweepCd -= dt;
        if (cryoSweepCd <= 0.f) {
            cryoSweepCd = 5.f - cryoSweepLevel * 0.3f;
            float r = 150.f + cryoSweepLevel * 25.f;
            hitEnemiesInRadius(playerPos, r,
                playerAtk * (1.2f + cryoSweepLevel * 0.2f),
                sf::Color(150, 230, 255), true);
        }
    }

    // ── EMP Flash — ogłuszenie wrogów co 4s ──────────────
    if (hasEmpFlash) {
        empFlashCd -= dt;
        if (empFlashCd <= 0.f) {
            empFlashCd = 4.f - empFlashLevel * 0.2f;
            float r = 200.f + empFlashLevel * 30.f;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                if (vlen(e.worldPos - playerPos) < r) {
                    e.frozenTimer = 2.f + empFlashLevel * 0.3f;
                    e.flashTimer  = 0.2f;
                }
            }
            WeaponEffect ef;
            ef.worldPos = playerPos; ef.radius = 0.f;
            ef.alpha = 220.f; ef.lifetime = 0.4f;
            ef.color = sf::Color(255, 255, 100); ef.alive = true;
            weaponEffects.push_back(ef);
        }
    }

    // ── Laser Ray — laser do najbliższego co 0.5s ─────────
    if (hasLaserRay) {
        laserRayCd -= dt;
        if (laserRayCd <= 0.f) {
            laserRayCd = 0.5f;
            Enemy* t = nullptr; float bd = 600.f;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                float d = vlen(e.worldPos - playerPos);
                if (d < bd) { bd = d; t = &e; }
            }
            if (t) {
                t->hp -= playerAtk * (0.6f + laserRayLevel * 0.15f);
                t->flashTimer = 0.1f;
                if (t->hp <= 0.f) {
                    t->alive = false;
                    spawnXpOrb(t->worldPos, 1);
                    spawnParticles(t->worldPos, sf::Color(255,40,40), 6);
                }
                // efekt wizualny linii
                WeaponEffect ef;
                ef.worldPos = t->worldPos;
                ef.radius   = bd;
                ef.alpha    = 200.f;
                ef.lifetime = 0.08f;
                ef.color    = sf::Color(255, 40, 40);
                ef.alive    = true;
                weaponEffects.push_back(ef);
            }
        }
    }

    // tick efektów wizualnych
    for (auto& ef : weaponEffects) {
        ef.lifetime -= dt;
        ef.alpha    -= dt * 600.f;
        ef.radius   += dt * 200.f;
        if (ef.lifetime <= 0.f || ef.alpha <= 0.f) ef.alive = false;
    }
    weaponEffects.erase(
        std::remove_if(weaponEffects.begin(), weaponEffects.end(),
            [](const WeaponEffect& e){ return !e.alive; }),
        weaponEffects.end());
}

void Game::drawWeapons() {
    // ── Orbital Node ──────────────────────────────────────
    if (hasOrbitalNode) {
        float orbitR = 80.f + orbitalLevel * 10.f;
        sf::Vector2f orbPos = playerPos + sf::Vector2f(
            std::cos(orbitalAngle) * orbitR,
            std::sin(orbitalAngle) * orbitR);
        sf::Vector2f scr = worldToScreen(orbPos, playerPos);
        sf::CircleShape orb(10.f + orbitalLevel * 2.f);
        orb.setOrigin(orb.getRadius(), orb.getRadius());
        orb.setPosition(scr);
        orb.setFillColor(sf::Color(255, 180, 0, 200));
        orb.setOutlineColor(sf::Color(255, 220, 100));
        orb.setOutlineThickness(2.f);
        window.draw(orb);
    }

    // ── Shock Grid — pulsujący okrąg wokół gracza ────────
    if (hasShockGrid) {
        float pulse = 0.6f + 0.4f * std::sin(globalTime * 8.f);
        sf::CircleShape grid(80.f + shockGridLevel * 12.f);
        grid.setOrigin(grid.getRadius(), grid.getRadius());
        grid.setPosition(GW/2.f, GH/2.f);
        grid.setFillColor(sf::Color::Transparent);
        grid.setOutlineColor(sf::Color(0, 220, 255, (sf::Uint8)(pulse * 120.f)));
        grid.setOutlineThickness(2.f);
        window.draw(grid);
    }

    // ── Data Vortex — okrąg przyciągania ─────────────────
    if (hasDataVortex) {
        float pulse = 0.5f + 0.5f * std::sin(globalTime * 3.f);
        sf::CircleShape vortex(200.f + dataVortexLevel * 30.f);
        vortex.setOrigin(vortex.getRadius(), vortex.getRadius());
        vortex.setPosition(GW/2.f, GH/2.f);
        vortex.setFillColor(sf::Color::Transparent);
        vortex.setOutlineColor(sf::Color(255, 220, 0, (sf::Uint8)(pulse * 60.f)));
        vortex.setOutlineThickness(1.f);
        window.draw(vortex);
    }

    // ── Corruption Zones ──────────────────────────────────
    for (auto& cz : corruptionZones) {
        if (!cz.alive) continue;
        sf::Vector2f scr = worldToScreen(cz.worldPos, playerPos);
        float alpha = (cz.lifetime / (4.f + corruptionLevel * 0.5f)) * 120.f;
        sf::CircleShape zone(cz.radius);
        zone.setOrigin(cz.radius, cz.radius);
        zone.setPosition(scr);
        zone.setFillColor(sf::Color(120, 0, 220, (sf::Uint8)alpha));
        zone.setOutlineColor(sf::Color(180, 60, 255, 180));
        zone.setOutlineThickness(2.f);
        window.draw(zone);
    }

    // ── Efekty fale eksplozji) ───────────────────────
    for (auto& ef : weaponEffects) {
        if (!ef.alive) continue;
        sf::Vector2f scr = worldToScreen(ef.worldPos, playerPos);
        sf::CircleShape ring(ef.radius);
        ring.setOrigin(ef.radius, ef.radius);
        ring.setPosition(scr);
        ring.setFillColor(sf::Color::Transparent);
        sf::Color c = ef.color;
        c.a = (sf::Uint8)std::max(0.f, ef.alpha);
        ring.setOutlineColor(c);
        ring.setOutlineThickness(3.f);
        window.draw(ring);
    }

    // ── Laser Ray — linia do celu ─────────────────────────
    if (hasLaserRay && !weaponEffects.empty()) {
        for (auto& ef : weaponEffects) {
            if (!ef.alive || ef.color != sf::Color(255,40,40)) continue;
            sf::Vector2f scrP(GW/2.f, GH/2.f);
            sf::Vector2f scrT = worldToScreen(ef.worldPos, playerPos);
            sf::Color lc(255, 40, 40, (sf::Uint8)std::max(0.f, ef.alpha));
            sf::Vertex line[] = { {scrP, lc}, {scrT, lc} };
            window.draw(line, 2, sf::Lines);
        }
    }
}

