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
    initMatrixDrops();
    sound.loadAll();
    sound.playMusic("menu_ambient.wav", true);
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
    playerShape.setPoint(0, {  0.f, -40.f });
    playerShape.setPoint(1, { 28.f,  16.f });
    playerShape.setPoint(2, { 18.f,  36.f });
    playerShape.setPoint(3, {-18.f,  36.f });
    playerShape.setPoint(4, {-28.f,  16.f });
    playerShape.setFillColor(sf::Color(0, 20, 30));
    playerShape.setOutlineColor(COL_CORE);
    playerShape.setOutlineThickness(3.f);
}

// ── Zdarzenia ─────────────────────────────────────────────────
void Game::handleEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) window.close();
        if (ev.type == sf::Event::KeyPressed) {
            if (ev.key.code == sf::Keyboard::Escape) {
                if (state == State::Playing) { state = State::Paused; paused = true; pauseSelected = 0; sound.play("pause"); }
                else if (state == State::Paused) { state = State::Playing; paused = false; sound.play("unpause"); }
                else window.close();
            }
            if (ev.key.code == sf::Keyboard::Return && state == State::Menu) {
                state = State::ClassSelect;
                sound.play("ui_select");
            }
            if (ev.key.code == sf::Keyboard::Return && state == State::GameOver) {
                resetGame();
                state = State::Menu;
                sound.playMusic("menu_ambient.wav", true);
            }
            if (state == State::LevelUp) {
                if (ev.key.code == sf::Keyboard::Num1) applyUpgrade(0);
                if (ev.key.code == sf::Keyboard::Num2) applyUpgrade(1);
                if (ev.key.code == sf::Keyboard::Num3) applyUpgrade(2);
                if (ev.key.code == sf::Keyboard::Num4) applyUpgrade(3);
                if (ev.key.code == sf::Keyboard::Num5) applyUpgrade(4);
            }
            if (state == State::Paused) {
                if (ev.key.code == sf::Keyboard::Up)   pauseSelected = (pauseSelected + 1) % 2;
                if (ev.key.code == sf::Keyboard::Down) pauseSelected = (pauseSelected + 1) % 2;
                if (ev.key.code == sf::Keyboard::Return) {
                    if (pauseSelected == 0) { state = State::Playing; paused = false; sound.play("unpause"); }
                    else { resetGame(); state = State::Menu; paused = false; sound.play("ui_select"); sound.playMusic("menu_ambient.wav", true); }
                }
            }
            if (state == State::ClassSelect) {
                PlayerClass map[5] = {
                    PlayerClass::Payload, PlayerClass::Firewall,
                    PlayerClass::Packet,  PlayerClass::Daemon,
                    PlayerClass::Exploit
                };
                int idx = -1;
                if (ev.key.code == sf::Keyboard::Num1) idx = 0;
                if (ev.key.code == sf::Keyboard::Num2) idx = 1;
                if (ev.key.code == sf::Keyboard::Num3) idx = 2;
                if (ev.key.code == sf::Keyboard::Num4) idx = 3;
                if (ev.key.code == sf::Keyboard::Num5) idx = 4;
                if (idx >= 0) {
                    selectedClass = map[idx];
                    resetGame();
                    applyClass(selectedClass);
                    state = State::Playing;
                    if (idx >= 0) {
                    selectedClass = map[idx];
                    resetGame();
                    applyClass(selectedClass);
                    state = State::Playing;
                    sound.play("ui_select");
                    sound.playMusic("gameplay_music.ogg", true);
                }
                }
            }
        }
        // Kliknięcie myszy — wybór klasy
        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left &&
            state == State::ClassSelect) {
            sf::Vector2f mp((float)ev.mouseButton.x, (float)ev.mouseButton.y);
            float cardW = 200.f, cardH = 300.f;
            float startX = GW/2.f - 2.5f * cardW - 2.f * 10.f;
            for (int i = 0; i < 5; ++i) {
                float cx = startX + i * (cardW + 10.f);
                float cy = GH/2.f - cardH/2.f;
                if (mp.x >= cx && mp.x <= cx+cardW && mp.y >= cy && mp.y <= cy+cardH) {
                    PlayerClass pcs[] = {PlayerClass::Payload, PlayerClass::Firewall,
                        PlayerClass::Packet, PlayerClass::Daemon,
                        PlayerClass::Exploit};
                        selectedClass = pcs[i];
                        resetGame();
                        applyClass(selectedClass);
                        state = State::Playing;
                        selectedClass = pcs[i];
                        resetGame();
                        applyClass(selectedClass);
                        state = State::Playing;
                        sound.play("ui_select");
                        sound.playMusic("gameplay_music.ogg", true);
                    }
                }
            }

        // Kliknięcie myszy — wybór ulepszenia
        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left &&
            state == State::LevelUp) {
            sf::Vector2f mp((float)ev.mouseButton.x, (float)ev.mouseButton.y);
            float cardW = 220.f, cardH = 340.f;
            float totalW = 5.f * cardW + 4.f * 15.f;
            float startX = GW/2.f - totalW/2.f;
            for (int i = 0; i < (int)upgradeChoices.size(); ++i) {
                float cx = startX + i * (cardW + 15.f);
                float cy = GH/2.f - cardH/2.f;
                if (mp.x >= cx && mp.x <= cx+cardW && mp.y >= cy && mp.y <= cy+cardH)
                    applyUpgrade(i);
            }
        }
        // Hover myszy — klasy
        if (ev.type == sf::Event::MouseMoved && state == State::ClassSelect) {
            sf::Vector2f mp((float)ev.mouseMove.x, (float)ev.mouseMove.y);
            float cardW = 200.f, cardH = 300.f;
            float startX = GW/2.f - 2.5f * cardW - 2.f * 10.f;
            hoveredClass = -1;
            for (int i = 0; i < 5; ++i) {
                float cx = startX + i * (cardW + 10.f);
                float cy = GH/2.f - cardH/2.f;
                if (mp.x >= cx && mp.x <= cx+cardW && mp.y >= cy && mp.y <= cy+cardH)
                hoveredClass = i;
                if (ev.type == sf::Event::MouseMoved && state == State::ClassSelect) {
    sf::Vector2f mp((float)ev.mouseMove.x, (float)ev.mouseMove.y);
    float cardW = 200.f, cardH = 300.f;
    float startX = GW/2.f - 2.5f * cardW - 2.f * 10.f;

    int oldHover = hoveredClass;
    hoveredClass = -1;

    for (int i = 0; i < 5; ++i) {
        float cx = startX + i * (cardW + 10.f);
        float cy = GH/2.f - cardH/2.f;

        if (mp.x >= cx && mp.x <= cx + cardW &&
            mp.y >= cy && mp.y <= cy + cardH)
        {
            hoveredClass = i;
        }
    }

    if (hoveredClass != oldHover && hoveredClass != -1) {
        sound.play("ui_hover");
    }
}

            }
        }
    }
}

