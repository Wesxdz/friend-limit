#pragma once

enum Direction
{
    NORTH,
    EAST,
    SOUTH,
    WEST,
};

#include <SFML/Graphics.hpp>

class Movement
{
public:
    static sf::Vector2i GetOffsetVector(Direction direction)
    {
        switch (direction)
        {
            case NORTH:
                return {0, -1};
            case EAST:
                return {1, 0};
            case SOUTH:
                return {0, 1};
            case WEST:
                return {-1, 0};
        }
    }
    static Direction GetRandomDirection()
    {
        return (Direction)(rand() % 4);
    }
    static Direction GetRandomDirection(std::vector<Direction>& potential)
    {
        return potential[rand() % potential.size()];
    }
};
