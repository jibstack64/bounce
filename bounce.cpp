#include <raylib.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>

// holds the height and width of a movement.
struct vector {
    int x;
    int y;

    vector(int x, int y) : x(x), y(y) {}
};

// holds a game object's data.
struct object {
    // the type of object
    enum objectType {
        Circle, Rectangle, Text
    };

    const int width;
    const int height;
    const objectType type;
    char * value; // for text types
    vector location;

    // increments the object's coordinates by the ones provided. returns true if the movement was a success.
    bool move(int x, int y) {
        // check for collisions
        vector n(location.x+x, location.y+y);
        if (n.y >= GetScreenHeight() - height || n.y <= 0 || n.x >= GetScreenWidth() - width || n.x <= 0) {
            return false;
        } else {
            location = n;
            return true;
        }
    }

    // increments the object's coordinates by those in the vector. returns true if the movement was a success.
    bool move(vector& v) {
        return move(v.x, v.y);
    }

    // if the object is touching the object provided, a vector containing the
    // coordinates in which they touch is returned. a nullptr is returned otherwise.
    vector* touching(object& object) {
        vector& r = location;
        vector& v = object.location;
        if (r.x < v.x + object.width && r.x > v.x && r.y > v.y && r.y <= v.y + object.height) {
            return &(location);
        } else {
            //std::cout << "NO!" << std::endl;
            return nullptr;
        }
    }

    object(int width, int height, objectType type, int x = 0, int y = 0) : width(width), height(height),
            type(type), location(x, y) {}
    object(int width, int height, objectType type, vector location = vector(0, 0)) : width(width), height(height),
            type(type), location(location) {}
};

// a glob containing all game objects
std::vector<object*> objectGlob;

int main(int argc, char ** argv) {
    const int screenWidth = 750;
    const int screenHeight = 750;
    const int move = 10;
    const float gravity = 9.8;
    const int fps = 60;
    const char * title = "Bounce";
    int speed = argc > 1 ? atoi(argv[1]) : 6; // increases every point scored

    InitWindow(screenWidth, screenHeight, title);
    InitAudioDevice();
    SetTargetFPS(fps);

    // load sounds
    Sound lose = LoadSound("./sounds/lose.wav");
    Sound hit = LoadSound("./sounds/hit.wav");

    // create balls, walls and text
    vector starting(screenWidth/2 - 5, screenHeight/2 - 5);
    object ball(10, 10, object::Circle, starting);
    object leftWall(10, 200, object::Rectangle, 10, screenHeight/2 - 100);
    object rightWall(10, 200, object::Rectangle, screenWidth - 20, screenHeight/2 - 100);
    objectGlob = {&ball, &leftWall, &rightWall};

    // scores
    int player1Score = 0;
    int player2Score = 0;

    // vector in which the ball is moved by
    vector blmv(speed, 0);

    while (!WindowShouldClose()) {
        // if ball is touching a wall, flip the direction
        vector* locTouch = ball.touching(leftWall);
        object* touchedWall = &leftWall;
        if (locTouch != nullptr) {
            goto calculate;
        } else {
            locTouch = ball.touching(rightWall);
            touchedWall = &rightWall;
        }

        calculate:
        // calculate the direction in which the ball will travel
        if (locTouch != nullptr) {
            // play hit sound
            PlaySound(hit);
            float distanceY = static_cast<int>((touchedWall->location.y + (touchedWall->height / 2)) - locTouch->y);
            //std::cout << distanceY << std::endl;
            float angle = ((distanceY) / (touchedWall->height / 2)); // how close to the center of the wall
            blmv.y = round((speed / 2) * angle);
            //std::cout << blmv.y << std::endl;
            blmv.x = -blmv.x;
        }

        // move the walls down/up
        // multi-key for two-player
        if (IsKeyDown(KEY_S)) {
            leftWall.move(0, move);
        }
        if (IsKeyDown(KEY_W)) {
            leftWall.move(0, -move);
        } 
        if (IsKeyDown(KEY_DOWN)) {
            rightWall.move(0, move);
        }
        if (IsKeyDown(KEY_UP)) {
            rightWall.move(0, -move);
        } 
        if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_Q)) {
            break;
        }

        // move the ball
        bool success = ball.move(blmv);
        if (!success) {
            // someone has scored a point
            if (ball.location.x < screenWidth/2) {
                player2Score++;
            } else {
                player1Score++;
            }
            // play sound
            PlaySound(lose);
            // reset the locations of the balls and move factor of the balls
            ball.location = vector(screenWidth/2, screenHeight/2);
            srand(time(0));
            blmv.x = (rand() / RAND_MAX) > 0.5 ? speed : -speed;
            blmv.y = 0;
            // increase speed
            speed++;
        }

        // begin drawing and ensure the background is black
        BeginDrawing();
        ClearBackground(BLACK);

        // draw title, score and fps
        DrawText((char *)title, screenWidth/2 - 70, 0, 40, RAYWHITE);
        DrawText((char *)(std::to_string(player1Score) + " : " + std::to_string(player2Score)).c_str(), screenWidth/2 - 50, 50, 50, RAYWHITE);
        DrawText((char *)(std::to_string(fps - GetFrameTime()) + " FPS").c_str(), 0, 0, 25, RAYWHITE);

        // draw objects
        for (auto& obj : objectGlob) {
            switch (obj->type) {
                case object::Rectangle:
                DrawRectangle(obj->location.x, obj->location.y, obj->width, obj->height, RAYWHITE);
                break;
                case object::Circle:
                DrawCircle(obj->location.x, obj->location.y, obj->width/2, RAYWHITE);
                break;
            }
        }

        // end drawing
        EndDrawing();
    }

    CloseWindow();

    return 0;
}