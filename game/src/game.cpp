/*
    FlepBerd by Alexander Klann; (C) 2024

    A basic physics-based FlappyBird clone in Raylib.
*/

#include "raylib.h"
#include "rlgl.h"
#include "rres.h"
#include "rres-raylib.h"
#include "dev_utils.h"
#include "ResourceRegister.h"
#include <iostream>
#include <string>

Vector2 bird_pos = {400, 225};
Vector2 bird_velocity = { 0, 0 };
int bird_angle = 0;

const int MAX_POSITIVE_BIRD_VELOCITY = -250;
const int MAX_NEGATIVE_BIRD_VELOCITY = 500;

const int MAX_POSITIVE_BIRD_ANGLE = 85;
const int MAX_NEGATIVE_BIRD_ANGLE = -45;

const int PIPE_GAP = 200;
const int PIPE_PORTAL_GAP = 100;

const int PIPE_WIDTH = 30;

const int SCROLLING_SPEED = 200;

const Vector2 SCREEN_PROPORTIONS = { 800, 450 };

typedef enum GameScreen {
    TITLE,
    GAMEPLAY
} GameScreen;

typedef enum PipeDirection {
    UPPER,
    LOWER
} PipeDirection;

typedef struct Pipe {
    PipeDirection direction;
    Vector2 position;
    Vector2 dimensions;
} Pipe;

typedef struct PipePair {
    Pipe upper_pipe;
    Pipe lower_pipe;
    bool points_collected;
} PipePair;

PipePair generate_pipe_pair() {
    int randomPipeHeight = GetRandomValue(20, GetScreenHeight() * 0.6);

    Pipe upperPipe = {
        PipeDirection::UPPER,
        {
            GetScreenWidth(), // Place pipe at the end of the screen
            0 // Since it's the top pipe and the coordinates of a rectangle start in the top-left
        },
        {
            PIPE_WIDTH,
            randomPipeHeight
        }
    };

    Pipe lowerPipe = {
        PipeDirection::UPPER,
        {
            GetScreenWidth(), // Place pipe at the end of the screen
            upperPipe.dimensions.y + PIPE_PORTAL_GAP // Pipe starting point placed under the top pipe
        },
        {
            PIPE_WIDTH,
            GetScreenHeight() - upperPipe.dimensions.y + PIPE_PORTAL_GAP // Calculate the remaining space between end of the window and the top pipe
        }
    };

    PipePair pipePair = { upperPipe, lowerPipe, false};
    return pipePair;
}

struct PipePair pipes[5];

const int BIRD_GRAVITY = 1000;
const int FLAP_FORCE = 1750;

//const Color BIRD_COLOR = { 0xf5, 0xc5, 0x42, 0xff };
const Color BIRD_COLOR = HexCode("#f5c542");
const Color PIPE_COLOR = HexCode("#2fc258");
const Color GAME_OVER_TEXT_COLOR = HexCode("#000000");
const Vector2 BIRD_SIZE = { 20, 20 };

GameScreen currentScreen = GameScreen::TITLE;

int points = 0;

bool space_pressed = false;
bool is_alive = true;
bool generate_pipes = true;
bool points_lock = false;

Sound flapSound{};
Sound deathSound{};
Sound pointSound{};
Sound combo10Sound{};
Sound combo50Sound{};
Sound combo100Sound{};
Image flepBerdLogo{};

void SetupResources() {
    const char* RESOURCE_FILE_NAME = "resources.rres";
    const rresCentralDir CENTRAL_DIR = rresLoadCentralDirectory("resources.rres");

    flepBerdLogo = RegisterImage(CENTRAL_DIR, RESOURCE_FILE_NAME, "flepberd_icon.png");

    flapSound = RegisterSound(CENTRAL_DIR, RESOURCE_FILE_NAME, "jump.wav");
    pointSound = RegisterSound(CENTRAL_DIR, RESOURCE_FILE_NAME, "point.wav");
    combo10Sound = RegisterSound(CENTRAL_DIR, RESOURCE_FILE_NAME, "combo10.wav");
    combo50Sound = RegisterSound(CENTRAL_DIR, RESOURCE_FILE_NAME, "combo50.wav");
    combo100Sound = RegisterSound(CENTRAL_DIR, RESOURCE_FILE_NAME, "combo100.wav");
    deathSound = RegisterSound(CENTRAL_DIR, RESOURCE_FILE_NAME, "death.wav");

    rresUnloadCentralDirectory(CENTRAL_DIR);
}

