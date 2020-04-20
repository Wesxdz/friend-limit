#pragma once
enum GROUP : Group
{
    NONE, // TODO: When a compatible constraint has no stages to iterate through, call with this
    
    EMPTY,
    SNAKE_HEAD,
    SNAKE_BODY,
    SNAKE_TAIL,
    PEASANT,
    KNIGHT,
    HERO,
    APPLE,
    TREASURE,
    
    SNAKE,
    GAME_STATE,
    TITLE_SCREEN,
    
    PLAYER,
    
    UI,
};
