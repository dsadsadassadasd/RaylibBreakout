#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <vector>
using namespace std;

Color black = { 33, 34, 34, 255 };
// Global sound variables for each scenario
Sound bounceSound;
Sound gameOverSound;
Sound winSound;
// Represents a single brick in the grid
struct Bricks { 
    Vector2 position;
    Rectangle rect;
    bool active;
    Texture2D texture;
};
// Manages the brick grid and drawing of the bricks
class BricksGrid {
private:
    int rows;
    int cols;
    float brickWidth;
    float brickHeight;
    std::deque<Bricks> bricks;
    std::vector<Texture2D>brickTextures;

public:
    BricksGrid(int numRows, int numCols, float width, float height, const std::vector<Texture2D>& textures) {
    rows = numRows;
    cols = numCols;
    brickWidth = width;
    brickHeight = height;
    brickTextures = textures;

    float spacing = 7;
    float totalWidth = cols * brickWidth + (cols - 1) * spacing;
    float startX = (GetScreenWidth() - totalWidth) / 2;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            Bricks brick;
            brick.position = {floor(startX + c * (brickWidth + spacing)), floor(60 + r * (brickHeight + spacing))};

            brick.rect = {brick.position.x, brick.position.y, 65, 30};
            brick.active = true;

            if (!brickTextures.empty()) {
                int randomTexture = GetRandomValue(0, brickTextures.size() - 1);
                brick.texture = brickTextures[randomTexture];
            }
            bricks.push_back(brick);

            }
        }
    }
    // Resets all the bricks to active and randomizes textures for the next level
    void ResetBricks() {
        for (Bricks& brick : bricks) {
            brick.active = true;

            if(!brickTextures.empty()) {
                int randomTexture = GetRandomValue(0, brickTextures.size() - 1);
                brick.texture = brickTextures[randomTexture];
            }
        }
    }
    // Draws all active bricks in the grid
    void DrawBricks() {
        for (const auto& brick : bricks) {
            if (brick.active) {
                DrawTexturePro(brick.texture, Rectangle{0, 0, (float)brick.texture.width, (float)brick.texture.height}, brick.rect, Vector2{0, 0}, 0.0f, WHITE);
            }
        }
    }
    std::deque<Bricks>& GetBricks() {
    return bricks;
        }
};
// Represents the ball in the game, such as its movement, drawing, and resetting
class Ball {
public:
    float x, y;
    int speedX, speedY;
    int radius;
    bool launched = false;
    Texture2D texture;

    void Draw() {
        DrawTexture(texture, x - radius, y - radius, WHITE);
    }

    void Update() {
        if (!launched) {
            return;
        }
        x += speedX;
        y += speedY;

        if (y - radius <= 0) {
            y = radius;
            speedY *= -1;
            PlaySound(bounceSound);
            
        }
        if (x + radius >= GetScreenWidth()) {
            x = GetScreenWidth() - radius;
            speedX *= -1;
            PlaySound(bounceSound);
        }
        if (x - radius <= 0) {
            x = radius;
            speedX *= -1;
            PlaySound(bounceSound);
        }
        }

    void Reset() {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;
    }

};
// Represents the paddle in the game, such as its movement, drawing, and resetting
class Paddle {

protected:
// Keeps the paddle within the window
    void LimitMovement() {
        if (x <= 0) {
            x = 0;
        }
        if (x + width >= GetScreenWidth()) {
            x = GetScreenWidth() - width;
        }
    }
public:
    float x, y;
    float width, height;
    int speed;
    Texture2D texture;

    void Draw() {
        DrawTexture(texture, x, y, WHITE);
    }
    // Allows for the paddle to move left or right based on whether the left or right arrow key is pressed.
    // It also limits the paddle using the LimitMovement function made prior.
    void Update() {
            if (IsKeyDown(KEY_LEFT)) {
                x = x - speed;
            }
            if (IsKeyDown(KEY_RIGHT)) {
                x = x + speed;
            }
            LimitMovement();
        }