void ResetGameLoop() {
    bird_pos = { SCREEN_PROPORTIONS.x / 2, SCREEN_PROPORTIONS.y / 2 };
    bird_velocity = { 0, 0 };
    points = 0;

    for (int i = 0; i < 5; i++) {
        pipes[i] = {};
    }

    is_alive = true;
    generate_pipes = true;
}

void TriggerDeath() {
    PlaySound(deathSound);
    is_alive = false;
}

void MaintainPipesInBounds(PipePair& pipePair) {
    Pipe& upperPipe = pipePair.upper_pipe;
    Pipe& lowerPipe = pipePair.lower_pipe;

    lowerPipe.position.x = upperPipe.position.x -= SCROLLING_SPEED * GetFrameTime();

    bool pipesOutOfBounds = upperPipe.position.x < -upperPipe.dimensions.x || lowerPipe.position.x < -lowerPipe.dimensions.x;

    if (pipesOutOfBounds) {
        pipePair = generate_pipe_pair(); // Generate new Pipe

        upperPipe.position.x = GetScreenWidth() + PIPE_GAP - PIPE_WIDTH;
        lowerPipe.position.x = GetScreenWidth() + PIPE_GAP - PIPE_WIDTH;
    }
}

void CheckForCollisions(PipePair& pipePair) {
    Pipe& upperPipe = pipePair.upper_pipe;
    Pipe& lowerPipe = pipePair.lower_pipe;

    Rectangle birdRect = { bird_pos.x, bird_pos.y, BIRD_SIZE.x - 10, BIRD_SIZE.y - 10 };
    Rectangle upperPipeRect = { upperPipe.position.x, upperPipe.position.y, upperPipe.dimensions.x, upperPipe.dimensions.y };
    Rectangle lowerPipeRect = { lowerPipe.position.x, lowerPipe.position.y, lowerPipe.dimensions.x, lowerPipe.dimensions.y };
    Rectangle pipeDeltaRect = { lowerPipe.position.x, upperPipe.dimensions.y, lowerPipe.dimensions.x, lowerPipe.position.y - upperPipe.dimensions.y };

    bool collidedWithUpperPipe = CheckCollisionRecs(birdRect, upperPipeRect);
    bool collidedWithLowerPipe = CheckCollisionRecs(birdRect, lowerPipeRect);
    bool passedThroughPipes = CheckCollisionRecs(birdRect, pipeDeltaRect);

    if (collidedWithUpperPipe) TriggerDeath();
    if (collidedWithLowerPipe) TriggerDeath();

    if (passedThroughPipes && !pipePair.points_collected) {
        pipePair.points_collected = true;
        points++;
        PlaySound(pointSound);
        if (points % 100 == 0) {
            PlaySound(combo100Sound);
            return;
        }
        else if (points % 50 == 0) {
            PlaySound(combo50Sound);
            return;
        }
        else if (points % 10 == 0) {
            PlaySound(combo10Sound);
            return;
        }
    }
}

void DrawTitleScreen() {
    DrawText("FlepBerd", (GetScreenWidth() - MeasureText("FlepBerd", 36)) / 2, (GetScreenHeight() - 36) / 2, 36, GAME_OVER_TEXT_COLOR);
    DrawText("Press SPACE to start", (GetScreenWidth() - MeasureText("Press SPACE to start", 24)) / 2, (GetScreenHeight() + 36) / 2, 24, GAME_OVER_TEXT_COLOR);
}

void DrawGameplayScreen() {
    DrawRectanglePro({ bird_pos.x, bird_pos.y, BIRD_SIZE.x, BIRD_SIZE.y }, { BIRD_SIZE.x / 2, BIRD_SIZE.y / 2 }, bird_angle, BIRD_COLOR); // Bird

    // Pipe Pair Generation
    for (int i = 0; i < 5; i++) {
        // Yes I hardcoded it idc
        Pipe upper_pipe = pipes[i].upper_pipe;
        DrawRectangle(upper_pipe.position.x, upper_pipe.position.y, upper_pipe.dimensions.x, upper_pipe.dimensions.y, PIPE_COLOR);
        Pipe lower_pipe = pipes[i].lower_pipe;
        DrawRectangle(lower_pipe.position.x, lower_pipe.position.y, lower_pipe.dimensions.x, lower_pipe.dimensions.y, PIPE_COLOR);
    }

    DrawText(TextFormat("Score: %03i", points), 20, 20, 24, GAME_OVER_TEXT_COLOR);

    if (!is_alive) {
        DrawText("Game Over", (GetScreenWidth() - MeasureText("Game Over", 36)) / 2, (GetScreenHeight() - 36) / 2, 36, GAME_OVER_TEXT_COLOR);
        DrawText("Press SPACE to restart", (GetScreenWidth() - MeasureText("Press SPACE to restart", 24)) / 2, (GetScreenHeight() + 36) / 2, 24, GAME_OVER_TEXT_COLOR);
    }
}


