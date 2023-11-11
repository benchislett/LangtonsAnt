#include "raylib.h"

#include <cstdlib>
#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <iostream>

class Grid {
public:
    Grid(int w, int h, int ns) : width(w), height(h), numStates(ns), pixels(w*h, WHITE) {
        assert (numStates < maxStateCount);
    }

    int getState(int x, int y) const { return stateFromColor(pixels[y * width + x]); }
    void setState(int x, int y, int newState) { pixels[y * width + x] = stateMap[newState]; }

    const Color* getImageContent() const { return pixels.data(); }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    constexpr static int maxStateCount = 16;
    constexpr static std::array<Color, maxStateCount> stateMap = {(Color){255, 255, 255, 255},(Color){93, 39, 93, 255},(Color){177, 62, 83, 255},(Color){239, 125, 87, 255},(Color){255, 205, 117, 255},(Color){167, 240, 112, 255},(Color){56, 183, 100, 255},(Color){37, 113, 121, 255},(Color){41, 54, 111, 255},(Color){59, 93, 201, 255},(Color){65, 166, 249, 255},(Color){115, 239, 247, 255},(Color){148, 176, 194, 255},(Color){86, 108, 134, 255},(Color){51, 60, 87, 255},(Color){26, 28, 44, 255}};
    static bool colorEquals(const Color& a, const Color& b) {
        return a.a == b.a && a.r == b.r && a.b == b.b && a.g == b.g;
    }

    int stateFromColor(const Color& c) const {
        for (int i = 0; i < maxStateCount; i++) {
            if (colorEquals(c, stateMap[i]))
                return i;
        }
        return -1;
    }

    std::vector<Color> pixels;
    int width, height, numStates;
};

class Ant {
public:
    enum Direction {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };

    enum RelativeDirection {
        FORWARD,
        RIGHT,
        BACKWARD,
        LEFT
    };
    
    Ant(int pos_x, int pos_y, const std::string& p = "LR", Direction h = NORTH) : x(pos_x), y(pos_y), heading(h), pattern(p.size()) {
        std::map<char, RelativeDirection> directionMap = {{'L', LEFT}, {'R', RIGHT}};
        for (int i = 0; i < p.size(); i++) {
            pattern[i] = directionMap[p[i]];
        }
    }

    void iter(Grid& grid) {
        turn(grid);
        flip(grid);
        advance();
        wrap(grid);
    }

    void turn(const Grid& grid) {
        RelativeDirection rel = pattern[grid.getState(x, y)];
        switch (rel) {
            case RIGHT:
                heading = (Direction)((heading + 1) % 4);
                break;
            case BACKWARD:
                heading = (Direction)((heading + 2) % 4);
                break;
            case LEFT:
                heading = (Direction)((heading + 3) % 4);
                break;
            case FORWARD:
            default:
                break;
        }
    }

    void flip(Grid& grid) const {
        grid.setState(x, y, (grid.getState(x, y) + 1) % pattern.size());
    }

    void advance() {
        switch (heading) {
            case NORTH:
                y--;
                break;
            case EAST:
                x++;
                break;
            case SOUTH:
                y++;
                break;
            case WEST:
                x--;
                break;
        }
    }

    void wrap(const Grid& grid) {
        x = (x + grid.getWidth()) % grid.getWidth();
        y = (y + grid.getHeight()) % grid.getHeight();
    }

private:
    int x, y;
    Direction heading;
    std::vector<RelativeDirection> pattern;
};

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
constexpr int winWidth = 800;
constexpr int winHeight = 400;

constexpr int gridWidth = winWidth / 4;
constexpr int gridHeight = winHeight / 4;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(Grid&, Ant&);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Entry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(winWidth, winHeight, "raylib [core] example - basic window");

    SetTraceLogLevel(LOG_NONE);

    Grid grid(gridWidth, gridHeight, 2);
    Ant ant(gridWidth / 2, gridHeight / 2);

    // SetTargetFPS(500);
    //--------------------------------------------------------------------------------------

    unsigned long long int frameid = 0;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame(grid, ant);
        ant.iter(grid);
        frameid++;
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(Grid& grid, Ant& ant)
{

    // // Load pixels data into an image structure and create texture
    Image im = {
        .data = (void*)grid.getImageContent(),             // We can assign pixels directly to data
        .width = gridWidth,
        .height = gridHeight,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    Texture2D tex = LoadTextureFromImage(im);

    int fps = GetFPS();

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);
        Rectangle src = {
            .width = gridWidth,
            .height = gridHeight,
            .x = 0,
            .y = 0
        };
        Rectangle dest = {
            .width = winWidth,
            .height = winHeight,
            .x = 0,
            .y = 0
        };
        DrawTexturePro(tex, src, dest, {0, 0}, 0, WHITE);
        
        DrawText(std::to_string(fps).c_str(), 0, 0, 14, RED);

    EndDrawing();

    UnloadTexture(tex);
    //----------------------------------------------------------------------------------

}
