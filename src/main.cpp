#include "sun_lambda.h"
#include "props.h"

#include "SunLambdas/Suns.h"

#include "resources.h"

#include <SFML/Graphics.hpp>

int main() 
{
    srand(time(NULL));
    Resources::inst = new Resources();
    
//     sf::Music soundtrack;
//     if (!soundtrack.openFromFile("resources/sounds/distant-friend.ogg"))
//     {
//         std::cout <<"Soundtrack failed to load" << std::endl;
//     }
//     soundtrack.setLoop(true);
//     soundtrack.play();
    
    BicycleMango::Emerge(SetupGrid::Id());
    
    // Suns
    // SUNS MUST BE PLANNED BEFORE ADDING PROPS SO THAT THERE IS A NON-NULL COMPATABILITY CONSTRANT CURRENTLY
    // What should probably be done instead is only require adding compatability constraints before adding props, not necessarily all planning
    BicycleMango::Plan(PollWindowEvents::Id(), {FORAGE, 3});
    BicycleMango::Plan(TickMoveEvent::Id(), {FORAGE, 100});
    
    auto moverIsPlayer = 
    [](BicycleMango::PropTypeId propTypeId, const Stage& stage) -> bool
    {
        if (BicycleMango::GetPropTypeId<Mover>() == propTypeId)
            return stage.group == PLAYER;
        return true;
    };
    BicycleMango::Plan(PlayerChangeMovementDirection::Id(), {INPUT}, moverIsPlayer);
    BicycleMango::Plan(PlayerPlaceTiles::Id(), {INPUT}, moverIsPlayer);
    BicycleMango::Plan(EvaluateMoves::Id(), {UPDATE, 100});
    // TODO: More convenient way to add partial statics
//     BicycleMango::novelTupleCreators[EvaluateMoves::Id()].stageConstraints[BicycleMango::GetPropTypeId<Grid>()] = 
//     [](const Stage&){return true;};
//     BicycleMango::novelTupleCreators[EvaluateMoves::Id()].stageConstraints[BicycleMango::GetPropTypeId<MoveResolver>()] = 
//     [](const Stage&){return true;};
    BicycleMango::Plan(DisplayWindow::Id(), {DISPLAY});
    BicycleMango::Plan(SpriteRenderer::Id(), {RENDER});
    BicycleMango::Plan(DisplayUpdateStats::Id(), {RENDER, 10});
    BicycleMango::Plan(RenderGrid::Id(), {RENDER, 5});

#ifdef HOT_RELOAD
    BicycleMango::Plan(HotReloadWatcher::Id(), {UPDATE});
#endif
    
    // Props
    
    auto game = BicycleMango::AddProp<WindowGameState>({{GAME_STATE, 0}}); // I want to specify that this prop can be reused in multiple novel tuples
    // More generically, I want to specify that ANY prop of WindowGameState can be reused in multiple novel tuples on GAME_STATE
    game->window.setVerticalSyncEnabled(true);
    game->window.setKeyRepeatEnabled(false);
    game->window.create(sf::VideoMode(128, 160), "Friend Limit", sf::Style::Titlebar | sf::Style::Close);
    auto title = BicycleMango::AddProp<Sprite>({{GAME_STATE, 0}, {TITLE_SCREEN, 0}});
    title->sprite.setTexture(*Resources::inst->LoadTexture("play-area.png"));
    
    BicycleMango::AddProp<ConflictStats>({{UI, 0}, {SNAKE, 0}});
    auto stats = BicycleMango::AddProp<StatRenderInfo>({{SNAKE, 0}});
    stats->statFont = *Resources::inst->LoadFont("madness.ttf");
    stats->blood.setFont(stats->statFont);
    stats->blood.setCharacterSize(16);
    stats->blood.setOrigin(0, 8);
    stats->blood.setFillColor(sf::Color::Red);
    stats->blood.setPosition(44, 7);
    
    stats->swords.setFont(stats->statFont);
    stats->swords.setCharacterSize(16);
    stats->swords.setOrigin(0, 8);
    stats->swords.setFillColor(sf::Color(148, 0, 255));
    stats->swordsAnchorPos = {84, 7};
    
    auto grid = BicycleMango::AddProp<Grid>({{GAME_STATE, 0}});
    auto moveResolver = BicycleMango::AddProp<MoveResolver>({{GAME_STATE, 0}});
    moveResolver->beatSFX.setBuffer(*Resources::inst->LoadSoundBuffer("beat.wav"));
    
    // TODO Variadic stages for AddProp
    
    auto snakeMover = BicycleMango::AddProp<Mover>({{SNAKE, 0}, {SNAKE_HEAD, 0}});
    snakeMover->pos = {grid->cols - 2, grid->rows - 2};
    snakeMover->prevMove = snakeMover->nextMove = Direction::NORTH;
    
    auto playerMover = BicycleMango::AddProp<Mover>({{PLAYER, 0}, {GAME_STATE, 0}});
    playerMover->pos = {1, 1};
    playerMover->prevMove = playerMover->nextMove = Direction::SOUTH;
    
    
    while (!BicycleMango::brake)
    {
        BicycleMango::Loop();
    }
    
    delete Resources::inst;

    return 0;
}