int main(void)
{
    InitAudioDevice();
    InitWindow(SCREEN_PROPORTIONS.x, SCREEN_PROPORTIONS.y, "FlepBerd");
    SetupResources();
    SetWindowIcon(flepBerdLogo);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (currentScreen == GameScreen::TITLE) {
            if (IsKeyDown(KEY_SPACE)) {
                currentScreen = GameScreen::GAMEPLAY;
            }
        }
        else {
            if (generate_pipes) {
                // Initial Pipe Generation
                for (int i = 0; i < 5; i++) {
                    // Generate pipe at the right of the screen
                    pipes[i] = generate_pipe_pair();

                    // If not the first pipe then make distance to last pipe relative.
                    // Only used for the first time the pipes are loaded. After that
                    // each pipe gets put all the way at the back of the level again when
                    // it leaves the boundaries on its own.
                    if (i != 0) {
                        pipes[i].upper_pipe.position.x = pipes[i - 1].upper_pipe.position.x + PIPE_GAP;
                        pipes[i].lower_pipe.position.x = pipes[i - 1].lower_pipe.position.x + PIPE_GAP;
                    }
                }
                generate_pipes = false;
            }

            // Physics Calculation
            if (is_alive) {
                // Gravity
                bird_velocity.y += BIRD_GRAVITY * GetFrameTime();
                bird_pos.y += bird_velocity.y * GetFrameTime();
                bird_angle += 100 * GetFrameTime();

                // Physics and angle calculations
                bird_angle = clamp(bird_angle, MAX_NEGATIVE_BIRD_ANGLE, MAX_POSITIVE_BIRD_ANGLE);
                bird_velocity.y = clamp(bird_velocity.y, MAX_POSITIVE_BIRD_VELOCITY, MAX_NEGATIVE_BIRD_VELOCITY);
                bird_pos.y = clamp(bird_pos.y, 0, 450);

                if (IsKeyDown(KEY_SPACE) && !space_pressed) {
                    bird_velocity.y -= FLAP_FORCE;
                    bird_angle = MAX_NEGATIVE_BIRD_ANGLE;
                    SetSoundPitch(flapSound, 1.0 + (1 - (bird_pos.y / GetScreenHeight())));
                    PlaySound(flapSound);
                    space_pressed = true;
                }

                if (IsKeyReleased(KEY_SPACE)) {
                    space_pressed = false;
                }

                // Bounce-back effect when touching roof of level
                if (bird_pos.y == 0) {
                    bird_velocity.y = MAX_NEGATIVE_BIRD_VELOCITY;
                }

                // Check if bird touched ground
                if (bird_pos.y == GetScreenHeight() && is_alive) TriggerDeath();

                const int PIPE_ARRAY_SIZE = sizeof(pipes) / sizeof(pipes[0]);
                for (int i = 0; i < PIPE_ARRAY_SIZE; i++) {
                    MaintainPipesInBounds(pipes[i]);
                    CheckForCollisions(pipes[i]);
                }
            }
            else {
                if (IsKeyDown(KEY_SPACE)) {
                    ResetGameLoop();
                }
            }
        }
         
        BeginDrawing();
        ClearBackground(RAYWHITE);
        switch (currentScreen) {
            case TITLE:
                DrawTitleScreen();
                break;
            case GAMEPLAY:
                DrawGameplayScreen();
                break;
        }
        EndDrawing();
    }

    UnloadSound(flapSound);
    UnloadSound(deathSound);
    UnloadSound(pointSound);
    UnloadSound(combo10Sound);
    UnloadSound(combo50Sound);
    UnloadSound(combo100Sound);
    UnloadImage(flepBerdLogo);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}   