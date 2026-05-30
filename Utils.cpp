
#include "Utils.hpp"
#include "Constants.hpp"
#include <cstdlib>
#include <cmath>

float frand() { return (float)rand() / (float)RAND_MAX; }
float frandr(float a, float b) { return a + frand() * (b - a); }
float vlen(sf::Vector2f v) { return std::sqrt(v.x*v.x + v.y*v.y); }

sf::Vector2f vnorm(sf::Vector2f v) {
    float l = vlen(v);
    return l < 0.0001f ? sf::Vector2f(0,0) : sf::Vector2f(v.x/l, v.y/l);
}

sf::Color hueToColor(float hue, sf::Uint8 alpha) {
    float h = hue / 60.f; int i = (int)h; float f = h - i;
    switch (i % 6) {
        case 0: return sf::Color(255, (sf::Uint8)(f*255), 0, alpha);
        case 1: return sf::Color((sf::Uint8)((1-f)*255), 255, 0, alpha);
        case 2: return sf::Color(0, 255, (sf::Uint8)(f*255), alpha);
        case 3: return sf::Color(0, (sf::Uint8)((1-f)*255), 255, alpha);
        case 4: return sf::Color((sf::Uint8)(f*255), 0, 255, alpha);
        default:return sf::Color(255, 0, (sf::Uint8)((1-f)*255), alpha);
    }
}

sf::Vector2f worldToScreen(sf::Vector2f w, sf::Vector2f p) {
    return w - p + sf::Vector2f(GW/2.f, GH/2.f);
}
bool isOnScreen(sf::Vector2f s, float m) {
    return s.x > -m && s.x < GW+m && s.y > -m && s.y < GH+m;
}
void centerText(sf::Text& t, float x, float y) {
    auto b = t.getLocalBounds();
    t.setPosition(x - b.width/2.f, y - b.height/2.f - b.top);
}
