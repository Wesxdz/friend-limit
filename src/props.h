 #pragma once

#include "sun_lambda.h"
#include <SFML/Graphics.hpp>
#include "resources.h"

struct WindowGameState : Prop
{
    static inline sf::RenderWindow window;
    std::map<sf::Event::EventType, std::vector<sf::Event>> events;
    bool focus = false;
};

struct Sprite : Prop
{
    sf::Sprite sprite;
};

struct Frames : Prop
{
    std::vector<sf::IntRect> frames;
};

struct FrameAnimator : Prop
{
    float progress;
    std::vector<sf::Time> frameTimes;
};

struct Camera : Prop
{
    sf::View view;
};

struct StatRenderInfo : Prop
{
    sf::Font statFont;
    sf::Text blood;
    sf::Text swords;
    sf::Vector2i swordsAnchorPos;
};

struct ConflictStats : Prop
{
    int blood;
    int swords;
};

struct Grid : Prop
{
    static const int rows = 14;
    static const int cols = 16;
    static const int tile_width = 8;
    static const int tile_height = 10;
    static constexpr int offset_y = tile_height * 2;
    Stage tiles[rows][cols];
};

struct MoveResolver : Prop
{
    float timeBetweenMoves = 0.25f;
    sf::Clock moveTimer;
    bool moveThisFrame = false;
    // If two Movers try to move to the same pos, we need to resolve this appropriately depending on the groups
    std::unordered_map<sf::Vector2i, std::vector<sf::Vector2i>> requestedMoves; // to->froms
};

#include "Direction.h"
struct Mover : Prop
{
    sf::Vector2i pos;
    Direction prevMove;
    Direction nextMove;
};
