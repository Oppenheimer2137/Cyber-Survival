#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ── 5 klas postaci ────────────────────────────────────────────
enum class PlayerClass { None, Payload, Firewall, Packet, Daemon, Exploit };

struct ClassDef {
    std::string  name;
    std::string  desc;
    sf::Color    color;
    int          shapePoints;   // 3=trojkat, 4=diament, 5=pentagon, 6=szesciobok
    float        hpMult;        // mnożnik max HP
    float        spdMult;       // mnożnik prędkości
    float        atkMult;       // mnożnik obrażeń
    float        fireRateMult;  // mnożnik czasu między strzałami (niższy = szybciej)
    float        magnetBonus;   // bonus do zasięgu magnesu
    float        regenBonus;    // bonus HP/s
};

inline ClassDef getClassDef(PlayerClass pc) {
    switch (pc) {
    case PlayerClass::Payload:
        // Ofensywna - dużo obrażeń, mało HP - kolor czerwony - trojkat
        return {"PAYLOAD",  "Destrukcja protokolow.\n+40% DMG, +20% SPD\n-20% HP",
                sf::Color(255, 60, 60),  3,
                0.80f, 1.20f, 1.40f, 1.0f, 0.f, 0.f};
    case PlayerClass::Firewall:
        // Defensywna - dużo HP, regeneracja - kolor niebieski - szesciobok
        return {"FIREWALL", "Straznik sieci.\n+80% HP, Regen 2/s\n-15% DMG",
                sf::Color(0, 180, 255),  6,
                1.80f, 0.90f, 0.85f, 1.0f, 0.f, 2.f};
    case PlayerClass::Packet:
        // Utility - duży magnes, szybki ogień - kolor zielony - diament
        return {"PACKET",   "Kolekcjoner danych.\n+200 magnes, x2 fire rate\n-10% DMG",
                sf::Color(60, 220, 120), 4,
                1.00f, 1.10f, 0.90f, 0.50f, 200.f, 0.f};
    case PlayerClass::Daemon:
        // Balans - bonusy do wszystkiego po trochu - kolor fioletowy - pentagon
        return {"DAEMON",   "Proces tla.\n+20% DMG, +10% SPD\n+1 HP/s regen",
                sf::Color(180, 80, 255), 5,
                1.10f, 1.10f, 1.20f, 0.9f, 0.f, 1.f};
    case PlayerClass::Exploit:
        // Szybka - ultra prędkość, słaba - kolor żółty - trojkat mały
        return {"EXPLOIT",  "Zero-day w systemie.\n+60% SPD, szybki dash\n-30% HP, -20% DMG",
                sf::Color(255, 200, 0),  3,
                0.70f, 1.60f, 0.80f, 0.8f, 0.f, 0.f};
    default:
        return {"???", "", sf::Color::White, 5, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f};
    }
}