// ── Update ────────────────────────────────────────────────────
void Game::update(float dt) {
    globalTime += dt;
    bgPulse += dt;
    if (state == State::Paused) return;
    if (state == State::Playing) {
        if (!bossArenaActive) gameTime += dt;
        // Ruch WASD
        sf::Vector2f dir(0.f, 0.f);
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))) dir.y -= 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::S))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))) dir.y += 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))) dir.x -= 1.f;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D))||(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) dir.x += 1.f;
        if (vlen(dir) > 0.01f) {
            sound.playLoop("move_loop");
        } else {
            sound.stopLoop("move_loop");
        }
        playerPos += vnorm(dir) * PLAYER_SPD * playerSpeedMult * dt;
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
        updateEnemyBullets(dt);
        if (invincTimer > 0.f) invincTimer -= dt;
        if (playerHpRegen > 0.f) {
            static float hpRegenAcc = 0.f;
            hpRegenAcc += playerHpRegen * dt;
            if (hpRegenAcc >= 1.f) {
                int toRegen = (int)hpRegenAcc;
                playerHp = std::min(playerMaxHp, playerHp + toRegen);
                hpRegenAcc -= (float)toRegen;
            }
        }

        checkPlayerHit();
        if (playerHp <= playerMaxHp / 4) {
            sound.playLoop("low_hp_alarm");
        } else {
            sound.stopLoop("low_hp_alarm");
        }
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
    updateFireTrails(dt);
    updateBossArena();
    recycleStars();
    scanlineOffset += 60.f * dt;
    updateMatrixDrops(dt);
    if (screenFlashTimer > 0.f) screenFlashTimer -= dt;
    }


}

// ── Render ────────────────────────────────────────────────────
void Game::render() {
    window.clear(sf::Color(2, 5, 12));
    drawBG();
    if (state == State::Playing || state == State::Paused) {
        drawParticles(); drawBossArena(); drawXpOrbs(); drawHpOrbs(); drawBoostOrbs();
        drawEnemies(); drawFireTrails(); drawBullets(); drawEnemyBullets(); drawWeapons();
        drawPlayer(); drawHUD(); drawMinimap();
    }
    if (state == State::Paused)  drawPauseMenu();
    if (state == State::Menu)    drawMenu();
    if (state == State::ClassSelect) drawClassSelect();
    if (state == State::GameOver) drawGameOver();
    if (state == State::LevelUp) drawLevelUp();

    // Scanlines
    {
        float off = std::fmod(scanlineOffset, 4.f);
        for (float y = off; y < GH; y += 4.f) {
            sf::Vertex line[2] = {
                { sf::Vector2f(0.f, y), sf::Color(0, 0, 0, 14) },
                { sf::Vector2f(GW,  y), sf::Color(0, 0, 0, 14) }
            };
            window.draw(line, 2, sf::Lines);
        }
    }
    // Screen flash
    if (screenFlashTimer > 0.f) {
        float a = screenFlashTimer / screenFlashMax;
        sf::RectangleShape fl({GW, GH});
        sf::Color fc = screenFlashCol;
        fc.a = (sf::Uint8)(a * 110.f);
        fl.setFillColor(fc);
        window.draw(fl);
    }

    window.display();
}

void Game::drawBG() {
    float t = globalTime;

    // ── Nebule ────────────────────────────────────────────
    static const struct { float wx, wy, r; sf::Color col; } nebulae[] = {
        { -800.f, -400.f, 320.f, sf::Color(0,   80, 180) },
        {  600.f,  300.f, 280.f, sf::Color(80,   0, 160) },
        { -200.f,  700.f, 250.f, sf::Color(0,  140,  80) },
        {  900.f, -600.f, 300.f, sf::Color(120,  0,  80) },
        {-1200.f,  200.f, 360.f, sf::Color(0,   60, 140) },
    };
    for (auto& nb : nebulae) {
        sf::Vector2f scr = worldToScreen({nb.wx, nb.wy}, playerPos);
        float pulse = 0.5f + 0.5f * std::sin(bgPulse * 0.3f + nb.wx * 0.001f);
        sf::Color c = nb.col;
        c.a = (sf::Uint8)(20.f + 10.f * pulse);
        sf::CircleShape neb(nb.r); neb.setOrigin(nb.r, nb.r);
        neb.setPosition(scr); neb.setFillColor(c);
        window.draw(neb);
    }

    // ── Hex grid (zamiast prostej siatki) ─────────────────
    float hexR = 70.f;
    float ox = std::fmod(playerPos.x, hexR * 1.73f);
    float oy = std::fmod(playerPos.y, hexR * 1.5f);
    for (int hx = -2; hx < (int)(GW / (hexR * 1.73f)) + 3; hx++) {
        for (int hy = -2; hy < (int)(GH / (hexR * 1.5f)) + 3; hy++) {
            float sx = hx * hexR * 1.73f + (hy % 2) * hexR * 0.866f - ox + GW/2.f;
            float sy = hy * hexR * 1.5f  - oy + GH/2.f;
            float phase = std::sin(t * 0.7f + (hx + hy) * 0.5f);
            float alpha = 5.f + 4.f * phase;
            if (alpha < 0.f) alpha = 0.f;
            sf::Color hc(0, (sf::Uint8)(150 + 60 * phase), 255, (sf::Uint8)alpha);
            sf::ConvexShape hex; hex.setPointCount(6);
            for (int i = 0; i < 6; i++) {
                float a = i * GPI / 3.f;
                hex.setPoint(i, sf::Vector2f(sx + std::cos(a) * hexR * 0.88f,
                                             sy + std::sin(a) * hexR * 0.88f));
            }
            hex.setFillColor(sf::Color::Transparent);
            hex.setOutlineThickness(0.7f);
            hex.setOutlineColor(hc);
            window.draw(hex);
        }
    }

    // ── Matrix rain ───────────────────────────────────────
    if (!font.getInfo().family.empty()) {
        for (auto& d : matrixDrops) {
            sf::Text ch(std::string(1, d.ch), font, 10);
            ch.setFillColor(sf::Color(0, 200, 80, (sf::Uint8)(d.alpha * 100.f)));
            ch.setPosition(d.x, d.y);
            window.draw(ch);
        }
    }

    // ── Gwiazdy z kolorem ─────────────────────────────────
    sf::CircleShape dot;
    for (auto& s : stars) {
        sf::Vector2f scr = worldToScreen(s.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        float tw = s.bright * (0.6f + 0.4f * std::sin(globalTime * s.twSpd + s.twPhase));
        sf::Color sc = (tw > 0.7f)
            ? sf::Color(80, 200, 255, (sf::Uint8)(tw * 200.f))
            : sf::Color(200, 220, 255, (sf::Uint8)(tw * 200.f));
        dot.setRadius(s.r); dot.setOrigin(s.r, s.r);
        dot.setPosition(scr); dot.setFillColor(sc);
        window.draw(dot);
    }

    // ── Pulsująca ramka w kolorze klasy ───────────────────
    sf::Color outlineCol = playerShape.getOutlineColor();
    float pulse = 0.4f + 0.6f * std::abs(std::sin(t * 1.5f));
    for (int i = 0; i < 3; i++) {
        sf::RectangleShape bord(sf::Vector2f(GW - 4.f - i*4.f, GH - 4.f - i*4.f));
        bord.setPosition(2.f + i*2.f, 2.f + i*2.f);
        bord.setFillColor(sf::Color::Transparent);
        bord.setOutlineThickness(1.f);
        sf::Color bc = outlineCol;
        bc.a = (sf::Uint8)(130 * pulse / (i + 1));
        bord.setOutlineColor(bc);
        window.draw(bord);
    }

    // ── Core pulse w centrum ekranu ───────────────────────
    float cp = 0.5f + 0.5f * std::sin(t * 2.5f);
    sf::CircleShape core(28.f * cp); core.setOrigin(28.f * cp, 28.f * cp);
    core.setPosition(GW/2.f, GH/2.f);
    core.setFillColor(sf::Color(0, 255, 180, (sf::Uint8)(15 * cp)));
    core.setOutlineThickness(1.5f);
    sf::Color coreOut(0, 200, 180, (sf::Uint8)(35 * cp));
    core.setOutlineColor(coreOut);
    window.draw(core);
}

void Game::drawPlayer() {
    sf::Vector2f scr(GW/2.f, GH/2.f);
    float t = globalTime;

    bool visible = true;
    if (invincTimer > 0.f) visible = (int)(invincTimer * 12.f) % 2 == 0;

    // Multi-layer glow (4 warstwy)
    sf::Color gc = playerShape.getOutlineColor();
    float glowPulse = 0.7f + 0.3f * std::sin(t * 3.2f);
    for (int g = 4; g >= 1; g--) {
        float gr = 28.f * (1.6f + g * 0.5f) * glowPulse;
        sf::CircleShape glow(gr); glow.setOrigin(gr, gr);
        glow.setPosition(scr);
        sf::Color gc2 = gc; gc2.a = (sf::Uint8)(10 * g);
        glow.setFillColor(gc2);
        window.draw(glow);
    }

    if (visible) {
        playerShape.setPosition(scr);
        window.draw(playerShape);
        // Centralny biały punkt
        sf::CircleShape core(3.5f); core.setOrigin(3.5f, 3.5f);
        core.setPosition(scr);
        core.setFillColor(sf::Color::White);
        window.draw(core);
    }
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
    float dist  = 900.f + frand() * 400.f;

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
    case 2: e.hp = 35.f*hpMult;  e.maxHp=e.hp; e.speed=60.f*spdMult;  break;
    case 3: e.hp = 60.f*hpMult;  e.maxHp=e.hp; e.speed=70.f*spdMult;  break;
    case 4: e.hp = 40.f*hpMult;  e.maxHp=e.hp; e.speed=55.f*spdMult;  break;
    case 5: e.hp = 25.f*hpMult;  e.maxHp=e.hp; e.speed=0.f;           break;
    case 6: e.hp = 10.f*hpMult;  e.maxHp=e.hp; e.speed=200.f*spdMult; break;
}
    enemies.push_back(e);
}

