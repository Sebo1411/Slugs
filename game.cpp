#include <iostream>
#include <array>
#include <thread>
#include <functional>
#include <random>
#include <cassert>
#include <cmath>

#include "raylib.h"
#include "raylib-cpp.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include "windows.h"
#endif


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
        //DEBUG_ONLY(std::cout << "Prije: \n" << *this << "\n\n";)
        for (size_t i = 0; i < num; i++) {
            velocities[i] += (accelerations[i] * deltaT.count()) / std::remove_cvref_t<decltype(deltaT)>::period::den;
            positions[i] += (velocities[i] * deltaT.count()) / std::remove_cvref_t<decltype(deltaT)>::period::den;
        }
        //DEBUG_ONLY(std::cout << "Poslije: \n" << *this << "\n\n";)
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

                //to prevent balls getting stuck
                circles.positions[i] += (circles.velocities[i] * 10) / std::chrono::steady_clock::duration::period::den;
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

class PerlinNoise {
private:
    std::vector<int> permutation;

    std::vector<int>& FisherYatesShuffle(std::vector<int>& permutation) {
        std::chrono::high_resolution_clock clock;
        std::ranlux48 rEngine { static_cast<uint64_t>(clock.now().time_since_epoch().count()) };
        for (size_t i = permutation.size() - 1; i > 0; i--) {
            std::swap(permutation[rEngine() % i], permutation[i]);
        }

        DEBUG_ONLY(for (size_t i = 0; i < permutation.size(); i++) {
            std::cout << permutation[i] << "\n";
        })

        return permutation;
    }

    int repeat = 0;

    Vec2<double> randomGradient(int ix, int iy) {
        // No precomputed gradients mean this works for any number of grid coordinates
        const unsigned w = 8 * sizeof(unsigned);
        const unsigned s = w / 2;
        unsigned a = ix, b = iy;
        a *= 3284157443;

        b ^= a << s | a >> w - s;
        b *= 1911520717;

        a ^= b << s | b >> w - s;
        a *= 2048419325;
        double random = a * (PI / ~(~0u >> 1)); // in [0, 2*Pi]

        return Vec2<double> {
            std::sin(random), std::cos(random)
        };
    }

    //dot prod distance and gradient vectors
    double dotGridGradient(int ix, int iy, double x, double y) {
        //gradient from integer coordinates: pseudo-random, deterministic, well distibuted
        Vec2<double> gradient = randomGradient(ix, iy);

        //distance vector
        double dx = x - (double)ix;
        double dy = y - (double)iy;

        //dot product
        return dx * gradient.x + dy * gradient.y;
    }

    double interpolate1(double t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    double interpolate2(double a0, double a1, double w) {
        return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
    }

    double CPUhelper(double x, double y) {
        //grid cell corners
        int x0 = (int)x;
        int y0 = (int)y;
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        //interpolation weights
        double sx = x - (double)x0;
        double sy = y - (double)y0;

        //compute and interpolate top 2 corners
        double n0 = dotGridGradient(x0, y0, x, y);
        double n1 = dotGridGradient(x1, y0, x, y);
        double ix0 = interpolate2(n0, n1, sx);

        //compute and interpolate bottom 2 corners
        n0 = dotGridGradient(x0, y1, x, y);
        n1 = dotGridGradient(x1, y1, x, y);
        double ix1 = interpolate2(n0, n1, sx);

        //interpolate between the two previously interpolated values, now in y
        double value = interpolate2(ix0, ix1, sy);

        return value;
    }

public:
    void CPU(std::vector<uint8_t>& pixels, int width, int height) {
        const int GRID_SIZE = 200;
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int index = (y * width + x) * 4;

                double val = 0;

                double freq = 1;
                double amp = 1;

                //number of overlays, change range for simplicity
                for (int i = 0; i < 2; i++) {
                    val += CPUhelper(x * freq / GRID_SIZE, y * freq / GRID_SIZE) * amp;

                    freq *= 2;
                    amp /= 2;
                }

                //contrast
                val *= 1.2;

                //clipping
                if (val > 1.0f) {
                    val = 1.0f;
                } else if (val < -1.0f) {
                    val = -1.0f;
                }

                //  // // this program specific \\ \\ \\

                if (val > 0.0f) {
                    val = 1.0f;
                } else {
                    val = -1.0f;
                }
                
                //convert 1 to -1 into 255 to 0
                int color = (int)(((val + 1.0f) * 0.5f) * 255);
                pixels[index] = color;
                pixels[index + 1] = color;
                pixels[index + 2] = color;
                pixels[index + 3] = 255;
            }
        }
    }


    void SIMD() { }

    #define PERLIN_WIDTH 0
    void GPU() {
        std::string perlinCode { raylib::LoadFileText("perlinNoise.glsl") };
        unsigned int perlinShader = rlCompileShader(perlinCode.c_str(), RL_COMPUTE_SHADER);
        unsigned int perlinProgram = rlLoadComputeShaderProgram(perlinShader);


        unsigned int ssboA = rlLoadShaderBuffer(PERLIN_WIDTH * sizeof(unsigned int), nullptr, RL_DYNAMIC_COPY);
        unsigned int ssboB = rlLoadShaderBuffer(PERLIN_WIDTH * sizeof(unsigned int), nullptr, RL_DYNAMIC_COPY);

        //... itd
    }

private:
    std::vector<uint8_t> pixels;

public:

    raylib::Texture2DUnmanaged texture;

    PerlinNoise(raylib::Window& window) : permutation(), texture(){
        permutation.reserve(512);
        for (size_t i = 0; i < 256; i++) {
            permutation.emplace_back(i);
        }
        assert(permutation.capacity() == 512);

        FisherYatesShuffle(permutation);

        for (size_t i = 0; i < 256; i++) {
            permutation.emplace_back(permutation[i]);
        }
        assert(permutation.size() == permutation.capacity() && permutation.capacity() == 512);

        int width = window.GetWidth();
        int height = window.GetHeight();

        pixels.resize(height * width * 4);
        assert(pixels.size() == height * width * 4);

        //
        //// change to CPU simd or GPU later when implemented
        //
        CPU(pixels, width, height);
        assert(pixels.size() == height * width * 4);

        //raylib::Image gives black image/texture!!
        Image image = { 0 };
        image.height = height;
        image.data = pixels.data();
        image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        image.mipmaps = 1;
        image.width = width;
        
        texture = raylib::Texture2DUnmanaged { image };
    }

    void draw() {
        texture.Draw();
    }

    ~PerlinNoise() {
        texture.Unload();
    }
};

class Game {
public:
    Line line;
    Background background;
    raylib::Camera2D camera;

private:
    GameThreadPool gtp;
    PerlinNoise perlinNoise;

public:

    Game(raylib::Window& window) : line(window, window.GetWidth(), window.GetHeight()), background(window, raylib::Color { 31, 31, 30, 255 }),
                                   camera({ 0, 0 }, {0,0}, 0, 0), gtp(window, line, background, [this]() { this->draw(); }, background.color),
                                   perlinNoise(window)
    {

    }

    void draw() {
        perlinNoise.draw();
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
