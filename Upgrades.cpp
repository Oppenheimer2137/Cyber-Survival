#include "Upgrades.hpp"

std::vector<Upgrade> getUpgradePool() {
    std::vector<Upgrade> pool;

    // ── PASYWNE STATYSTYKI ────────────────────────────────────

    pool.push_back({
        "ghost_process", "Ghost Process",
        "+8% predkosci ruchu per poziom",
        Rarity::Common, 0, 5, 10.f, 10.f,
        sf::Color(60, 220, 180)
    });
    pool.push_back({
        "pulse_bolt", "Pulse Bolt",
        "+10% obrazen pociskow per poziom",
        Rarity::Common, 0, 5, 10.f, 10.f,
        sf::Color(40, 160, 255)
    });
    pool.push_back({
        "firewall_shield", "Firewall Shield",
        "+15 max HP per poziom",
        Rarity::Common, 0, 5, 10.f, 10.f,
        sf::Color(0, 200, 255)
    });
    pool.push_back({
        "encryption_ring", "Encryption Ring",
        "+1 HP regeneracji na sekunde",
        Rarity::Rare, 0, 3, 7.f, 7.f,
        sf::Color(180, 80, 255)
    });
    pool.push_back({
        "data_bolt", "Data Bolt",
        "+1 dodatkowy pocisk jednoczesnie",
        Rarity::Rare, 0, 4, 7.f, 7.f,
        sf::Color(100, 200, 255)
    });

    // ── AKTYWNE BRONIE ────────────────────────────────────────

    pool.push_back({
        "chain_shock", "Chain Shock",
        "Lancuch elektryczny miedzy wrogami",
        Rarity::Rare, 0, 7, 7.f, 7.f,
        sf::Color(255, 220, 0)
    });
    pool.push_back({
        "frost_burst", "Frost Burst",
        "Fala mroz wokol gracza, zamraza wrogow",
        Rarity::Rare, 0, 7, 7.f, 7.f,
        sf::Color(100, 220, 255)
    });
    pool.push_back({
        "virus_bomb", "Virus Bomb",
        "Eksplozja AoE w miejscu najblizszego wroga",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(255, 60, 120)
    });
    pool.push_back({
        "orbital_node", "Orbital Node",
        "Satelita krazacy wokol gracza zadajacy DMG",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(255, 180, 0)
    });
    pool.push_back({
        "static_storm", "Static Storm",
        "Losowe pioruny wokol gracza",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(200, 100, 255)
    });
    pool.push_back({
        "data_vortex", "Data Vortex",
        "Przyciaga wrogow do srodka i zadaje DMG",
        Rarity::Legendary, 0, 5, 3.f, 3.f,
        sf::Color(255, 220, 0)
    });
    pool.push_back({
        "core_drop", "Core Drop",
        "Meteoryt spada na losowego wroga",
        Rarity::Legendary, 0, 5, 3.f, 3.f,
        sf::Color(255, 100, 0)
    });
    pool.push_back({
        "corruption_zone", "Corruption Zone",
        "Strefa obrazen pozostaje na podlodze",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(120, 0, 220)
    });
    pool.push_back({
        "shock_grid", "Shock Grid",
        "Elektryczna strefa wokol gracza",
        Rarity::Rare, 0, 7, 7.f, 7.f,
        sf::Color(0, 220, 255)
    });
    pool.push_back({
        "packet_flood", "Packet Flood",
        "Fala pociskow przed graczem",
        Rarity::Rare, 0, 7, 7.f, 7.f,
        sf::Color(0, 180, 255)
    });
    pool.push_back({
        "overburn", "Overburn",
        "Zostawia ogien za soba podczas ruchu",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(255, 80, 0)
    });
    pool.push_back({
        "cryo_sweep", "Cryo Sweep",
        "Fala mroz we wszystkich kierunkach",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(150, 230, 255)
    });
    pool.push_back({
        "emp_flash", "EMP Flash",
        "Oglusza wszystkich wrogow wokol gracza",
        Rarity::Rare, 0, 7, 7.f, 7.f,
        sf::Color(255, 255, 100)
    });
    pool.push_back({
        "laser_ray", "Laser Ray",
        "Laser przebijajacy wrogow przed graczem",
        Rarity::Epic, 0, 7, 5.f, 5.f,
        sf::Color(255, 40, 40)
    });

    return pool;
}
