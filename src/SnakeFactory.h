#pragma once

#include "props.h"

class SnakeFactory
{
public:
    static int GetSnakeLength(sf::Vector2i snakeHead, Grid& grid)
    {
        return 1;
    }
};
