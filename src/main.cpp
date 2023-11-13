#include "raylib.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "rlImGui.h"

#include <cstdlib>
#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

class Grid {
public:
    enum ResizePlacement {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        CENTER
    };

    static std::pair<int, int> computeOffsets(ResizePlacement spot, int new_width, int new_height, int width, int height) {
        int xoffset, yoffset;
        switch (spot) {
            case CENTER:
                xoffset = (new_width - width) / 2;
                yoffset = (new_height - height) / 2;
                break;
            case BOTTOM_RIGHT:
                xoffset = new_width - width;
                yoffset = new_height - height;
                break;
            case BOTTOM_LEFT:
                xoffset = 0;
                yoffset = new_height - height;
                break;
            case TOP_RIGHT:
                xoffset = new_width - width;
                yoffset = 0;
                break;
            case TOP_LEFT:
            default:
                xoffset = 0;
                yoffset = 0;
                break;
        };
        return std::make_pair(xoffset, yoffset);
    }

    Grid(int w, int h, int ns) : width(w), height(h), pixels(w*h, WHITE) {}

    void clear() {
        std::fill(pixels.begin(), pixels.end(), WHITE);
    }

    void resize(int new_width, int new_height, ResizePlacement spot = TOP_LEFT) {
        std::vector<Color> pixels_new(new_width*new_height, WHITE);

        auto [xoffset, yoffset] = computeOffsets(spot, new_width, new_height, width, height);
        
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int yy = y + yoffset;
                int xx = x + xoffset;
                if (xx < 0 || yy < 0 || xx >= new_width || yy >= new_height)
                    continue;
                pixels_new[yy * new_width + xx] = pixels[y * width + x];
            }
        }

        width = new_width;
        height = new_height;
        pixels = std::move(pixels_new);
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
    int width, height;
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
    
    Ant(Grid& grid, const std::string& p) : pattern(p.size()) {
        setPattern(grid, p);
        recenter(grid);
    }

    void setPattern(Grid& grid, const std::string& p) {
        std::map<char, RelativeDirection> directionMap = {{'L', LEFT}, {'R', RIGHT}, {'U', BACKWARD}, {'N', FORWARD}};
        pattern.clear();
        for (int i = 0; i < p.size(); i++) {
            if (directionMap.count(p[i]) > 0) {
                pattern.push_back(directionMap[p[i]]);
            }
        }
        grid.clear();
        recenter(grid);
    }

    void recenter(const Grid& grid) {
        x = grid.getWidth() / 2;
        y = grid.getHeight() / 2;
        heading = NORTH;
    }

    void iter(Grid& grid) {
        turn(grid);
        flip(grid);
        advance();
        wrap(grid);
    }

    void check(const Grid& grid) const {
        assert (x >= 0);
        assert (x < grid.getWidth());
        assert (y >= 0);
        assert (y < grid.getHeight());
    }

    void turn(const Grid& grid) {
        check(grid);
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
        check(grid);
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

    void offsetPosition(std::pair<int, int> xy) {
        x += xy.first;
        y += xy.second;
    }

private:
    int x, y;
    Direction heading;
    std::vector<RelativeDirection> pattern;
};

class ConfigWindow {
public:
    enum FrameSpeed {
        SLOW,
        MEDIUM,
        FAST,
        MAX
    };

    static inline std::array<int, 4> SpeedValues = {30, 144, 500, 100000};
    static inline std::map<FrameSpeed, std::string> SpeedNames = {{SLOW, "Slow"}, {MEDIUM, "Medium"}, {FAST, "Fast"}, {MAX, "Max"}};

    ConfigWindow(int w, int h) : winWidth(w), winHeight(h), isOpen(true), gridScaleFactor(4), currentPattern("LR"), iterSteps(1) {
        setSpeed(FAST);
    }

    void setSpeed(FrameSpeed speed) {
        fpsLimit = speed;
        SetTargetFPS(SpeedValues[speed]);
    }

    void reset(Grid& grid, Ant& ant) const {
        grid.clear();
        ant.recenter(grid);
    }

