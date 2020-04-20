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
}

// TODO Refactor into multiple AI with compatibility constraints
void MoverDirectionChoiceAI_Act(Grid& grid, Mover& mover)
{
    Stage stage = grid.tiles[mover.pos.y][mover.pos.x];
    if (stage.group == SNAKE_HEAD)
    {
        
    }
}

void EvaluateMoves_Act(Grid& grid, MoveResolver& resolver, Mover& mover)
{
    if (!resolver.moveThisFrame) return;
    // Movers don't always actually move, but they always try to move
    sf::Vector2i moveToPos = mover.pos + Movement::GetOffsetVector(mover.nextMove);
    if (grid.shouldGroupWrap.count(grid.tiles[mover.pos.y][mover.pos.x].group))
    {
        if (moveToPos.x == grid.cols) 
        {
            moveToPos.x = 0;
        } else if (moveToPos.x == -1)
        {
            moveToPos.x = grid.cols - 1;
        }
        if (moveToPos.y == grid.rows) 
        {
            moveToPos.y = 0;
        } else if (moveToPos.y == -1)
        {
            moveToPos.y = grid.rows - 1;
        }
    }
    resolver.requestedMoves.push_back({mover.pos, moveToPos, mover.id});
};

void ResolveMoves_Act(MoveResolver& resolver, Grid& grid)
{
    if (!resolver.moveThisFrame) return;
    // TODO Sort by priority
    for (MoveResolver::MoveRequest& request : resolver.requestedMoves)
    {
        Stage current = grid.tiles[request.from.y][request.from.x];
        grid.tiles[request.from.y][request.from.x] = {EMPTY, 0};
        grid.tiles[request.to.y][request.to.x] = current;
        // TODO If the move was successful!
        BicycleMango::GetProps<Mover>()[request.moverId].pos = request.to;
    }
    resolver.requestedMoves.clear();
}

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
