#include "Suns.h"

#include "props.h"
#include "bicycle_mango.h"
#include <string>
#include "Direction.h"

void PlayerPlaceTiles_Act(WindowGameState& game, Grid& grid, Mover& mover)
{
    for (auto& event : game.events[sf::Event::EventType::KeyPressed])
    {
        if (event.key.code == sf::Keyboard::Key::Space)
        {
            sf::Vector2i placePos = mover.pos;
            placePos.y++;
            if (placePos.y == grid.rows) placePos.y = 0;
            if (grid.tiles[placePos.y][placePos.x].group == EMPTY)
            {
                if (grid.placeApple.getPlayingOffset().asSeconds() == 0.0f)
                {
                    grid.placeApple.play();
                }
                grid.tiles[placePos.y][placePos.x] = BicycleMango::Next(APPLE);
            }
        }
    }
}

void PlayerChangeMovementDirection_Act(WindowGameState& game, Mover& mover)
{
    for (auto& event : game.events[sf::Event::EventType::KeyPressed])
    {
        switch (event.key.code)
        {
            case sf::Keyboard::Key::W:
            case sf::Keyboard::Key::Up:
                mover.nextMove = Direction::NORTH;
                break;
            case sf::Keyboard::Key::D:
            case sf::Keyboard::Key::Right:
                mover.nextMove = Direction::EAST;
                break;
            case sf::Keyboard::Key::S:
            case sf::Keyboard::Key::Down:
                    mover.nextMove = Direction::SOUTH;
                    break;
            case sf::Keyboard::Key::A:
            case sf::Keyboard::Key::Left:
                mover.nextMove = Direction::WEST;
                break;
            default:
                break;
        }
    }
}

void SetupGrid_Act(Grid& grid)
{
    for (int row = 0; row < grid.rows; row++)
    {
        for (int col = 0; col < grid.cols; col++)
        {
            grid.tiles[row][col] = {EMPTY, 0};
        }
    }
    grid.tiles[1][1] = {PLAYER, 0};
    grid.tiles[grid.rows - 2][grid.cols - 2] = {SNAKE_HEAD, 0};
    grid.placeApple.setBuffer(*Resources::inst->LoadSoundBuffer("place-apple.wav"));
}

void TickMoveEvent_Act(MoveResolver& resolver)
{
    resolver.moveThisFrame = false;
    if (resolver.moveTimer.getElapsedTime().asSeconds() >= resolver.timeBetweenMoves)
    {
        resolver.beatsCount++;
        if (resolver.beatsCount % 4 == 0)
        {
            resolver.beatSFX.play();
            resolver.beatSFX.setVolume(10.0f);
            resolver.beatSFX.setPitch(0.7f);
        }
        resolver.moveThisFrame = true;
        resolver.moveTimer.restart();
    }
};

void EvaluateMoves_Act(Grid& grid, MoveResolver& resolver, Mover& mover)
{
    if (!resolver.moveThisFrame) return;
    // TODO: All these fucking moves MAY need to be evaluated simultaneously rather than sequentially
    Stage current = grid.tiles[mover.pos.y][mover.pos.x];
    // Only evaluate the move IF the mover can move there
    // The SNAKE can always move
    // For example, two KNIGHT group movers CANNOT move onto the same tile
    grid.tiles[mover.pos.y][mover.pos.x] = {EMPTY, 0};
    auto moveVec = Movement::GetOffsetVector(mover.nextMove);
    // TODO Resolve if moving onto non empty tile, otherwise pick a new movement or do nothing
    mover.pos += {moveVec.x, moveVec.y};
    // Player wrap around
    if (mover.pos.x == grid.cols) 
    {
        mover.pos.x = 0;
    } else if (mover.pos.x == -1)
    {
        mover.pos.x = grid.cols - 1;
    }
    if (mover.pos.y == grid.rows) 
    {
        mover.pos.y = 0;
    } else if (mover.pos.y == -1)
    {
        mover.pos.y = grid.rows - 1;
    }
    grid.tiles[mover.pos.y][mover.pos.x] = current;
};

void RenderGrid_Act(WindowGameState& game, Grid& grid)
{
    sf::Sprite tiles;
    tiles.setTexture(*Resources::inst->LoadTexture("tiles.png"));
    sf::Vector2i playerPos; // Keep track to render cursor
    sf::Sprite player;
    player.setTexture(*Resources::inst->LoadTexture("player.png"));
    player.setOrigin(2, 6);
    for (int row = 0; row < grid.rows; row++)
    {
        for (int col = 0; col < grid.cols; col++)
        {
            Group g = grid.tiles[row][col].group;
            switch (g)
            {
                case PLAYER:
                    player.setPosition(col * grid.tile_width, grid.offset_y + row * grid.tile_height);
                    playerPos = {col, row};
                    game.window.draw(player);
                    break;
                case EMPTY:
                    break;
                default:
                    tiles.setPosition(col * grid.tile_width - 1, grid.offset_y + row * grid.tile_height - 1);
                    tiles.setTextureRect({(g - SNAKE_HEAD) * (grid.tile_width + 2), 0, grid.tile_width + 2, grid.tile_height + 2});
                    game.window.draw(tiles);
            }
        }
    }
    sf::Sprite cursor;
    cursor.setTexture(*Resources::inst->LoadTexture("cursor.png"));
    cursor.setOrigin(1, 1);
    sf::Vector2i cursorPos = playerPos;
    cursorPos.y++;
    if (cursorPos.y == grid.rows) cursorPos.y = 0;
    cursor.setPosition(cursorPos.x * grid.tile_width, grid.offset_y + cursorPos.y * grid.tile_height);
    game.window.draw(cursor);
}

void DisplayUpdateStats_Act(WindowGameState& game, StatRenderInfo& statInfo, ConflictStats& stats)
{
    statInfo.blood.setString(std::to_string(stats.blood));
    game.window.draw(statInfo.blood);
    statInfo.swords.setString(std::to_string(stats.swords));
    statInfo.swords.setPosition(statInfo.swordsAnchorPos.x - statInfo.swords.getLocalBounds().width, statInfo.swordsAnchorPos.y);
    game.window.draw(statInfo.swords);
}

void SpriteRenderer_Act(WindowGameState& game, Sprite& sprite)
{
    game.window.draw(sprite.sprite);
}

void PollWindowEvents_Act(WindowGameState& game)
{
    game.events.clear(); // Events are only considered for a single loop

    sf::Event event;
    while (game.window.pollEvent(event))
    {
        if (event.type == sf::Event::GainedFocus)
        {
            game.focus = true;
        } 
        if (game.focus) // Push back LostFocus event
        {
            game.events[event.type].push_back(event);
        }
        if (event.type == sf::Event::LostFocus)
        {
            game.focus = false;
            break;
        }
    }
    if (game.events[sf::Event::Closed].size() || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
    {
        game.window.close();
        BicycleMango::brake = true;
    }

    game.window.clear();
}

void DisplayWindow_Act(WindowGameState& game)
{
    game.window.display();
}

#ifdef HOT_RELOAD
void HotReloadWatcher_Act(WindowGameState& game)
{
    for (auto& event : game.events[sf::Event::EventType::KeyPressed])
    {
        if (event.key.code == sf::Keyboard::R)
        {
            BicycleMango::ShouldReloadLambdas = true;
        }
    }
}
#endif
