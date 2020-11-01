#include "Suns.h"

#include "props.h"
#include "bicycle_mango.h"
#include <string>
#include "Direction.h"
#include "FriendLimit.h"

void EnterGame_Act(WindowGameState& game, MenuManager& menu)
{
    if (game.events.count(sf::Event::EventType::KeyPressed))
    {
        if (menu.status == MenuManager::TITLE || menu.status == MenuManager::GAME_OVER)
        {
            menu.status = MenuManager::PLAYING;
            // background.sprite.setTexture(*Resources::inst->LoadTexture("play-area.png"));
            FriendLimit::SetupGameplay();
        }
    }
}

void RenderCursor_Act(WindowGameState& game, Mover& mover)
{
    sf::Sprite cursor;
    cursor.setTexture(*Resources::inst->LoadTexture("cursor.png"));
    cursor.setOrigin(1, 1);
    sf::Vector2i cursorPos = mover.pos;
    cursorPos -= Movement::GetOffsetVector(mover.prevMove);
//     cursorPos.y--;
    Grid::Wrap(cursorPos);
    cursor.setPosition(cursorPos.x * Grid::tile_width, Grid::offset_y + cursorPos.y * Grid::tile_height);
    game.window.draw(cursor);
}

void SetPrevPos_Act(Mover& mover)
{
    mover.prevMove = mover.nextMove;
}