    void draw(Grid& grid, Ant& ant) {
        ImGui::Begin("Config", &isOpen, ImGuiWindowFlags_MenuBar);

        ImGui::Text("FPS: %d", GetFPS());

        ImGui::SliderInt("Step Size", &iterSteps, 1, 10000, "%d", ImGuiSliderFlags_Logarithmic);

        if (ImGui::SliderInt("Scale", &gridScaleFactor, 1, 8))
            setGridScaleFactor(grid, ant, gridScaleFactor);

        if (ImGui::SliderInt("Speed", (int*)&fpsLimit, SLOW, MAX, SpeedNames[fpsLimit].c_str()))
            setSpeed(fpsLimit);

        if (ImGui::InputText("Pattern", &currentPattern) && currentPattern.size() > 1)
            ant.setPattern(grid, currentPattern);

        // show ImGui Content
        if (ImGui::Button("Reset")) {
            reset(grid, ant);
        }

        ImGui::SameLine();

        if (ImGui::Button("Step 10k")) {
            for (int i = 0; i < 10000; i++) {
                ant.iter(grid);
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Step 100k")) {
            for (int i = 0; i < 100000; i++) {
                ant.iter(grid);
            }
        }

        ImGui::End();
    }

    void iter(Grid& grid, Ant& ant) {
        for (int i = 0; i < iterSteps; i++)
            ant.iter(grid);
    }

    int getWinWidth() const { return winWidth; }
    int getWinHeight() const { return winHeight; }

    int getGridScaleFactor() const { return gridScaleFactor; }
    void setGridScaleFactor(Grid& grid, Ant& ant, int factor) {
        gridScaleFactor = factor;
        auto offsets = Grid::computeOffsets(Grid::CENTER, winWidth / factor, winHeight / factor, grid.getWidth(), grid.getHeight());
        grid.resize(winWidth / factor, winHeight / factor, Grid::CENTER);
        ant.offsetPosition(offsets);
        ant.wrap(grid);
    }

    std::string getPattern() const { return currentPattern; }

private:
    bool isOpen;

    // Global window size
    int winWidth;
    int winHeight;

    int gridScaleFactor;
    FrameSpeed fpsLimit;
    std::string currentPattern;
    int iterSteps;
};

void UpdateDrawFrame(Grid& grid, Ant& ant, ConfigWindow& config)
{
    // Load pixel data into an image structure and send to GPU via texture
    Image im = {
        .data = (void*)grid.getImageContent(), // sharing host data, no need to deallocate this Image
        .width = grid.getWidth(),
        .height = grid.getHeight(),
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D tex = LoadTextureFromImage(im);

    BeginDrawing();
    {
        ClearBackground(RAYWHITE);

        // Map the texture to a sized rectangle for efficient upscaling
        Rectangle src = {
            .x = 0,
            .y = 0,
            .width = (float) grid.getWidth(),
            .height = (float) grid.getHeight()
        };
        Rectangle dest = {
            .x = 0,
            .y = 0,
            .width = (float) config.getWinWidth(),
            .height = (float) config.getWinHeight()
        };
        DrawTexturePro(tex, src, dest, {0, 0}, 0, WHITE);
        
		rlImGuiBegin();
        {
            config.draw(grid, ant);
        }
		rlImGuiEnd();
    }
    EndDrawing();

    UnloadTexture(tex);
}

int main()
{
    ConfigWindow cfg(1280, 720);

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(cfg.getWinWidth(), cfg.getWinHeight(), "raylib [core] example - basic window");

    SetTraceLogLevel(LOG_NONE);

    rlImGuiSetup(true);

    Grid grid(cfg.getWinWidth() / cfg.getGridScaleFactor(), cfg.getWinHeight() / cfg.getGridScaleFactor(), 2);
    Ant ant(grid, cfg.getPattern());

    unsigned long long int frameid = 0;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame(grid, ant, cfg);
        cfg.iter(grid, ant);
    }

    // De-Initialization
    rlImGuiShutdown();
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

