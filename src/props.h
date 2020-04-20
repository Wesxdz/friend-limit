 #pragma once

#include "sun_lambda.h"
#include <SFML/Graphics.hpp>
#include "resources.h"
#include "bicycle_mango.h"

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
    std::set<Group> shouldGroupWrap = {PLAYER, PEASANT, HERO};
    
    sf::Sound placeApple;
    
    static inline bool PosInGrid( sf::Vector2i pos){return pos.x >= 0 && pos.x < cols && pos.y >= 0 && pos.y < rows; }
};

struct MoveResolver : Prop
{
    float timeBetweenMoves = 0.25f;
    sf::Clock moveTimer;
    bool moveThisFrame = false;
    sf::Sound beatSFX;
    int beatsCount;
    // If two Movers try to move to the same pos, we need to resolve this appropriately depending on the groups
    struct MoveRequest
    {
        sf::Vector2i from;
        sf::Vector2i to;
        BicycleMango::PropIdRaw moverId;
    };
    std::vector<MoveRequest> requestedMoves;
    // We need to resolve moves by having the higher priority moves happen first in case two things try to move onto the same tile
    // For example, SNAKE_HEAD should move onto a tile after KNIGHT or PLAYER
    std::map<Group, int> movePriority = {{SNAKE_HEAD, 50}, {SNAKE_BODY, 40}, {SNAKE_TAIL, 30}, {PEASANT, 60}, {KNIGHT, 70}, {HERO, 80}, {PLAYER, 100}};
};

#include "Direction.h"
struct Mover : Prop
{
    sf::Vector2i pos;
    Direction prevMove;
    Direction nextMove;
};

struct SnakeAI : Prop
{
    static inline std::unordered_map<Group, int> magnets = {{PEASANT, 100}, {KNIGHT, 70}, {HERO, 80}, {APPLE, 150}, {PLAYER, 100}};
};
