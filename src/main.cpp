#include "window_base.h"

#include <cstdlib>
#include <cassert>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <numeric>

constexpr static int maxStateCount = 16;
constexpr static std::array<Color, maxStateCount> colorPalette = {(Color){255, 255, 255, 255},(Color){93, 39, 93, 255},(Color){177, 62, 83, 255},(Color){239, 125, 87, 255},(Color){255, 205, 117, 255},(Color){167, 240, 112, 255},(Color){56, 183, 100, 255},(Color){37, 113, 121, 255},(Color){41, 54, 111, 255},(Color){59, 93, 201, 255},(Color){65, 166, 249, 255},(Color){115, 239, 247, 255},(Color){148, 176, 194, 255},(Color){86, 108, 134, 255},(Color){51, 60, 87, 255},(Color){26, 28, 44, 255}};
constexpr static int TileWidthBits = 6;
constexpr static int TileHeightBits = 6;
constexpr static int TileWidth = 1 << TileWidthBits;
constexpr static int TileHeight = 1 << TileHeightBits;

class Tile {
public:
    Tile() : values{} {
        assert (std::accumulate(values.begin(), values.end(), 0) == 0);
    }

    int getValue(int x, int y) {
        auto [byteAddr, parity] = lookup(x, y);
        return parity ? (*byteAddr >> 4) : (*byteAddr & 0xf);
    }

    void setValue(int x, int y, int quantity) {
        assert (quantity < 16);
        auto [byteAddr, parity] = lookup(x, y);
        unsigned char byte = *byteAddr;
        if (parity) {
            *byteAddr = ((quantity << 4) & (0xf0)) | (byte & 0xf);
        } else {
            *byteAddr = (byte & (0xf0)) | (quantity & 0xf);
        }
    }

private:
    std::array<unsigned char, TileWidth*TileHeight/2> values;

    std::pair<unsigned char*, bool> lookup(int x, int y) {
        assert (x >= 0);
        assert (x < TileWidth);
        assert (y >= 0);
        assert (y < TileHeight);

        unsigned char *byte = &values[(y / 2) * TileWidth + x];
        bool parity = (y % 2);
        return std::make_pair(byte, parity);
    }
};

class Grid {
public:
    Grid() {}

    int getState(int x, int y) {
        auto [tileX, tileY, tileXOffset, tileYOffset] = indexDecomp(x, y);
        return refTile(tileX, tileY)->getValue(tileXOffset, tileYOffset);
    }

    void setState(int x, int y, int newState) { 
        auto [tileX, tileY, tileXOffset, tileYOffset] = indexDecomp(x, y);
        refTile(tileX, tileY)->setValue(tileXOffset, tileYOffset, newState);
    }

    void clear() {
        tileMap.clear();
    }

private:
    std::tuple<int, int, int, int> indexDecomp(int x, int y) {
        int tileX = x >> 6;
        int tileY = y >> 6;
        int tileXOffset = (x >= 0) ? (x % TileWidth) : -(x % TileWidth);
        int tileYOffset = (y >= 0) ? (y % TileWidth) : -(y % TileWidth);
        return std::make_tuple(tileX, tileY, tileXOffset, tileYOffset);
    }

    Tile* refTile(int tileX, int tileY) {
        auto key = std::make_pair(tileX, tileY);
        if (tileMap.count(key) == 0) {
            tileMap[key] = Tile();
        }
        return &tileMap[key];
    }

    std::map<std::pair<int, int>, Tile> tileMap;
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
    
    Ant(int xx, int yy, const std::string& p, Direction h = NORTH) : x(xx), y(yy), heading(h), pattern(p.size()) {
        setPattern(p);
    }

    void setPattern(const std::string& p) {
        std::map<char, RelativeDirection> directionMap = {{'L', LEFT}, {'R', RIGHT}, {'U', BACKWARD}, {'N', FORWARD}};
        pattern.clear();
        for (int i = 0; i < p.size(); i++) {
            if (directionMap.count(p[i]) > 0) {
                pattern.push_back(directionMap[p[i]]);
            }
        }
    }

    void iter(Grid& grid, bool should_wrap, int wrap_x_min=0, int wrap_x_width=0, int wrap_y_min=0, int wrap_y_width=0) {
        turn(grid);
        flip(grid);
        advance();

        if (should_wrap) {
            wrap(wrap_x_min, wrap_x_width, wrap_y_min, wrap_y_width);
        }
    }

    void wrap(int wrap_x_min, int wrap_x_width, int wrap_y_min, int wrap_y_width) {
        if (x < wrap_x_min) {
            x += wrap_x_width;
        } else if (x >= wrap_x_min + wrap_x_width) {
            x -= wrap_x_width;
        }

        if (y < wrap_y_min) {
            y += wrap_y_width;
        } else if (y >= wrap_y_min + wrap_y_width) {
            y -= wrap_y_width;
        }
    }