void Game::updateWave(float dt) {
    if (!bossArenaActive) {
    waveTimer  += dt;
    spawnTimer += dt;
}
if (bossArenaActive) return;

    if (waveTimer >= 30.f && waveNumber < 60) {
        waveTimer = 0.f;
        waveNumber++;
        bossSpawned = false;
        sound.play("wave_start");
        if (waveNumber % 5 == 0) {
            spawnBoss();
            bossSpawned = true;
        }
    }

    spawnInterval = std::max(0.15f, 3.5f - waveNumber * 0.025f);
    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.f;
        int count = 3 + waveNumber / 4;
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
}       else if (e.type == 2) {

            float d = vlen(playerPos - e.worldPos);
            if (d > 350.f) {
                e.worldPos += vnorm(playerPos - e.worldPos) * e.speed * dt;
            } else if (d < 250.f) {
                e.worldPos -= vnorm(playerPos - e.worldPos) * e.speed * dt;
            }
            e.zigzagTimer -= dt;
            if (e.zigzagTimer <= 0.f) {
                e.zigzagTimer = 1.8f;
                EnemyBullet eb;
                eb.worldPos = e.worldPos;
                eb.dir      = vnorm(playerPos - e.worldPos);
                eb.speed    = 320.f;
                eb.lifetime = 3.f;
                eb.alive    = true;
                enemyBullets.push_back(eb);
                enemyBullets.push_back(eb);
                sound.play("enemy_shoot");
            }
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
                e.flashTimer = 0.3f;
                sound.play("teleport");
    }
}       else {
            e.worldPos += vnorm(playerPos - e.worldPos) * e.speed * dt;
}}

    for (int i = 0; i < (int)enemies.size(); ++i) {
        if (!enemies[i].alive) continue;
        float ri = enemies[i].isBoss ? 120.f : (enemies[i].type == 2 ? 22.f : (enemies[i].type == 6 ? 16.f : 24.f));
        for (int j = i+1; j < (int)enemies.size(); ++j) {
            if (!enemies[j].alive) continue;
            float rj = enemies[j].isBoss ? 120.f : (enemies[j].type == 2 ? 22.f : (enemies[j].type == 6 ? 16.f : 24.f));
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
    sf::CircleShape shape(24.f);
    shape.setOrigin(24.f, 24.f);
    shape.setOutlineColor(sf::Color(255, 60, 60));
    shape.setOutlineThickness(3.f);
    for (auto& e : enemies) {
        if (!e.alive) continue;
        e.glowPhase += 0.016f * 3.f; // nie idealnie dt, ale wystarczy

        sf::Color col;

        if (e.isBoss) {
            // Boss — 5x większy niż bazowy (bazowy=24 => boss=120 + puls)
            float pulse = 120.f + 15.f * std::sin(globalTime * 2.f);
            // Zewnętrzna poświata bossa
            sf::CircleShape glow(pulse + 30.f);
            glow.setOrigin(pulse + 30.f, pulse + 30.f);
            sf::Vector2f scr = worldToScreen(e.worldPos, playerPos);
            if (!isOnScreen(scr, 200.f)) continue;
            glow.setPosition(scr);
            glow.setFillColor(sf::Color(255, 140, 0, 18));
            window.draw(glow);
            // Środkowy pierścień
            sf::CircleShape ring(pulse + 10.f);
            ring.setOrigin(pulse + 10.f, pulse + 10.f);
            ring.setPosition(scr);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(255, 200, 0, 120));
            ring.setOutlineThickness(3.f);
            window.draw(ring);
            // Ciało
            shape.setRadius(pulse);
            shape.setOrigin(pulse, pulse);
            shape.setFillColor(e.flashTimer > 0.f ? sf::Color::White : sf::Color(200, 80, 0));
            shape.setOutlineColor(sf::Color(255, 220, 0));
            shape.setOutlineThickness(4.f);
            shape.setPosition(scr);
            window.draw(shape);
            // Pasek HP
            float bw = 300.f;
            float bf = bw * (e.hp / e.maxHp);
            sf::RectangleShape bbg({bw, 12.f});
            bbg.setOrigin(bw/2.f, 0.f);
            bbg.setPosition(scr.x, scr.y - pulse - 20.f);
            bbg.setFillColor(sf::Color(60, 0, 0));
            window.draw(bbg);
            sf::RectangleShape bbf({bf, 12.f});
            bbf.setOrigin(bw/2.f, 0.f);
            bbf.setPosition(scr.x, scr.y - pulse - 20.f);
            bbf.setFillColor(sf::Color(255, 180, 0));
            window.draw(bbf);
            if (!font.getInfo().family.empty()) {
                sf::Text bossLbl;
                bossLbl.setFont(font);
                bossLbl.setString("BOSS");
                bossLbl.setCharacterSize(18);
                bossLbl.setFillColor(sf::Color(255, 220, 0));
                centerText(bossLbl, scr.x, scr.y - pulse - 44.f);
                window.draw(bossLbl);
            }
            continue;
        }

        switch(e.type) {
            case 0: col = sf::Color(220, 40,  40);  break;
            case 1: col = sf::Color(60,  240, 120); break;
            case 2: col = sf::Color(160,  40, 220); break;
            case 3: col = sf::Color(220, 140,  40); break;
            case 4: col = sf::Color(40,  180, 255); break;
            case 5: col = sf::Color(160,   0, 255); break;
            case 6: col = sf::Color(80,  200, 255); break;
            default:col = sf::Color(220,  40,  40); break;
        }
        // Rozmiary 2x: bazowy 24, duży 36, mały 16
        float eRad = (e.type == 2) ? 22.f : (e.type == 6 ? 16.f : 24.f);

        // Poświata wokół wroga
        float glowR = eRad + 10.f + 4.f * std::sin(e.glowPhase);
        sf::CircleShape eGlow(glowR);
        eGlow.setOrigin(glowR, glowR);
        sf::Vector2f scr = worldToScreen(e.worldPos, playerPos);
        if (!isOnScreen(scr, 60.f)) continue;
        eGlow.setPosition(scr);
        sf::Color gc = col; gc.a = 35;
        eGlow.setFillColor(gc);
        window.draw(eGlow);

        shape.setRadius(eRad);
        shape.setOrigin(eRad, eRad);
        shape.setFillColor(e.flashTimer > 0.f ? sf::Color::White : col);
        shape.setOutlineColor(sf::Color(col.r, col.g, col.b, 180));
        shape.setOutlineThickness(2.f);
        shape.setPosition(scr);
        window.draw(shape);

        if (e.hp < e.maxHp) {
            float bw = eRad * 2.f;
            float bf = bw * (e.hp / e.maxHp);
            sf::RectangleShape hbg({bw, 4.f});
            hbg.setOrigin(bw/2.f, 0.f);
            hbg.setPosition(scr.x, scr.y - eRad - 8.f);
            hbg.setFillColor(sf::Color(60, 0, 0));
            window.draw(hbg);
            sf::RectangleShape hbf({bf, 4.f});
            hbf.setOrigin(bw/2.f, 0.f);
            hbf.setPosition(scr.x, scr.y - eRad - 8.f);
            hbf.setFillColor(sf::Color(220, 60, 60));
            window.draw(hbf);
        }

        if (e.frozenTimer > 0.f) {
            float fr = eRad + 6.f;
            sf::CircleShape frost(fr);
            frost.setOrigin(fr, fr);
            frost.setPosition(scr);
            frost.setFillColor(sf::Color(120, 220, 255, 60));
            frost.setOutlineColor(sf::Color(180, 240, 255, 180));
            frost.setOutlineThickness(2.f);
            window.draw(frost);

            for (int k = 0; k < 4; ++k) {
                float a = k * (GPI / 2.f) + e.glowPhase * 0.3f;
                sf::CircleShape shard(2.5f);
                shard.setOrigin(2.5f, 2.5f);
                shard.setPosition(scr.x + std::cos(a) * fr, scr.y + std::sin(a) * fr);
                shard.setFillColor(sf::Color(200, 245, 255, 220));
                window.draw(shard);
            }
        }
    }
}

