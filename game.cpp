#include <iostream>
#include <array>
#include <thread>
#include <functional>

#include "raylib.h"
#include "raylib-cpp.hpp"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <expected>
using std::expected;
using std::unexpected;

#include <thread>
#include <chrono>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#ifdef _MSC_VER
#define MSVC_ELSE(X, Y) X
#else
#define MSVC_ELSE(X, Y) Y
#endif

#undef min
#undef max

#ifdef NDEBUG
#define DEBUG_ONLY(x)
#else
#define DEBUG_ONLY(x) {x}
#endif

#include <concepts>
#include <type_traits>


template <typename T>
concept Scalar = std::is_arithmetic_v<T>;

template <Scalar T>
struct Vec2 {
    T x;
    T y;

    Vec2 operator+(const Vec2& o) {
        return Vec2 { this->x + o.x, this->y + o.y };
    }

    Vec2& operator+=(const Vec2& o) {
        this->x += o.x;
        this->y += o.y;
        return *this;
    }

    Vec2 operator-(const Vec2& o) {
        return Vec2 { this->x - o.x, this->y - o.y };
    }

    Vec2& operator-=(const Vec2& o) {
        this->x -= o.x;
        this->y -= o.y;
        return *this;
    }

    template <Scalar S>
    Vec2 operator*(const S& a) {
        return Vec2 { a * x, a * y };
    }

    template <Scalar S>
    Vec2& operator*=(const S& a) {
        x *= a;
        y *= a;
        return *this;
    }

    T operator*(const Vec2<T>& o) {
        return x*o.x + y*o.y;
    }

    template <Scalar S>
    Vec2 operator/(const S& a) {
        return Vec2 { x / a, y / a };
    }

    template <Scalar S>
    Vec2& operator/=(const S& a) {
        x /= a;
        y /= a;
        return *this;
    }

    double length() {
        return std::sqrt(x*x + y*y);
    }

    Vec2& normalize() {
        return *this /= length();
    }

    T determinant(const Vec2& o) {
        return x * o.y - y * o.x;
    }

    void draw(Vec2<T> start, raylib::Color color) {
        DrawLine(start.x, start.y, start.x + x, start.y + y, color);
        DrawRectangle(start.x + x -2, start.y+y -2, 4, 4, color);
    }

    friend std::ostream& operator<<(std::ostream& stream, Vec2& v) {
        stream << "(" << v.x << ", " << v.y << ")";
        return stream;
    }
};

#define CIRCLE_NUM 8
#define CIRCLE_RADIUS 15.0f

template <size_t num>
class Circles {
public:
    std::array<Vec2<double>, num> positions;
    std::array<Vec2<double>, num> accelerations;
    std::array<Vec2<double>, num> velocities;

    Circles(const raylib::Window& window) : positions(), accelerations(), velocities() {
        Vec2<double> firstPosition{ 100, 100 };
        Vec2<double> deltaPosition { (window.GetWidth() - (100 + 100)) / (num - 1), 0 };
        for (size_t i = 0; i < num; i++) {
            positions[i] = firstPosition + deltaPosition * i;
            velocities[i] = Vec2<double> { 0, 0 };
            accelerations[i] = Vec2<double> { 0, 9.81 - 0.2*i };
        }
    }

    friend std::ostream& operator<<(std::ostream& stream, Circles& c) {
        for (size_t i = 0; i < num; i++) {
            stream << "P: " << c.positions[i] << ",\tV: "
                   << c.velocities[i] << ",\tA: " << c.accelerations[i] << "\n";
        }
        return stream;
    }

    template <typename Rep, typename Period>
    void physics(std::chrono::duration<Rep, Period> deltaT) {
        DEBUG_ONLY(std::cout << "Prije: \n" << *this << "\n\n";)
        for (size_t i = 0; i < num; i++) {
            velocities[i] += (accelerations[i] * deltaT.count()) / std::remove_cvref_t<decltype(deltaT)>::period::den;
            positions[i] += (velocities[i] * deltaT.count()) / std::remove_cvref_t<decltype(deltaT)>::period::den;
        }
        DEBUG_ONLY(std::cout << "Poslije: \n" << *this << "\n\n";)
    }
};

class Background {
public:

    Circles<CIRCLE_NUM> circles;

    raylib::Color color;

    Background() = delete;

    Background(const raylib::Window& window, raylib::Color bgColor) : circles(window), color(bgColor) {
    }

    void draw() {
        for (size_t i = 0; i < CIRCLE_NUM; i++) {
            DrawCircleLines(circles.positions[i].x, circles.positions[i].y, CIRCLE_RADIUS, raylib::Color::RayWhite());
            circles.velocities[i].draw(circles.positions[i], raylib::Color::RayWhite());
        }
    }
};