    void turn(Grid& grid) {
        int idx = grid.getState(x, y);
        assert (idx >= 0);
        assert (idx < pattern.size());
        RelativeDirection rel = pattern[idx];
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

    int getX() const { return x; }
    int getY() const { return y; }

    void offsetPosition(std::pair<int, int> xy) {
        x += xy.first;
        y += xy.second;
    }

private:
    int x, y;
    Direction heading;
    std::vector<RelativeDirection> pattern;
};

class ConfigWindow : public WindowManagerBase {
public:
    ConfigWindow(int w, int h, const std::string& winTitle = "Langton's Ant Simulator", int fps = 60) : WindowManagerBase(w, h, winTitle, fps), grid(), ant(0, 0, "LR"), xOffset(0), yOffset(0), currentPattern("LR"), iterSteps(1), hostImageBuffer{}, wrap(false), stepsTaken(0) {
        setZoom(1.f);
        im = {
            .data = (void*)hostImageBuffer.data(), // sharing host data, no need to deallocate this Image
            .width = getCameraWidth()+1,
            .height = getCameraHeight()+1,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };

        Camera2D camera = {
            .offset = {w / 2.f, h / 2.f},
            .rotation = 0.f,
            .target = {0.f, 0.f},
            .zoom = 1.f
        };

        prevMousePos = GetMousePosition();
    }

    void reset() {
        grid.clear();
        ant = Ant(0, 0, getPattern());
        stepsTaken = 0;
    }

    void drawImGuiImpl() override {
        ImGui::Begin("Config", &guiIsOpen);

        if (ImGui::Button("Locate")) {
            xOffset = ant.getX();
            yOffset = ant.getY();
        }

        ImGui::SameLine();

        ImGui::Text("FPS: %d", GetFPS());

        ImGui::SameLine();

        ImGui::Checkbox("Wrap", &wrap);

        ImGui::SliderInt("Speed", &iterSteps, 1, 10000, "%d", ImGuiSliderFlags_Logarithmic);

        if (IsKeyPressed(KEY_EQUAL) && IsKeyDown(KEY_LEFT_CONTROL)) {
            setZoom(zoom + 1.f);
        } else if (IsKeyPressed(KEY_MINUS) && IsKeyDown(KEY_LEFT_CONTROL)) {
            if (zoom > 1.f)
                setZoom(zoom - 1.f);
        }

        float diff = GetMouseWheelMove();
        if (fabsf(diff) > 1e-3) {
            setZoom(std::max(1.f, zoom + diff));
        }

        Vector2 thisPos = GetMousePosition();

        if (IsMouseButtonDown(0) && !ImGui::IsWindowFocused()) {
            float dx = thisPos.x - prevMousePos.x;
            float dy = thisPos.y - prevMousePos.y;
            xOffset -= dx / zoom;
            yOffset -= dy / zoom;
        }
        prevMousePos = thisPos;

        if (ImGui::InputText("Pattern", &currentPattern) && currentPattern.size() > 1) {
            reset();
        }

        // show ImGui Content
        if (ImGui::Button("Reset")) {
            reset();
        }

        ImGui::SameLine();

        if (ImGui::Button("Step 10k")) {
            for (int i = 0; i < 10000; i++) {
                antStep();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Step 100k")) {
            for (int i = 0; i < 100000; i++) {
                antStep();
            }
        }

        ImGui::Text("Steps Taken: %llu", stepsTaken);

        ImGui::End();
    }

    void antStep() {
        ant.iter(grid, wrap, screenXOffset(), getCameraWidth(), screenYOffset(), getCameraHeight());
        stepsTaken++;
    }

    void loopImpl() override {
        for (int i = 0; i < iterSteps; i++) {
            antStep();
        }
    }

    float getZoom() const { return zoom; }
    void setZoom(float factor) {
        zoom = factor;
        hostImageBuffer.clear();
        hostImageBuffer.resize((getCameraWidth()+1)*(getCameraHeight()+1), WHITE);
        im.data = (void*)hostImageBuffer.data();
        im.width = getCameraWidth()+1;
        im.height = getCameraHeight()+1;
    }

    int getCameraWidth() const { return (int) (getWinWidth() / zoom); }
    int getCameraHeight() const { return (int) (getWinHeight() / zoom); }

    int screenXOffset() const { return -getWinWidth() / zoom / 2.f + xOffset; }
    int screenYOffset() const { return -getWinHeight() / zoom / 2.f + yOffset; }

    std::string getPattern() const { return currentPattern; }

    void preDrawImpl() override {
        for (int y = 0; y < getCameraHeight() + 1; y++) {
            for (int x = 0; x < getCameraWidth() + 1; x++) {
                hostImageBuffer[y * (getCameraWidth()+1) + x] = colorPalette[grid.getState(x + screenXOffset(), y + screenYOffset())];
            }
        }

        tex = LoadTextureFromImage(im);
    }

    void drawImpl() override {
        Rectangle src = {
            .x = 0,
            .y = 0,
            .width = (float) getWinWidth() / zoom,
            .height = (float) getWinHeight() / zoom
        };
        Rectangle dest = {
            .x = 0,
            .y = 0,
            .width = (float) getWinWidth(),
            .height = (float) getWinHeight()
        };

        DrawTexturePro(tex, src, dest, {0, 0}, 0, WHITE);
    }

    void postDrawImpl() override {
        UnloadTexture(tex);
    }

private:
    bool guiIsOpen;

    Vector2 prevMousePos;

    Image im;
    Texture2D tex;

    Camera2D camera;

    std::vector<Color> hostImageBuffer;

    Grid grid;
    Ant ant;

    bool wrap;

    float zoom;
    float xOffset, yOffset;

    std::string currentPattern;
    int iterSteps;

    unsigned long long int stepsTaken;
};

int main()
{
    ConfigWindow cfg(1280, 720);
    cfg.loop();
    return 0;
}
