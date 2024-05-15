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

struct IntColorizer {
  static Color colorize(int x) {
    assert(x >= 0);

    // scale so that different values are more easily distinguishable
    Color selection = colorPalette[x % maxStateCount];
    int times = x / maxStateCount + 1;
    return Color{static_cast<unsigned char>(selection.r / float(times)),
                 static_cast<unsigned char>(selection.g / float(times)),
                 static_cast<unsigned char>(selection.b / float(times)), 255};
  }
};

template<class Grid>
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
        int idx = grid.get(x, y);
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
        grid.set(x, y, (grid.get(x, y) + 1) % pattern.size());
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

    void setPosition(int xx, int yy) {
        x = xx;
        y = yy;
    }

private:
    int x, y;
    Direction heading;
    std::vector<RelativeDirection> pattern;
};

template<class Grid>
struct AntRuntime {
    AntRuntime() : guiIsOpen(false), pattern("LRRRRRLLR"), ant(256, 256, "LRRRRRLLR"), stepsTaken(0), reset(false), iterSteps(1) {}

    void tick(Grid& grid) {
        if (reset) {
            ant = Ant<Grid>(256, 256, pattern);
            grid.clear();
            stepsTaken = 0;
            reset = false;
        }
        for (int i = 0; i < iterSteps; i++) {
            ant.iter(grid, false);
        }
        stepsTaken += iterSteps;
    }

    void drawGui() {
        ImGui::Begin("Config", &guiIsOpen);

        ImGui::Text("FPS: %d", GetFPS());

        ImGui::SameLine();

        ImGui::SliderInt("Speed", &iterSteps, 1, 500000, "%d", ImGuiSliderFlags_Logarithmic);

        if (ImGui::InputText("Pattern", &pattern) && pattern.size() > 1) {
            reset = true;
        }

        if (ImGui::Button("Reset")) {
            reset = true;
        }

        ImGui::Text("Steps Taken: %llu", stepsTaken);

        ImGui::End();
    }

    int iterSteps;
    bool guiIsOpen;
    std::string pattern;
    Ant<Grid> ant;
    unsigned long long int stepsTaken;
    bool reset;
};

int main()
{
    DynamicGridWindow<AntRuntime, int, IntColorizer> cfg(512, 512, "Langton's Ant Simulator", 60);
    cfg.loop();
    return 0;
}