    // Resets the paddle to the middle of the screen
    void Reset() {
        x = GetScreenWidth() / 2 - width / 2;
        y = GetScreenHeight() - height - 20;
    }

};
// Handles the overall game state, including the ball, paddle, bricks, and more
class Game {
public:
    Game(const std::vector<Texture2D>& textures) : bricks(4, 7, 65, 30, textures) {}
    Ball ball = Ball();
    BricksGrid bricks;
    Paddle paddle = Paddle();
    bool running = true;
    int player_lives = 3;
    int player_score = 0;
    int level = 1;
    int ballSpeed = 6;
    bool winSoundPlayed = false;

    // Starts the game by launching the ball when the up arrow key is pressed
    void Start() {
        if (!ball.launched) {
            ball.x = paddle.x + paddle.width/2;
            ball.y = paddle.y - ball.radius - 2;
            if (IsKeyPressed(KEY_UP) && !ball.launched) {
                ball.launched = true;
                ball.speedX = ballSpeed;
                ball.speedY = -ballSpeed;
                PlaySound(bounceSound);
            }
        }
    }
    // Stops running the game and plays the game over sound
    void GameOver() {
        running = false;
        ball.launched = false;
        PlaySound(gameOverSound);
    }
    // Checks if the player's lives reaches zero and triggers the game over function if its found true
    void CheckGameOver() {
        if(player_lives <= 0) {
            GameOver();
            }
        }
    // Checks if the ball goes out of bounds and reduces the players lives, while also resetting the ball's position to the middle.
    void CheckBallOutOfBounds() {
        if (ball.y + ball.radius >= GetScreenHeight()) {
            player_lives--;
            ball.Reset();
            PlaySound(gameOverSound);
        }
    }
    // Checks if the ball hits a brick, determines the collision side,
    // destroys the brick, increases the score, and reverses the ball's direction
    void CheckCollisionWithBrick() {
        for (Bricks& brick : bricks.GetBricks()) {
            if (brick.active && CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, brick.rect)) {
                // Calculates the overlap on each side to determine where the collision happened
                // The smallest overlap determines the collision side
                float overlapLeft = (ball.x + ball.radius) - brick.rect.x;
                float overlapRight = (brick.rect.x + brick.rect.width) - (ball.x - ball.radius);
                float overlapTop = (ball.y + ball.radius) - brick.rect.y;
                float overlapBottom = (brick.rect.y + brick.rect.height) - (ball.y - ball.radius);
                // Checks which side of the brick the ball collides with and reverses the ball's direction accordingly
                if (overlapLeft < overlapRight && overlapLeft < overlapTop && overlapLeft < overlapBottom) {
                    ball.speedX *= -1;
                }
                else if (overlapRight < overlapLeft && overlapRight < overlapTop && overlapRight < overlapBottom) {
                    ball.speedX *= -1;
                }
                else if (overlapTop < overlapLeft && overlapTop < overlapRight && overlapTop < overlapBottom) {
                    ball.speedY *= -1;
                }
                else {
                    ball.speedY *= -1;
                }

                brick.active = false;
                player_score += 250;
                PlaySound(bounceSound);
                break;
            }
        }
    }
    // Checks collision between the ball and the paddle and decides the ball's speed and direction based on where its hit
    void CheckCollisionWithPaddle() {
        if(CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{paddle.x, paddle.y, paddle.width, paddle.height})) {
            float paddleCenter = paddle.x + paddle.width / 2;
            float offset = ball.x - paddleCenter;

            ball.y = paddle.y - ball.radius;

            if(offset < -paddle.width/6) {
                ball.speedX = -ballSpeed;
                ball.speedY = -ballSpeed;
                PlaySound(bounceSound);
            }
            else if (offset > paddle.width/6) {
                ball.speedX = ballSpeed;
                ball.speedY = -ballSpeed;
                PlaySound(bounceSound);
            }
            else {
                ball.speedX = 2;
                ball.speedY = -ballSpeed - 2;
                PlaySound(bounceSound);
            }
        }
    }
    // A function that checks if all bricks are destroyed
    // If so, it plays the win sound and increases the level and ball speed
    // It then resets the bricks and sets the ball to relaunch
    void CheckLevelComplete() {
        bool levelComplete = true;

        for (Bricks& brick : bricks.GetBricks()) {
            if (brick.active) {
                levelComplete = false;
                break;
            }
        }
        if (levelComplete && !winSoundPlayed) {
            PlaySound(winSound);
            winSoundPlayed = true;
            level++;
            ballSpeed++;

            bricks.ResetBricks(); // Resets the bricks and randomizes textures
            winSoundPlayed = false;

            ball.launched = false;
        }
    }
};

