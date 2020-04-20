#pragma once

#include "bicycle_mango.h"
#include "props.h"

// Dismiss Title Screen
// Move Player
// Place Apple/Treasure
// Spawn Units
// Unit Movement AI
// Snake Movement AI
// Resolve Grid Changes (this includes updating SNAKE)
// Detect/Render/Reset Game Over

// TODO Make sure typesets are in the same order always (for performance) and enforce that :/
// TODO: Refactor WindowGameState into another Events prop for clarity

DeclareSunLambda(EnterGame, WindowGameState&, MenuManager&, Sprite&);
DeclareSunLambda(RenderCursor, WindowGameState&, Mover&);
DeclareSunLambda(SetPrevPos, Mover&);
DeclareSunLambda(PlayerPlaceTiles, WindowGameState&, Grid&, Mover&, AudioManager&);
DeclareSunLambda(PlayerChangeMovementDirection, WindowGameState&, Mover&);
DeclareSunLambda(TickMoveEvent, MoveResolver&);
// Declare MoveResolver and Grid can be reused with PartialStaticIndicators
DeclareSunLambda(MoverDirectionChoiceAI, Grid&, Mover&, MoveResolver&);
DeclareSunLambda(EvaluateMoves, Grid&, MoveResolver&, Mover&);
DeclareSunLambda(ResolveMoves, MoveResolver&, Grid&, SnakeAI&, ConflictStats&, AudioManager&);

DeclareSunLambda(SetupGrid, Grid&);
DeclareSunLambda(RenderGrid, WindowGameState&, Grid&, SnakeAI&);

DeclareSunLambda(DisplayUpdateStats, WindowGameState&, StatRenderInfo&, ConflictStats&);

DeclareSunLambda(PollWindowEvents, WindowGameState&);
DeclareSunLambda(DisplayWindow, WindowGameState&);

#ifdef HOT_RELOAD
DeclareSunLambda(HotReloadWatcher, WindowGameState&)
#endif

DeclareSunLambda(SpriteRenderer, WindowGameState&, Sprite&);
