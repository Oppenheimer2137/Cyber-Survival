#pragma once
#include <string>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

// ── Rzadkość ─────────────────────────────────────────────────
enum class Rarity { Common, Rare, Epic, Legendary };

// ── Definicja ulepszenia ─────────────────────────────────────
struct Upgrade {
    std::string   id;          // unikalny identyfikator np. "pulse_bolt"
    std::string   name;        // wyświetlana nazwa
    std::string   desc;        // opis efektu
    Rarity        rarity;
    int           level    = 0;   // aktualny poziom (0 = nie odblokowane)
    int           maxLevel = 7;   // poziom maksymalny
    float         weight   = 10.f; // waga losowania
    float         baseWeight = 10.f; // waga bazowa (do resetu)
    sf::Color     color;
};

// ── Pula wszystkich ulepszeń ─────────────────────────────────
// Zdefiniowana w Upgrades.cpp, dostępna przez getUpgradePool()
std::vector<Upgrade> getUpgradePool();