int main() {
    
    InitWindow(650, 650, "Breakout Game");
    InitAudioDevice();
    bounceSound = LoadSound("Sounds/bounce.wav");
    gameOverSound = LoadSound("Sounds/gameover.mp3");
    winSound = LoadSound("Sounds/win.mp3");

    Texture2D redBrickTexture = LoadTexture("Textures/red_brick.png");
    Texture2D yellowBrickTexture = LoadTexture("Textures/yellow_brick.png");
    Texture2D blueBrickTexture = LoadTexture("Textures/blue_brick.png");
    Texture2D greenBrickTexture = LoadTexture("Textures/green_brick.png");
    Texture2D ballTexture = LoadTexture("Textures/pixel_ball.png");
    Texture2D paddleTexture = LoadTexture("Textures/pixel_paddle.png");

    std::vector<Texture2D> textures =  {redBrickTexture, yellowBrickTexture, greenBrickTexture, blueBrickTexture};
    Game game(textures);
    SetTargetFPS(60);
    
    game.ball.radius = 14;
    game.ball.x = GetScreenWidth() / 2;
    game.ball.y = GetScreenHeight() - 60;
    game.ball.texture = ballTexture;

    game.ball.speedX = 6;
    game.ball.speedY = 6;

    game.paddle.width = 130;
    game.paddle.height = 15;
    game.paddle.texture = paddleTexture;
    game.paddle.x = GetScreenWidth() / 2 - game.paddle.width / 2;
    game.paddle.y = GetScreenHeight() - game.paddle.height - 20;
    game.paddle.speed = 10;

    while(WindowShouldClose() == false) 
    {
        if (game.running) {
            game.paddle.Update();
            game.Start();
            game.ball.Update();
            game.CheckBallOutOfBounds();
            game.CheckGameOver();
            game.CheckCollisionWithBrick();
            game.CheckLevelComplete();

            game.CheckCollisionWithPaddle();
    }

        BeginDrawing();

        ClearBackground(black);

        game.ball.Draw();
        game.paddle.Draw();
        game.bricks.DrawBricks();
        if (!game.running) {
            DrawText("Press Enter to Restart", GetScreenWidth() / 2 - MeasureText("Press Enter to Restart", 20) / 2, 650 / 2 + 20, 20, WHITE);
            if (IsKeyPressed(KEY_ENTER)) {
                    game.running = true;
                    game.player_score = 0;
                    game.ball.launched = false;
                    game.player_lives = 3;
                    game.ball.x = GetScreenWidth() / 2;
                    game.ball.y = GetScreenHeight() - 60;
                    game.ball.speedX = 6;
                    game.ball.speedY = 6; 
                    game.ballSpeed = 6; // Reset ball speed to initial value
                    game.paddle.Reset();
                    game.level = 1;
                    game.winSoundPlayed = false;
                    game.bricks.ResetBricks();

                }
            }

        DrawText(TextFormat("Lives: %i", game.player_lives), 5, 10, 20, WHITE);
        DrawText(TextFormat("Score: %i", game.player_score), GetScreenWidth() / 2 - MeasureText(TextFormat("Score: %i", game.player_score), 20) / 2, 10, 20, WHITE);
        DrawText(TextFormat("Level: %i", game.level), GetScreenWidth() - MeasureText(TextFormat("Level: %i", game.level), 20) - 5, 10, 20, WHITE);
        EndDrawing();
    }
    UnloadTexture(redBrickTexture);
    UnloadTexture(blueBrickTexture);
    UnloadTexture(yellowBrickTexture);
    UnloadTexture(greenBrickTexture);
    UnloadTexture(ballTexture);
    UnloadTexture(paddleTexture);

    UnloadSound(bounceSound);
    UnloadSound(gameOverSound);
    UnloadSound(winSound);
    CloseWindow();
    return 0;
    }