class Line {
public:
    static constexpr std::chrono::seconds ttFall = std::chrono::seconds(3);

    Vec2<double> start;
    Vec2<double> end;

    Vec2<double> v;

    Line(const raylib::Window& window, int width, int height) : start(0, maxDepth(window)+100), end(width, maxDepth(window)+100), v(0, window.GetRenderHeight() / ttFall.count()) {
        v = Vec2<double> { 0, 0 };
    }

    int minDepth(const raylib::Window& window) {
        return 100;
    }

    int maxDepth(const raylib::Window& window) {
        return window.GetHeight() - 100;
    }

    void draw(const raylib::Color color) {
        DrawLine(start.x, start.y, end.x, end.y, color);
    }

    template <typename Rep, typename Period>
    void physics(const raylib::Window& window, const std::chrono::duration<Rep, Period>& deltaT) {
        int maxD = maxDepth(window);
        auto s = v * deltaT.count();
        auto sInM = s / std::remove_cvref_t<decltype(deltaT)>::period::den;
        if (start.y < maxD) {
            start.y += sInM.y;
            if (start.y > maxD) {
                start.y = maxD-500;
            }
        }
        if (end.y < maxD) {
            end.y += sInM.y;
            if (end.y > maxD) {
                end.y = maxD;
            }
        }
    }

    double distance(Vec2<double> point) {
        Vec2<double> pravac { end.x - start.x, end.y - start.y };
        Vec2<double> pPravacTocka { point.x - start.x, point.y - start.y }; //vektor od pocetka pravca do tocke
        double determinant = pPravacTocka.determinant(pravac); //dvostruka povrsina trokuta izmedu pravca i drugog vektora
        return std::abs(determinant) / pravac.length(); //duljina visine trokuta
    }

    void collisions(Circles<CIRCLE_NUM>& circles) {
        for (size_t i = 0; i < CIRCLE_NUM; i++) {
            if (distance(circles.positions[i]) <= CIRCLE_RADIUS) {
                Vec2<double> n { start.y - end.y, end.x - start.x };
                n.normalize();
                circles.velocities[i] -= n * 2 * (circles.velocities[i] * n);
            }
        }
    }
};

class Player {
public:
    
};

class GameThreadPool {
private:
    std::jthread fizika;

public:
    void fizikaLoop(const raylib::Window& window, Line& line, Background& background) {
        std::chrono::high_resolution_clock timer;
        std::chrono::duration lastTime = timer.now().time_since_epoch();
        std::cout << "pocetak: " << lastTime;
        while (true) {
            auto newTime = timer.now().time_since_epoch();
            auto deltaT = (newTime - lastTime);

            line.collisions(background.circles);

            //line.physics(window, deltaT);
            background.circles.physics(deltaT);

            lastTime = newTime;
        }

    }

    GameThreadPool(raylib::Window& window, Line& line, Background& background, const std::function<void()> draw, const raylib::Color bgColor) : fizika([this, &window, &line, &background]() {
        this->fizikaLoop(window, line, background);
    }){
    }
};

class Game {
public:
    Line line;
    Background background;
    raylib::Camera2D camera;

private:
    GameThreadPool gtp;

public:

    Game(raylib::Window& window) : line(window, window.GetWidth(), window.GetHeight()), background(window, raylib::Color { 31, 31, 30, 255 }),
                                   camera({ 0, 0 }, {0,0}, 0, 0), gtp(window, line, background, [this]() { this->draw(); }, background.color) {
    }

    void draw() {
        background.draw();
        line.draw(raylib::Color::RayWhite());
    }
};

#ifdef NDEBUG
int WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] int nShowCmd) {
#else
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
#endif

    SetTraceLogLevel(TraceLogLevel::LOG_INFO);

    raylib::Window window;

    window.SetConfigFlags(ConfigFlags::FLAG_WINDOW_RESIZABLE);
    window.SetConfigFlags(ConfigFlags::FLAG_MSAA_4X_HINT);
    window.SetConfigFlags(ConfigFlags::FLAG_WINDOW_HIGHDPI);

    EnableEventWaiting();

    window.Init(1200, 600, "Slugs");

    window.SetMinSize(800, 400);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
#endif

    Game game {window};

    //--------------------------------------------------------------------------------------


    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        window.BeginDrawing();
        window.ClearBackground(game.background.color);

        game.draw();

        DrawFPS(window.GetWidth() - 150, 20);
        window.EndDrawing();
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    //UnloadGame(); // Unload loaded data (textures, sounds, models...)

    //CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
