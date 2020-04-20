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
            default:
                return {-1, 0};
        }
    }
    static Direction Opposite(Direction direction)
    {
        switch (direction)
        {
            case NORTH:
                return SOUTH;
            case EAST:
                return WEST;
            case SOUTH:
                return NORTH;
            case WEST:
            default:
                return EAST;
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