void Game::checkPlayerHit() {
    for (auto& e : enemies) {
        if (!e.alive) continue;
        float eRadius = e.isBoss ? 120.f : (e.type == 2 ? 22.f : (e.type == 6 ? 16.f : 24.f));
        if (vlen(e.worldPos - playerPos) < eRadius + 28.f) {
            if (invincTimer <= 0.f && boostGhostTimer <= 0.f) {
                 playerHp -= 10;
                invincTimer = 1.f;
                screenFlashTimer = 0.30f;
                screenFlashMax   = 0.30f;
                screenFlashCol   = sf::Color(220, 20, 20);
                sound.play("player_hit");
                if (playerHp <= 0) {
                    state = State::GameOver;
                    screenFlashTimer = 1.0f;
                    screenFlashMax   = 1.0f;
                    screenFlashCol   = sf::Color(200, 0, 0);
                    sound.play("game_over");
                    sound.stopLoop("move_loop");
                    sound.stopLoop("low_hp_alarm");
                    sound.playMusic("menu_ambient.wav", true);
                }
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
    if (effectiveCd < 0.12f) effectiveCd = 0.12f;
    fireTimer = effectiveCd;
    Bullet b;
    b.worldPos = playerPos;
    b.dir      = vnorm(target->worldPos - playerPos);
    b.speed    = 700.f;
    b.lifetime = 2.5f;
    b.alive    = true;
    b.color    = bulletColor;
    bullets.push_back(b);
    sound.play("shoot");
}

void Game::updateBullets(float dt) {
    for (auto& b : bullets) {
        if (!b.alive) continue;
        b.worldPos += b.dir * b.speed * dt;
        b.lifetime -= dt;
        if (b.lifetime <= 0.f) { b.alive = false; continue; }

        for (auto& e : enemies) {
            if (!e.alive) continue;
            float eR = e.isBoss ? 120.f : (e.type == 2 ? 22.f : (e.type == 6 ? 16.f : 24.f));
            if (vlen(b.worldPos - e.worldPos) < 10.f + eR) {
                float dmg = playerAtk * 1.5f * (boostDmgTimer > 0.f ? 2.f : 1.f);
                e.hp -= dmg;
                e.flashTimer = 0.1f;
                b.alive = false;
                sound.play("hit_enemy");
                if (e.hp <= 0.f) {
                    e.alive = false;
                    totalKills++;
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
                    sound.play("split_enemy");
                }
                    int xpVal = e.isBoss ? 50 : (e.type == 2 ? 3 : 1);
                    spawnXpOrb(e.worldPos, xpVal);
                    if (e.type == 2 && frand() < 0.10f) spawnBoostOrb(e.worldPos, rand() % 5);
                    sf::Color col = e.isBoss ? sf::Color(255,180,0) :
                    e.type == 1 ? sf::Color(60,220,120) :
                    e.type == 2 ? sf::Color(120,120,140) :
                                  sf::Color(180,30,30);
                    spawnParticles(e.worldPos, col, e.isBoss ? 20 : 8);
                    sound.play(e.isBoss ? "boss_death" : "enemy_death");
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

    for (auto& b : bullets) {
        if (!b.alive) continue;
        sf::Vector2f scr = worldToScreen(b.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;
        // Poświata pocisku
        sf::CircleShape glow(14.f);
        glow.setOrigin(14.f, 14.f);
        glow.setPosition(scr);
        sf::Color gc = b.color; gc.a = 50;
        glow.setFillColor(gc);
        window.draw(glow);
        // Pocisk
        sf::CircleShape shape(7.f);
        shape.setOrigin(7.f, 7.f);
        shape.setFillColor(b.color);
        shape.setOutlineColor(sf::Color(255,255,255,80));
        shape.setOutlineThickness(1.f);
        shape.setPosition(scr);
        window.draw(shape);
    }
}

void Game::drawEnemyBullets() {
    for (auto& b : enemyBullets) {
        if (!b.alive) continue;
        sf::Vector2f scr = worldToScreen(b.worldPos, playerPos);
        if (!isOnScreen(scr, 20.f)) continue;

        sf::CircleShape glow(12.f);
        glow.setOrigin(12.f, 12.f);
        glow.setPosition(scr);
        glow.setFillColor(sf::Color(160, 40, 220, 60));
        window.draw(glow);

        sf::CircleShape shape(6.f);
        shape.setOrigin(6.f, 6.f);
        shape.setFillColor(sf::Color(200, 80, 255));
        shape.setOutlineColor(sf::Color(255, 200, 255, 150));
        shape.setOutlineThickness(1.f);
        shape.setPosition(scr);
        window.draw(shape);
    }
}

void Game::updateEnemyBullets(float dt) {
    for (auto& b : enemyBullets) {
        if (!b.alive) continue;
        b.worldPos += b.dir * b.speed * dt;
        b.lifetime -= dt;
        if (b.lifetime <= 0.f) { b.alive = false; continue; }

        if (vlen(b.worldPos - playerPos) < 28.f + 8.f) {
            if (invincTimer <= 0.f && boostGhostTimer <= 0.f) {
                 playerHp -= 10;
                invincTimer = 1.f;
                screenFlashTimer = 0.30f;
                screenFlashMax   = 0.30f;
                screenFlashCol   = sf::Color(220, 20, 20);
                sound.play("player_hit");
                if (playerHp <= 0) {
                    state = State::GameOver;
                    screenFlashTimer = 1.0f;
                    screenFlashMax   = 1.0f;
                    screenFlashCol   = sf::Color(200, 0, 0);
                    sound.play("game_over");
                    sound.stopLoop("move_loop");
                    sound.stopLoop("low_hp_alarm");
                    sound.playMusic("menu_ambient.wav", true);
                }
            }
            b.alive = false;
        }
    }
    enemyBullets.erase(
        std::remove_if(enemyBullets.begin(), enemyBullets.end(),
                       [](const EnemyBullet& b){ return !b.alive; }),
        enemyBullets.end()
    );
}

void Game::drawHUD() {
    float barW   = 512.f;
    float barH   = 24.f;
    float x      = 20.f;
    float y      = GH - 40.f;
    float filled = barW * (playerHp / (float)playerMaxHp);

    sf::RectangleShape bgBar({barW, barH});
    bgBar.setPosition(x, y);
    bgBar.setFillColor(sf::Color(60, 0, 0));
    window.draw(bgBar);

    sf::RectangleShape hpBar({filled, barH});
    hpBar.setPosition(x, y);
    float hpRatio = (float)playerHp / (float)playerMaxHp;
    sf::Color hpCol;
    if      (hpRatio > 0.6f) hpCol = sf::Color(0,   220, 80);
    else if (hpRatio > 0.3f) hpCol = sf::Color(255, 180,  0);
    else                     hpCol = sf::Color(220,  40, 40);
    hpBar.setFillColor(hpCol);
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

    t.setString("HP: " + std::to_string(playerHp) + " / " + std::to_string(playerMaxHp));
    t.setFillColor(sf::Color(255, 255, 255));
    t.setPosition(x+5.f, y +1.f );
    window.draw(t);

    t.setString("LVL " + std::to_string(playerLevel) + "   XP: " + std::to_string(playerXp) + " / " + std::to_string(xpToNext));
    t.setFillColor(sf::Color(180, 200, 255));
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
    centerText(t, GW/2.f, GH*0.28f);
    window.draw(t);

    // Statystyki
    int gm = (int)gameTime;
    std::string timeStr = std::to_string(gm/60) + ":" + (gm%60 < 10 ? "0" : "") + std::to_string(gm%60);

    t.setCharacterSize(24);
    t.setFillColor(sf::Color(200, 200, 200));

    t.setString("Czas:    " + timeStr);
    centerText(t, GW/2.f, GH*0.45f);
    window.draw(t);

    t.setString("Fala:    " + std::to_string(waveNumber));
    centerText(t, GW/2.f, GH*0.52f);
    window.draw(t);

    t.setString("Kills:   " + std::to_string(totalKills));
    centerText(t, GW/2.f, GH*0.59f);
    window.draw(t);

    t.setString("Poziom:  " + std::to_string(playerLevel));
    centerText(t, GW/2.f, GH*0.66f);
    window.draw(t);

    float blink = 0.5f + 0.5f * std::sin(globalTime * 3.f);
    t.setString("[ ENTER - wroc do menu ]");
    t.setCharacterSize(22);
    t.setFillColor(sf::Color(160, 160, 160, (sf::Uint8)(blink * 220.f)));
    centerText(t, GW/2.f, GH*0.80f);
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
    const float pickupRadius = 40.f;
    const float magnetRadius = playerMagnet;
    for (auto& orb : xpOrbs) {
        if (!orb.alive) continue;
        float d = vlen(orb.worldPos - playerPos);
        if (d < magnetRadius)
            orb.worldPos += vnorm(playerPos - orb.worldPos) * 400.f * 0.016f;
        if (d < pickupRadius) {
            orb.alive = false;
            sound.play("xp_pickup");
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
                    sound.play("level_up");
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
            case 0: r=10.f; fill=sf::Color(40,160,255);  out=sf::Color(100,200,255); break;
            case 1: r=16.f; fill=sf::Color(255,200,40);  out=sf::Color(255,230,100); break;
            case 2: r=22.f; fill=sf::Color(255,100,180); out=sf::Color(255,160,210); break;
            case 3: r=28.f; fill=sf::Color(60,220,80);   out=sf::Color(120,255,140); break;
            default:r=10.f; fill=sf::Color(40,160,255);  out=sf::Color(100,200,255); break;
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
    const float magnetRadius = playerMagnet;
    for (auto& o : hpOrbs) {
        if (!o.alive) continue;
        float d = vlen(o.worldPos - playerPos);
        if (d < magnetRadius)
            o.worldPos += vnorm(playerPos - o.worldPos) * 400.f * 0.016f;
        if (d < 40.f) {
            o.alive   = false;
            playerHp = std::min(playerHp + 25, playerMaxHp);
            sound.play("hp_pickup");
        }
    }
}

void Game::checkBoostPickup() {
    const float magnetRadius = playerMagnet;
    for (auto& o : boostOrbs) {
        if (!o.alive) continue;
        float d = vlen(o.worldPos - playerPos);
        if (d < magnetRadius)
            o.worldPos += vnorm(playerPos - o.worldPos) * 400.f * 0.016f;
        if (d < 40.f) {
            o.alive = false;
            sound.play("boost_pickup");
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
    if (boostSpeedTimer > 0.f) {
        boostSpeedTimer -= dt;
        playerSpeedMult = baseSpeedMult + 0.5f;
    } else {
        playerSpeedMult = baseSpeedMult;
    }

    if (boostFireTimer > 0.f) {
        boostFireTimer -= dt;
        playerFireRate = baseFireRate * 0.5f;
    } else {
        playerFireRate = baseFireRate;
    }

    if (boostMagnetTimer > 0.f) {
        boostMagnetTimer -= dt;
        playerMagnet = baseMagnet * 5.f;
    } else {
        playerMagnet = baseMagnet;
    }

    if (boostDmgTimer   > 0.f) boostDmgTimer   -= dt;
    if (boostGhostTimer > 0.f) boostGhostTimer -= dt;
}

void Game::drawHpOrbs() {
    sf::CircleShape shape(16.f);
    shape.setOrigin(16.f, 16.f);
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
    sf::CircleShape shape(18.f);
    shape.setOrigin(18.f, 18.f);
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
    sound.play("dash");
}

void Game::tickDash(float dt) {
    bool wasOnCd = dashCd > 0.f;
    if (dashCd > 0.f) dashCd -= dt;
    if (wasOnCd && dashCd <= 0.f) sound.play("dash_ready");
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
    enemyBullets.clear();
    xpOrbs.clear();
    hpOrbs.clear();
    boostOrbs.clear();
    fireTrails.clear();
    playerShape.setRotation(0.f);
    gameTime = 0.f;
    totalKills = 0;
    playerAtk       = 15.f;
    playerSpeedMult = 1.f;
    playerFireRate  = 1.4f;
    playerMaxHp     = 100;
    playerMagnet    = 225.f;
    playerCdr       = 0.f;
    playerHpRegen   = 0.f;
    baseSpeedMult = 1.f;
    baseFireRate  = 1.4f;
    baseMagnet    = 225.f;
    upgradePool     = getUpgradePool();
    pendingLevelUps = 0;
    upgradeChoices.clear();
    hasOrbitalNode = hasChainShock = hasFrostBurst = hasVirusBomb = false;
    hasStaticStorm = hasDataVortex = hasCoreDropWeapon = hasCorruptionZone = false;
    hasShockGrid   = hasPacketFlood = hasOverburn = hasCryoSweep = false;
    orbitalLevel = chainShockLevel = frostBurstLevel = virusBombLevel = 0;
    staticStormLevel = dataVortexLevel = coreDropLevel = corruptionLevel = 0;
    shockGridLevel = packetFloodLevel = overburnLevel = cryoSweepLevel = 0;
    empFlashLevel = laserRayLevel = 0;
    hasEmpFlash    = hasLaserRay = false;
    empFlashLevel = laserRayLevel = 0;
    weaponEffects.clear();
    corruptionZones.clear();
    bossArenaActive = false;
    bossArenaRadius = 0.f;
    bossArenaCenter = {0.f, 0.f};
    bulletColor = sf::Color(0, 240, 180);
    sound.stopLoop("move_loop");
    sound.stopLoop("low_hp_alarm");
    sound.stopLoop("corruption_zone");
}

void Game::spawnBoss() {
    float angle = frand() * 2.f * GPI;
    float dist  = 700.f;

    Enemy e;
    e.worldPos    = playerPos + sf::Vector2f(std::cos(angle)*dist, std::sin(angle)*dist);
    e.flashTimer  = 0.f;
    e.alive       = true;
    e.isBoss      = true;
    e.zigzagTimer = 0.f;
    e.type        = 0;

    int bossIndex = waveNumber / 5;
    e.hp    = 500.f + bossIndex * 250.f;
    e.maxHp = e.hp;
    e.speed = 50.f  + bossIndex * 4.f;

    enemies.push_back(e);

    // Aktywuj sferę więzienną bossa
    bossArenaActive = true;
    bossArenaCenter = playerPos;
    bossArenaRadius = 1400.f;
    sound.play("boss_spawn");
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

    for (int i = 0; i < 5 && !available.empty(); ++i) {
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
    sound.play("upgrade_select");

    // Reset wagi zignorowanych
    for (auto* u : upgradeChoices)
        if (u != chosen) u->weight = u->baseWeight;

    // ── Faktyczne efekty ─────────────────────────────────
    const std::string& id = chosen->id;

    if (id == "ghost_process") {
        baseSpeedMult += 0.08f;
        playerSpeedMult = baseSpeedMult;               // +8% prędkości
    } else if (id == "pulse_bolt") {
        playerAtk *= 1.10f;                     // +10% obrażeń
    } else if (id == "firewall_shield") {
        playerMaxHp += 15;
        playerHp = std::min(playerHp + 15, playerMaxHp);
    } else if (id == "encryption_ring") {
        playerHpRegen += 1.f;                   // +1 HP/s
    } else if (id == "data_bolt") {
        // Każdy poziom: dodatkowy pocisk w spread
        // playerExtraBullets używane w findAndShoot — patrz niżej
        playerAtk *= 1.08f;
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
        baseMagnet += 40.f;
        playerMagnet = baseMagnet;
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
        screenFlashTimer = 0.25f;
        screenFlashMax   = 0.25f;
        screenFlashCol   = sf::Color(0, 160, 255);
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
    hint.setString("[ 1 ]  [ 2 ]  [ 3 ]  [ 4 ]  [ 5 ]");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(160, 160, 160));
    centerText(hint, GW/2.f, GH*0.88f);
    window.draw(hint);

    // 5 kart
    float cardW = 220.f, cardH = 340.f;
    float totalW = 5 * cardW + 4 * 15.f;
    float startX = GW/2.f - totalW/2.f;

    for (int i = 0; i < (int)upgradeChoices.size(); ++i) {
        Upgrade* u = upgradeChoices[i];
        float cx = startX + i * (cardW + 15.f);
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
                totalKills++;
                spawnXpOrb(e.worldPos, e.isBoss ? 50 : (e.type == 2 ? 15 : 1));
                spawnParticles(e.worldPos, col, 6);
            }
        }
    }
    // Efekt wizualny
    WeaponEffect ef;
    ef.worldPos = pos;
    ef.radius   = radius * 0.3f;
    ef.alpha    = 220.f;
    ef.lifetime = 0.4f;
    ef.color    = col;
    ef.alive    = true;
    weaponEffects.push_back(ef);
}

void Game::tickWeapons(float dt) {

    // ── Orbital Node — krąży wokół gracza ────────────────
    if (hasOrbitalNode) {
        orbitalAngle += (1.5f + orbitalLevel * 0.2f) * dt;
        orbitalSfxCd -= dt;
        float orbitR  = 80.f + orbitalLevel * 10.f;
        int   numSats = orbitalLevel;
        bool hitThisFrame = false;
        for (int s = 0; s < numSats; s++) {
            float angle = orbitalAngle + s * (2.f * GPI / std::max(1, numSats));
            sf::Vector2f orbPos = playerPos + sf::Vector2f(
                std::cos(angle) * orbitR,
                std::sin(angle) * orbitR);
            for (auto& e : enemies) {
                if (!e.alive) continue;
                if (vlen(e.worldPos - orbPos) < 18.f) {
                    e.hp -= (playerAtk * 0.5f) * dt;
                    e.flashTimer = 0.1f;
                    hitThisFrame = true;
                    if (e.hp <= 0.f) {
                        e.alive = false;
                        spawnXpOrb(e.worldPos, 1);
                        spawnParticles(e.worldPos, bulletColor, 6);
                    }
                }
            }
        }
        if (hitThisFrame && orbitalSfxCd <= 0.f) {
            orbitalSfxCd = 0.3f;
            sound.play("orbital_hit");
        }
    }
    // ── Chain Shock — łańcuch co 2s ──────────────────────
    if (hasChainShock) {
        chainShockCd -= dt;
        if (chainShockCd <= 0.f) {
            chainShockCd = 2.0f - chainShockLevel * 0.15f;
            sound.play("chain_shock");
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
            sound.play("frost_burst");
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
                sound.play("virus_bomb");
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
                sound.play("static_strike");
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
            sound.play("data_vortex");
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

                sound.play("core_drop");

                hitEnemiesInRadius(
                enemies[idx].worldPos,
                60.f + coreDropLevel * 10.f,
                playerAtk * (2.f + coreDropLevel * 0.4f),
                sf::Color(255, 100, 0)
                );

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
        if (!corruptionZones.empty()) {
            sound.playLoop("corruption_zone");
        } else {
            sound.stopLoop("corruption_zone");
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
                b.color    = bulletColor;
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
            overburnCd = 0.12f;
            FireTrail ft;
            ft.worldPos = playerPos;
            ft.radius   = 35.f + overburnLevel * 8.f;
            ft.lifetime = 0.6f + overburnLevel * 0.05f;
            ft.maxLife  = ft.lifetime;
            ft.alive    = true;
            fireTrails.push_back(ft);
        }
        for (auto& ft : fireTrails) {
            if (!ft.alive) continue;
            for (auto& e : enemies) {
                if (!e.alive) continue;
                if (vlen(e.worldPos - ft.worldPos) < ft.radius) {
                    e.hp -= playerAtk * 0.5f * dt;
                    e.flashTimer = 0.05f;
                    if (e.hp <= 0.f) {
                        e.alive = false;
                        totalKills++;
                        spawnXpOrb(e.worldPos, e.isBoss ? 50 : (e.type == 2 ? 3 : 1));
                        spawnParticles(e.worldPos, sf::Color(255,80,0), 6);
                    }
                }
            }
        }
    }

    // ── Cryo Sweep — fala we wszystkich kierunkach co 5s ──
    if (hasCryoSweep) {
        cryoSweepCd -= dt;
        if (cryoSweepCd <= 0.f) {
            cryoSweepCd = 5.f - cryoSweepLevel * 0.3f;
            sound.play("cryo_sweep");
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
            sound.play("emp_flash");
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
                sound.play("laser_ray");
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
        float orbitR  = 80.f + orbitalLevel * 10.f;
        int   numSats = orbitalLevel;
        float orbSize = 10.f + orbitalLevel * 2.f;

        // Pierścień ścieżki orbity
        sf::CircleShape ring(orbitR); ring.setOrigin(orbitR, orbitR);
        ring.setPosition(GW/2.f, GH/2.f);
        ring.setFillColor(sf::Color::Transparent);
        sf::Color ringCol = bulletColor; ringCol.a = 18;
        ring.setOutlineColor(ringCol); ring.setOutlineThickness(1.f);
        window.draw(ring);

        // Kolory na podstawie bulletColor
        sf::Color orbFill = bulletColor; orbFill.a = 220;
        sf::Color orbOut(
            std::min(255, (int)bulletColor.r + 60),
            std::min(255, (int)bulletColor.g + 60),
            std::min(255, (int)bulletColor.b + 60));

        for (int s = 0; s < numSats; s++) {
            float angle = orbitalAngle + s * (2.f * GPI / std::max(1, numSats));
            sf::Vector2f orbPos = playerPos + sf::Vector2f(
                std::cos(angle) * orbitR,
                std::sin(angle) * orbitR);
            sf::Vector2f scr = worldToScreen(orbPos, playerPos);

            // Glow
            float gr = orbSize * 2.5f;
            sf::CircleShape glow(gr); glow.setOrigin(gr, gr);
            glow.setPosition(scr);
            sf::Color gc = bulletColor; gc.a = 45;
            glow.setFillColor(gc);
            window.draw(glow);

            // Satelita
            sf::CircleShape orb(orbSize); orb.setOrigin(orbSize, orbSize);
            orb.setPosition(scr);
            orb.setFillColor(orbFill);
            orb.setOutlineColor(orbOut);
            orb.setOutlineThickness(2.f);
            window.draw(orb);
        }
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

void Game::updateFireTrails(float dt) {
    for (auto& ft : fireTrails) {
        ft.lifetime -= dt;
        if (ft.lifetime <= 0.f) ft.alive = false;
    }
    fireTrails.erase(
        std::remove_if(fireTrails.begin(), fireTrails.end(),
            [](const FireTrail& f){ return !f.alive; }),
        fireTrails.end());
}

void Game::drawFireTrails() {
    for (auto& ft : fireTrails) {
        if (!ft.alive) continue;
        sf::Vector2f scr = worldToScreen(ft.worldPos, playerPos);
        float alpha = ft.lifetime / ft.maxLife;

        sf::CircleShape glow(ft.radius * 1.3f);
        glow.setOrigin(ft.radius * 1.3f, ft.radius * 1.3f);
        glow.setPosition(scr);
        glow.setFillColor(sf::Color(255, 80, 0, (sf::Uint8)(alpha * 60.f)));
        window.draw(glow);

        sf::CircleShape fire(ft.radius * alpha);
        fire.setOrigin(ft.radius * alpha, ft.radius * alpha);
        fire.setPosition(scr);
        fire.setFillColor(sf::Color(255, (sf::Uint8)(120 + 100 * alpha), 0, (sf::Uint8)(alpha * 180.f)));
        window.draw(fire);

        sf::CircleShape core(ft.radius * alpha * 0.5f);
        core.setOrigin(ft.radius * alpha * 0.5f, ft.radius * alpha * 0.5f);
        core.setPosition(scr);
        core.setFillColor(sf::Color(255, 255, 180, (sf::Uint8)(alpha * 200.f)));
        window.draw(core);
    }
}

void Game::drawMinimap() {
    const float mmW  = 150.f;
    const float mmH  = 100.f;
    const float mmX  = GW - mmW - 20.f;
    const float mmY  = GH - mmH - 20.f;
    const float scale = mmW / 1600.f; // świat → minimap

    // Tło
    sf::RectangleShape bg({mmW, mmH});
    bg.setPosition(mmX, mmY);
    bg.setFillColor(sf::Color(0, 0, 0, 160));
    bg.setOutlineColor(sf::Color(0, 200, 150, 120));
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    // Wrogowie
    for (auto& e : enemies) {
        if (!e.alive) continue;
        sf::Vector2f rel = e.worldPos - playerPos;
        float mx = mmX + mmW/2.f + rel.x * scale;
        float my = mmY + mmH/2.f + rel.y * scale;
        if (mx < mmX || mx > mmX+mmW || my < mmY || my > mmY+mmH) continue;
        sf::CircleShape dot(e.isBoss ? 4.f : 2.f);
        dot.setOrigin(dot.getRadius(), dot.getRadius());
        dot.setPosition(mx, my);
        dot.setFillColor(e.isBoss ? sf::Color(255,180,0) : sf::Color(220,60,60));
        window.draw(dot);
    }

    // Orby XP
    for (auto& o : xpOrbs) {
        if (!o.alive) continue;
        sf::Vector2f rel = o.worldPos - playerPos;
        float mx = mmX + mmW/2.f + rel.x * scale;
        float my = mmY + mmH/2.f + rel.y * scale;
        if (mx < mmX || mx > mmX+mmW || my < mmY || my > mmY+mmH) continue;
        sf::CircleShape dot(1.5f);
        dot.setOrigin(1.5f, 1.5f);
        dot.setPosition(mx, my);
        dot.setFillColor(sf::Color(40, 160, 255, 180));
        window.draw(dot);
    }

    // Gracz — zawsze w centrum
    sf::CircleShape player(3.f);
    player.setOrigin(3.f, 3.f);
    player.setPosition(mmX + mmW/2.f, mmY + mmH/2.f);
    player.setFillColor(sf::Color(0, 240, 180));
    window.draw(player);
}

void Game::applyClass(PlayerClass pc) {
    ClassDef d = getClassDef(pc);
    playerMaxHp     = (int)(100.f * d.hpMult);
    playerHp        = playerMaxHp;
    baseSpeedMult   = d.spdMult;
    playerSpeedMult = d.spdMult;
    playerAtk       = 10.f * d.atkMult;
    baseFireRate    = 1.4f * d.fireRateMult;
    playerFireRate  = baseFireRate;
    baseMagnet      = 150.f + d.magnetBonus;
    playerMagnet    = baseMagnet;
    playerHpRegen   = d.regenBonus;

    int pts = d.shapePoints;
    playerShape.setPointCount(pts);
    for (int i = 0; i < pts; ++i) {
        float angle = (float)i / pts * 2.f * GPI - GPI/2.f;
        float r = (pts == 4) ? 36.f : 32.f;
        playerShape.setPoint(i, {std::cos(angle)*r, std::sin(angle)*r});
    }
    playerShape.setFillColor(sf::Color(0, 20, 30));
    playerShape.setOutlineColor(d.color);
    playerShape.setOutlineThickness(3.f);

    // Kolor pocisków = kolor klasy
    bulletColor = d.color;
}

void Game::drawClassSelect() {
    if (font.getInfo().family.empty()) return;

    // Przyciemnione tło
    sf::RectangleShape overlay({GW, GH});
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(overlay);

    sf::Text title;
    title.setFont(font);
    title.setString("WYBIERZ KLASE");
    title.setCharacterSize(52);
    title.setFillColor(COL_CORE);
    centerText(title, GW/2.f, GH*0.12f);
    window.draw(title);

    sf::Text hint;
    hint.setFont(font);
    hint.setString("Kliknij aby wybrac");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(120, 120, 120));
    centerText(hint, GW/2.f, GH*0.20f);
    window.draw(hint);

    PlayerClass classes[] = {PlayerClass::Payload, PlayerClass::Firewall,
                              PlayerClass::Packet, PlayerClass::Daemon,
                              PlayerClass::Exploit};

    float cardW  = 200.f, cardH = 300.f;
    float startX = GW/2.f - 2.5f * cardW - 2.f * 10.f;

    for (int i = 0; i < 5; ++i) {
        ClassDef d = getClassDef(classes[i]);
        float cx = startX + i * (cardW + 10.f);
        float cy = GH/2.f - cardH/2.f;
        bool hovered = (hoveredClass == i);

        // Tło karty
        sf::RectangleShape card({cardW, cardH});
        card.setPosition(cx, cy);
        card.setFillColor(hovered ? sf::Color(20, 30, 50, 240) : sf::Color(10, 15, 30, 220));
        card.setOutlineColor(hovered ? sf::Color(d.color.r, d.color.g, d.color.b, 255)
                                     : sf::Color(d.color.r, d.color.g, d.color.b, 140));
        card.setOutlineThickness(hovered ? 3.f : 1.5f);
        window.draw(card);

        // Kształt gracza jako ikona
        sf::ConvexShape icon;
        int pts = d.shapePoints;
        icon.setPointCount(pts);
        for (int j = 0; j < pts; ++j) {
            float angle = (float)j / pts * 2.f * GPI - GPI/2.f;
            float r = (pts == 4) ? 28.f : 26.f;
            icon.setPoint(j, {std::cos(angle)*r, std::sin(angle)*r});
        }
        icon.setFillColor(sf::Color(0, 20, 30));
        icon.setOutlineColor(d.color);
        icon.setOutlineThickness(2.5f);
        icon.setPosition(cx + cardW/2.f, cy + 60.f);
        window.draw(icon);

        // Nazwa
        sf::Text name;
        name.setFont(font);
        name.setString(d.name);
        name.setCharacterSize(20);
        name.setFillColor(d.color);
        centerText(name, cx + cardW/2.f, cy + 110.f);
        window.draw(name);

        // Opis
        sf::Text desc;
        desc.setFont(font);
        desc.setString(d.desc);
        desc.setCharacterSize(14);
        desc.setFillColor(sf::Color(200, 200, 200));
        // Wyśrodkuj każdą linię
        float dy = cy + 150.f;
        std::string line;
        std::string full = d.desc;
        for (char c : full) {
            if (c == '\n') {
                desc.setString(line);
                centerText(desc, cx + cardW/2.f, dy);
                window.draw(desc);
                dy += 22.f;
                line.clear();
            } else line += c;
        }
        if (!line.empty()) {
            desc.setString(line);
            centerText(desc, cx + cardW/2.f, dy);
            window.draw(desc);
        }
    }
}

void Game::updateBossArena() {
    if (!bossArenaActive) return;

    bool bossAlive = false;
    for (auto& e : enemies) {
        if (e.alive && e.isBoss) {
            bossAlive = true;
            break;
        }
    }

    if (!bossAlive) {
        bossArenaActive = false;
        return;
    }

    float d = vlen(playerPos - bossArenaCenter);
    if (d > bossArenaRadius - 30.f) {
        playerPos = bossArenaCenter +
            vnorm(playerPos - bossArenaCenter) *
            (bossArenaRadius - 30.f);
    }
}

void Game::drawBossArena() {
    if (!bossArenaActive) return;
    sf::Vector2f scrCenter = worldToScreen(bossArenaCenter, playerPos);
    float pulse = bossArenaRadius + 8.f * std::sin(globalTime * 4.f);

    // Zewnętrzne kółko sfery
    sf::CircleShape ring(pulse);
    ring.setOrigin(pulse, pulse);
    ring.setPosition(scrCenter);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(255, 80, 0, 160));
    ring.setOutlineThickness(4.f);
    window.draw(ring);

    // Drugi pierścień (migający)
    float inner = pulse - 20.f;
    sf::CircleShape ring2(inner);
    ring2.setOrigin(inner, inner);
    ring2.setPosition(scrCenter);
    ring2.setFillColor(sf::Color::Transparent);
    float a2 = 60.f + 40.f * std::sin(globalTime * 6.f);
    ring2.setOutlineColor(sf::Color(255, 40, 0, (sf::Uint8)a2));
    ring2.setOutlineThickness(2.f);
    window.draw(ring2);

    // Tekst ostrzeżenia
    if (!font.getInfo().family.empty()) {
        float blinkA = 0.5f + 0.5f * std::sin(globalTime * 5.f);
        sf::Text warn;
        warn.setFont(font);
        warn.setString("! BOSS ARENA !");
        warn.setCharacterSize(22);
        warn.setFillColor(sf::Color(255, 80, 0, (sf::Uint8)(blinkA * 220.f)));
        centerText(warn, GW / 2.f, 80.f);
        window.draw(warn);
    }
}

void Game::drawPauseMenu() {
    sf::RectangleShape overlay({GW, GH});
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    if (font.getInfo().family.empty()) return;

    sf::Text title;
    title.setFont(font);
    title.setString("PAUZA");
    title.setCharacterSize(60);
    title.setFillColor(COL_CORE);
    centerText(title, GW / 2.f, GH * 0.30f);
    window.draw(title);

    const char* opts[] = { "Wznow gre", "Wroc do menu" };
    for (int i = 0; i < 2; ++i) {
        sf::Text opt;
        opt.setFont(font);
        opt.setString(opts[i]);
        opt.setCharacterSize(30);
        bool sel = (pauseSelected == i);
        opt.setFillColor(sel ? sf::Color(255, 200, 0) : sf::Color(180, 180, 180));
        if (sel) {
            // Ramka wokół zaznaczonej opcji
            auto b = opt.getLocalBounds();
            float ox = GW / 2.f - b.width / 2.f - 16.f;
            float oy = GH * 0.50f + i * 60.f - b.height / 2.f - 8.f;
            sf::RectangleShape box({b.width + 32.f, b.height + 20.f});
            box.setPosition(ox, oy);
            box.setFillColor(sf::Color(30, 30, 60, 180));
            box.setOutlineColor(sf::Color(255, 200, 0, 180));
            box.setOutlineThickness(2.f);
            window.draw(box);
        }
        centerText(opt, GW / 2.f, GH * 0.50f + i * 60.f);
        window.draw(opt);
    }

    sf::Text hint;
    hint.setFont(font);
    hint.setString("Strzalki + Enter  |  ESC = wznow");
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(100, 100, 100));
    centerText(hint, GW / 2.f, GH * 0.78f);
    window.draw(hint);
}

void Game::recycleStars() {

    float halfW = GW * 1.2f;
    float halfH = GH * 1.2f;
    for (auto& s : stars) {
        sf::Vector2f rel = s.worldPos - playerPos;
        if (std::abs(rel.x) > halfW || std::abs(rel.y) > halfH) {
            float angle = frand() * 2.f * GPI;
            float dist  = frandr(GW * 0.5f, GW * 1.1f);
            s.worldPos  = playerPos + sf::Vector2f(std::cos(angle)*dist, std::sin(angle)*dist);
            s.r         = frandr(0.5f, 2.5f);
            s.bright    = frandr(0.4f, 1.f);
            s.twSpd     = frandr(1.f, 3.f);
            s.twPhase   = frand() * GPI * 2.f;
        }
    }
}

void Game::initMatrixDrops() {
    const std::string chars = "01アイウエABCDEF#&@%$!><";
    matrixDrops.resize(55);
    for (auto& d : matrixDrops) {
        d.x     = frandr(0.f, GW);
        d.y     = frandr(-GH, GH);
        d.speed = frandr(55.f, 175.f);
        d.alpha = frandr(0.12f, 0.6f);
        d.ch    = chars[rand() % chars.size()];
    }
}

void Game::updateMatrixDrops(float dt) {
    const std::string chars = "01アイウエABCDEF#&@%$!><";
    matrixTimer += dt;
    for (auto& d : matrixDrops) {
        d.y += d.speed * dt;
        if (matrixTimer > 0.12f)
            d.ch = chars[rand() % chars.size()];
        if (d.y > GH + 20.f) {
            d.y     = -20.f;
            d.x     = frandr(0.f, GW);
            d.speed = frandr(55.f, 175.f);
            d.alpha = frandr(0.12f, 0.6f);
        }
    }
    if (matrixTimer > 0.12f) matrixTimer = 0.f;
}
