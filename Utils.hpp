#pragma once
#include <SFML/Graphics.hpp>
#include <string>

float        frand();
float        frandr(float a, float b);
float        vlen(sf::Vector2f v);
sf::Vector2f vnorm(sf::Vector2f v);
sf::Color    hueToColor(float hue, sf::Uint8 alpha = 255);
sf::Vector2f worldToScreen(sf::Vector2f worldPos, sf::Vector2f playerPos);
bool         isOnScreen(sf::Vector2f scr, float margin = 80.f);
void         centerText(sf::Text& t, float x, float y);