void PlayerPlaceTiles_Act(WindowGameState& game, Grid& grid, Mover& mover, AudioManager& audio)
{
    for (auto& event : game.events[sf::Event::EventType::KeyPressed])
    {
        if (event.key.code == sf::Keyboard::Key::Space)
        {
            sf::Vector2i placePos = mover.pos;
            placePos -= Movement::GetOffsetVector(mover.prevMove);
            Grid::Wrap(placePos);
            if (grid.tiles[placePos.y][placePos.x].group == EMPTY)
            {
                if (audio.placeApple.getPlayingOffset().asSeconds() == 0.0f)
                {
                    audio.placeApple.play();
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
}

void TickMoveEvent_Act(MoveResolver& resolver)
{
    resolver.moveThisFrame = false;
    if (resolver.moveTimer.getElapsedTime().asSeconds() >= resolver.timeBetweenMoves)
    {
        if (resolver.beatsCount % 8 == 0)
        {
//             resolver.beatSFX.play();
        }
        resolver.beatsCount++;
        resolver.moveThisFrame = true;
        resolver.moveTimer.restart();
    }
}

struct MoveOption
{
    sf::Vector2i pos;
    Direction direction;
    Stage occupying;
    int prio{0};
};

// TODO Refactor into multiple AI with compatibility constraints
void MoverDirectionChoiceAI_Act(Grid& grid, Mover& mover, MoveResolver& resolver)
{
    if (!resolver.moveThisFrame) return;
    Stage stage = grid.tiles[mover.pos.y][mover.pos.x];
    std::vector<MoveOption> moveOptions;
    for (int i = 0; i < 4; i++)
    {
        Direction dir = static_cast<Direction>(i);
        sf::Vector2i potential = mover.pos + Movement::GetOffsetVector(dir);
        if (Grid::PosInGrid(potential) && dir != Movement::Opposite(mover.prevMove))
        {
//             std::cout << "Move option: " << dir << std::endl;
            moveOptions.push_back({potential, dir, grid.tiles[potential.y][potential.x]});
        }
    }
    auto Priority = [](std::vector<MoveOption> prioritySelect) -> MoveOption
    {
        std::sort(prioritySelect.begin(), prioritySelect.end(), [](MoveOption& a, MoveOption& b)
        {
            if (SnakeAI::magnets.count(a.occupying.group)) a.prio = SnakeAI::magnets[a.occupying.group];
            if (SnakeAI::magnets.count(b.occupying.group)) b.prio = SnakeAI::magnets[b.occupying.group];
            return a.prio > b.prio;
        });
        // Choose random when there are multiple best move options that tie for the highest priority
        std::vector<MoveOption> bestChoices;
        bestChoices.push_back(prioritySelect[0]);
        for (size_t i = 1; i < prioritySelect.size(); i++)
        {
            if (prioritySelect[i].prio == bestChoices[0].prio)
            {
                bestChoices.push_back(prioritySelect[i]);
            }
        }
        return bestChoices[rand() % bestChoices.size()];
    };
    if (stage.group == SNAKE_HEAD)
    {
        MoveOption bestChoice = Priority(moveOptions);
        mover.nextMove = bestChoice.direction;
    }
}

void EvaluateMoves_Act(Grid& grid, MoveResolver& resolver, Mover& mover)
{
    if (!resolver.moveThisFrame) return;
    // Movers don't always actually move, but they always try to move
    sf::Vector2i moveToPos = mover.pos + Movement::GetOffsetVector(mover.nextMove);
    if (grid.shouldGroupWrap.count(grid.tiles[mover.pos.y][mover.pos.x].group))
    {
        Grid::Wrap(moveToPos);
    }
    resolver.requestedMoves.push_back({mover.pos, moveToPos, BicycleMango::GetPropId<Mover>(&mover)});
};

void ResolveMoves_Act(MoveResolver& resolver, Grid& grid, SnakeAI& ai, ConflictStats& stats, AudioManager& audio, MenuManager& menu)
{
    if (!resolver.moveThisFrame) return;
    // TODO Sort by priority
    auto movers = BicycleMango::GetProps<Mover>();
    for (MoveResolver::MoveRequest& request : resolver.requestedMoves)
    {
        bool moveSuccessful = true;
        Stage current = grid.tiles[request.from.y][request.from.x];
        Stage hit = grid.tiles[request.to.y][request.to.x];
        if (hit.group == APPLE)
        {
            audio.eat.play();
        }
        // TODO Respond to all stages interaction
        if (current.group == SNAKE_HEAD)
        {
            if (hit.group == SNAKE_BODY || hit.group == SNAKE_TAIL)
            {
//                 std::cout << "Snake bit himself!" << std::endl;
                // Snake bites of his tail and reduced blood x2 for each 'follow part' removed
                auto followPartBitten = std::find(ai.snakeParts.begin(), ai.snakeParts.end(), request.to);
                if (ai.snakeParts.size() > 2)
                {
                    auto newTail = followPartBitten - 1;
                    grid.tiles[(*newTail).y][(*newTail).x] = grid.tiles[(*(--ai.snakeParts.end())).y][(*(--ai.snakeParts.end())).x];
                }
                for (auto part_it = followPartBitten; part_it != ai.snakeParts.end(); part_it++)
                {
                    const int bloodLostPerTail = 2;
                    stats.blood -= bloodLostPerTail;
                    grid.tiles[(*part_it).y][(*part_it).x] = {EMPTY, 0};
                }
                ai.snakeParts.erase(followPartBitten, ai.snakeParts.end());
            }
            if (hit.group == PLAYER)
            {
                std::cout << "GAME OVER!" << std::endl;
                FriendLimit::shouldSetupNewGame = true;
                BicycleMango::brake = true;
            }
            bool growSnake = hit.group == APPLE;
            if (growSnake)
            {
                // Decide if we need to spawn a tail or body part
                if (ai.snakeParts.size() == 1)
                {
//                     std::cout << "Spawn snake tail" << std::endl;
                    grid.tiles[request.from.y][request.from.x] = BicycleMango::Next(SNAKE_TAIL);
                    ai.snakeParts.push_back({request.from.x, request.from.y});
                    grid.tiles[request.to.y][request.to.x] = current;
                } else
                {
//                     std::cout << "Spawn snake body" << std::endl;
                    grid.tiles[request.from.y][request.from.x] = BicycleMango::Next(SNAKE_BODY);
                    ai.snakeParts.insert(ai.snakeParts.begin() + 1, {request.from.x, request.from.y});
                }
                // When the snake grows, we don't update the movement of the entire snake: only the head
                grid.tiles[request.to.y][request.to.x] = current;
                switch (hit.group)
                {
                    case APPLE:
                        stats.blood++;
                        break;
                }
            } else
            {
                sf::Vector2i prevPartPos = request.from;
                grid.tiles[request.from.y][request.from.x] = {EMPTY, 0};
                grid.tiles[request.to.y][request.to.x] = current;
                for (size_t p = 1; p < ai.snakeParts.size(); p++)
                {
                    sf::Vector2i followPos = ai.snakeParts[p];
                    ai.snakeParts[p] = prevPartPos;
                    grid.tiles[prevPartPos.y][prevPartPos.x] = grid.tiles[followPos.y][followPos.x];
                    grid.tiles[followPos.y][followPos.x] = {EMPTY, 0}; // Only really need to do this for the tail
                    prevPartPos = followPos;
                }
            }
        } 
        else if (current.group == PLAYER)
        {
            if (hit.group == SNAKE_HEAD)
            {
                menu.status = MenuManager::Status::GAME_OVER;
                std::cout << "GAME OVER!" << std::endl;
                FriendLimit::shouldSetupNewGame = true;
                BicycleMango::brake = true;
            }
            const std::set<Group> playerBlockedBy = {SNAKE_BODY, SNAKE_TAIL, KNIGHT, PEASANT, HERO};
            if (playerBlockedBy.count(hit.group))
            {
                std::cout << "Player blocked by " << hit << std::endl;
                moveSuccessful = false;
            } else
            {
                grid.tiles[request.from.y][request.from.x] = {EMPTY, 0};
                grid.tiles[request.to.y][request.to.x] = current;
            }
        }
        else
        {
            grid.tiles[request.from.y][request.from.x] = {EMPTY, 0};
            grid.tiles[request.to.y][request.to.x] = current;
        }
        if (moveSuccessful)
        {
            BicycleMango::GetProps<Mover>()[request.moverId].pos = request.to;
        }
    }
    resolver.requestedMoves.clear();
    stats.swords = ai.snakeParts.size();
}

void RenderGrid_Act(WindowGameState& game, Grid& grid, SnakeAI& ai)
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
                case SNAKE_BODY:
                case SNAKE_TAIL:
                    for (size_t i = 0; i < ai.snakeParts.size(); i++)
                    {
                        if (ai.snakeParts[i] == sf::Vector2i{col, row})
                        {
                            int tone = std::max(64, 255 - (((int)i/3) * 32));
                            tiles.setColor(sf::Color(tone, tone, tone));
                        }
                    }
                    tiles.setPosition(col * grid.tile_width - 1, grid.offset_y + row * grid.tile_height - 1);
                    tiles.setTextureRect({(g - SNAKE_HEAD) * (grid.tile_width + 2), 0, grid.tile_width + 2, grid.tile_height + 2});
                    game.window.draw(tiles);
                    tiles.setColor(sf::Color::White);
                    break;
                default:
                    tiles.setPosition(col * grid.tile_width - 1, grid.offset_y + row * grid.tile_height - 1);
                    tiles.setTextureRect({(g - SNAKE_HEAD) * (grid.tile_width + 2), 0, grid.tile_width + 2, grid.tile_height + 2});
                    game.window.draw(tiles);
            }
        }
    }
}

void DisplayUpdateStats_Act(WindowGameState& game, StatRenderInfo& statInfo, ConflictStats& stats)
{
    statInfo.blood.setString(std::to_string(stats.blood));
    game.window.draw(statInfo.blood);
    statInfo.swords.setString(std::to_string(stats.swords));
    statInfo.swords.setPosition(statInfo.swordsAnchorPos.x - statInfo.swords.getLocalBounds().width, statInfo.swordsAnchorPos.y);
    game.window.draw(statInfo.swords);
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
