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

std::vector<std::function<void()>> permananentDrawables;
struct RGBA {
    uint8_t R, G, B, A;

    bool operator==(const RGBA& other) {
        return R == other.R && G == other.G && B == other.B && A == other.A;
    }
};

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
        double len = length();
        if (len > EPSILON) {
            return *this /= len;
        }
        assert(length() == 1 || length() < EPSILON);
        return *this;
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
    Background() { }

    void draw() {
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

    void collisions(Circles<8>& circles) { //TODO: FIX OR SOMETHING
        for (size_t i = 0; i < 8; i++) {
            if (distance(circles.positions[i]) <= 15.0f) {
                Vec2<double> n { start.y - end.y, end.x - start.x };
                n.normalize();
                circles.velocities[i] -= n * 2 * (circles.velocities[i] * n);

                //to prevent balls getting stuck
                //circles.positions[i] += (circles.velocities[i] * 8000000) / std::chrono::steady_clock::duration::period::den;
                //circles.velocities[i] += (circles.accelerations[i] * 8000000000000) / std::chrono::steady_clock::duration::period::den;
            }
        }
    }
};

template <uint8_t num>
class Players {
public:
    enum Keys : uint8_t {
        w = 0,
        a,
        s,
        d,

        up,
        left,
        down,
        right,

        NUM_OF_REGISTERED_KEYS,
    };

    KeyboardKey fromKey(Keys key) {
        switch (key) {
            case Keys::w:
                return KEY_W;
            case Keys::a:
                return KEY_A;
            case Keys::s:
                return KEY_S;
            case Keys::d:
                return KEY_D;

            case Keys::up:
                return KEY_UP;
            case Keys::left:
                return KEY_LEFT;
            case Keys::down:
                return KEY_DOWN;
            case Keys::right:
                return KEY_RIGHT;

            default:
                exit(11108);
        }
    }

    std::array<Vec2<double>, Keys::NUM_OF_REGISTERED_KEYS> key2velocity = {
        Vec2<double>{ 0.0f,-1.0f },
        Vec2<double>{ -1.0f, 0.0f },
        Vec2<double>{ 0.0f, 1.0f },
        Vec2<double>{ 1.0f, 0.0f },
        Vec2<double> { 0.0f, -1.0f },
        Vec2<double> { -1.0f, 0.0f },
        Vec2<double> { 0.0f, 1.0f },
        Vec2<double> { 1.0f, 0.0f },
    };

    std::array<raylib::Camera2D, num> cameras;
    std::array<raylib::Rectangle, num> bodies; //encapsulates positions
    std::array<Vec2<double>, num> velocities;
    std::array<Vec2<double>, num> accelerations;
    std::array<std::array<bool,Keys::NUM_OF_REGISTERED_KEYS>, num> keyStates;

    Players() : cameras(), bodies({ { 150, 150, 20, 20 }, { 200, 150, 20, 20 } }),
                velocities(), accelerations(), keyStates({ {0} })
    {

    }

    template <typename Rep, typename Period>
    void physics(std::chrono::duration<Rep, Period> deltaT) {
        for (size_t i = 0; i < num; i++) {
            velocities[i] += (accelerations[i] * deltaT.count()) / std::remove_cvref_t<decltype(deltaT)>::period::den;
            Vec2<double> positionsDelta = (velocities[i] * deltaT.count()) / std::remove_cvref_t<decltype(deltaT)>::period::den;

            positionsDelta.normalize(); // * 300; //TODO: tune

            bodies[i].x += positionsDelta.x;
            bodies[i].y += positionsDelta.y;
        }
    }

    inline Vec2<double> centre(uint8_t i) {
        return { bodies[i].x + bodies[i].width / 2, bodies[i].y + bodies[i].height / 2 };
    }

    void draw(uint8_t index, raylib::Color color) {
        bodies[index].Draw(color);
    }
};

template <uint8_t pNum>
class GameThreadPool {
private:
    std::jthread fizika;

    const raylib::Window& window;
    Line& line;
    Players<pNum>& players;

    bool isDown(Players<pNum>::Keys key) { return IsKeyDown(players.fromKey(key));}
    bool wasDown(Players<pNum>::Keys key, uint8_t pIndex) { return players.keyStates[pIndex][key]; }

    void handleMovement(Players<pNum>::Keys key, uint8_t pIndex) {
        if (isDown(key)) {
            if (!wasDown(key, pIndex)) {
                players.velocities[pIndex] += players.key2velocity[key];

                players.keyStates[pIndex][key] = true;
            }
        } else if (wasDown(key, pIndex)) {
            players.velocities[pIndex] -= players.key2velocity[key];

            players.keyStates[pIndex][key] = false;
        }
    }

    void handleInput() {
        //wasd for player 0
        handleMovement(Players<pNum>::Keys::w, 0);
        handleMovement(Players<pNum>::Keys::a, 0);
        handleMovement(Players<pNum>::Keys::s, 0);
        handleMovement(Players<pNum>::Keys::d, 0);

        //arrows for player 1
        handleMovement(Players<pNum>::Keys::up, 1);
        handleMovement(Players<pNum>::Keys::left, 1);
        handleMovement(Players<pNum>::Keys::down, 1);
        handleMovement(Players<pNum>::Keys::right, 1);
    }

public:
    void fizikaLoop() {
        using namespace std::literals::chrono_literals;

        std::chrono::high_resolution_clock timer;
        std::chrono::duration lastTime = timer.now().time_since_epoch();
        std::cout << "pocetak: " << lastTime;

        const std::chrono::nanoseconds fixedDT { std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / 60 };
        std::chrono::nanoseconds accumulator { 0 };

        while (true) {
            handleInput();

            auto newTime = timer.now().time_since_epoch();
            auto frameDeltaT = (newTime - lastTime);
            lastTime = newTime;

            accumulator += frameDeltaT;
            //physics with fixed time step
            while (accumulator > fixedDT) {

                //line.collisions(background.circles);

                //line.physics(window, deltaT);
                //background.circles.physics(deltaT);
                players.physics(fixedDT);

                accumulator -= fixedDT;
            }

            //std::cout << players.velocities[0] << "  " << (deltaT.count()/100000)
            //<< "\n";

            
        }

    }

    GameThreadPool(raylib::Window& window, Line& line, Players<2>& players) : fizika([this]() {
        this->fizikaLoop();
    }), window(window), line(line), players(players)
    {

    }
};

template <typename _Ty>
class QuadTree {
private:
    const std::vector<_Ty>& pixels;
    const int width;

    enum class NodeType : char {
        empty,
        occupied,
        mixed,
    };

    class QTNode {
    public:
        Rectangle position;
        _Ty color;
        NodeType type;

        QTNode() : position(), color(), type(NodeType::empty) { }
    };

public:
    std::vector<QTNode> data;

private:
    bool isHomogenous(int x0, int y0, int x1, int y1) {
        _Ty first = pixels[y0 * width + x0];
        for (int j = y0; j < y1; j++) {
            for (int i = x0; i < x1; i++) {
                if (pixels[j * width + i] != first) {
                    return false;
                }
            }
        }

        return true;
    }

    void recursiveInit(int index, int x0, int y0, int x1, int y1) {
        if (!isHomogenous(x0, y0, x1, y1)) {
            data[index].type = NodeType::mixed;
            permananentDrawables.emplace_back([=]() { DrawLine((x0 + x1) / 2, y0, (x0 + x1) / 2, y1, RED); });
            permananentDrawables.emplace_back([=]() { DrawLine(x0, (y0 + y1) / 2, x1, (y0 + y1) / 2, RED); });

            if (index * 4 + 3 < data.size()) {
                recursiveInit(index * 4 + 0, x0, y0, (x0 + x1) / 2, (y0 + y1) / 2);
                recursiveInit(index * 4 + 1, (x0 + x1) / 2, y0, x1, (y0 + y1) / 2);
                recursiveInit(index * 4 + 2, x0, (y0 + y1) / 2, (x0 + x1) / 2, y1);
                recursiveInit(index * 4 + 3, (x0 + x1) / 2, (y0 + y1) / 2, x1, y1);
            }
        } else {
            data[index].color = pixels[y0 * width + x0];
        }
    }

    //first is inside second
    inline bool inside(Rectangle first, Rectangle second) {
        return (second.x < first.x) && (second.x + second.width > first.x + first.width) &&
               (second.y < first.y) && (second.y + second.height < first.y + first.height);
    }

public:
    std::vector<uint16_t> overlappedIndexes;

    QuadTree(const std::vector<_Ty>& pixels, const int width, const _Ty& filter) : pixels(pixels), width(width), data() {
        data.resize((1 - width * width) / (1 - 4)); // (1 - 4^(log2(width))) / (1 - 4)
        assert(data.capacity() == (1 - width * width) / (1 - 4) && data.capacity() == data.size());
        overlappedIndexes.reserve(4);
        recursiveInit(0, 0, 0, width, width);
    }


    //opcije: vratiti boju, vratiti referencu na objekt
    //          dal vratiti 
    void overlapRec(const Rectangle& object, const _Ty& color) {
        overlappedIndexes.clear();
        //we dont check for collisions with the root node


        auto overlapHelper = [](const Rectangle& object, _Ty matchColor, uint16_t dataIndex) {
            for (int i = 1; i < 5; i++) { // children are   index * 4 + 1..5
                if (CheckCollisionRecs(data[dataIndex + i].position, object)) {
                    if (!inside(data[dataIndex + i], object)) { //data entry not in object, otherwise ignore
                        overlappedIndexes.emplace_back(dataIndex + i); //we store indexes and not references to objects so they don't get invalidated on resize
                        overlapHelper(object, matchColor, dataIndex * 4);
                    }
                }
            }
        };

        overlapHelper(object, color, 0);
    }
};

class PerlinNoise {
private:
    std::vector<int> permutation;

    std::vector<int>& FisherYatesShuffle(std::vector<int>& permutation) {
        std::chrono::high_resolution_clock clock;
        std::mt19937_64 rEngine { static_cast<uint64_t>(clock.now().time_since_epoch().count()) };
        for (size_t i = permutation.size() - 1; i > 0; i--) {
            std::swap(permutation[rEngine() % i], permutation[i]);
        }

        //DEBUG_ONLY(for (size_t i = 0; i < permutation.size(); i++) {
        //    std::cout << permutation[i] << "\n";
        //})

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
    void CPU(std::vector<RGBA>& pixels, int width, int height) {
        const int GRID_SIZE = (const int)width/(1200/200); // for width 1200 : 200
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int index = (y * width + x);

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
                uint8_t color = (uint8_t)(((val + 1.0f) * 0.5f) * 255);
                pixels[index] = { color, color, color, 255 };
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
    std::vector<RGBA> pixels;

    size_t nextPowerOf2(size_t n) {
        if (n == 0) return 1;
        --n; // Decrement n to handle cases where n is already a power of 2
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        if constexpr (sizeof(size_t) > 4) {
            n |= n >> 32; // For 64-bit size_t
        }
        return n + 1;
    }

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

        //
        //// to get 2^n x 2^n to build a QuadTree
        //
        
        width = height = nextPowerOf2(std::max(width, height));

        pixels.resize(height * width);
        assert(pixels.size() == height * width);

        //
        //// change to CPU simd or GPU later when implemented
        //
        CPU(pixels, width, height);
        assert(pixels.size() == height * width);

        //raylib::Image gives black image/texture!!
        Image image = { 0 };
        image.height = height;
        image.data = pixels.data();
        image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        image.mipmaps = 1;
        image.width = width;
        
        texture = raylib::Texture2DUnmanaged { image };

        QuadTree<decltype(pixels)::value_type> quad { pixels, width, RGBA {255, 255, 255, 255}};
        
    }

    void draw() {
        texture.Draw();
    }

    ~PerlinNoise() {
        texture.Unload();
    }
};

class Game {
private:
    static const uint8_t numPlayers = 2;

public:
    Line line;
    Players<numPlayers> players;

private:
    GameThreadPool<numPlayers> gtp;
    PerlinNoise perlinNoise;

public:

    Game(raylib::Window& window) : line(window, window.GetWidth(), window.GetHeight()), players(),
                                   gtp(window, line, players), perlinNoise(window)
    {

    }

    void draw() {
        perlinNoise.draw();

        //background.draw();
        //line.draw(raylib::Color::RayWhite());

        for (uint8_t i = 0; i < numPlayers; i++) {
            players.draw(i, raylib::Color::RayWhite());
            (players.velocities[i] * 30).draw(players.centre(i), raylib::Color::Green());

            //std::cout << (Vec2<double> { 1, 0 }).length() - (Vec2<double> { 1, 1 }).normalize().length() << "\n";
        }
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

    window.Init(1200, 600, "Slugs"); //1200 600

    //window.SetMinSize(800, 400);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(1000);
#endif

    Game game {window};

    //--------------------------------------------------------------------------------------

    int FPScounter = 60;
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        window.BeginDrawing();
        //window.ClearBackground(game.background.color);

        game.draw();

        /*
        for (int i = 0; i < FPScounter / 60; i++) {
            permananentDrawables[i]();
        }*/

        for (auto& i : permananentDrawables) {
            i();
        }

        FPScounter++;

        DrawFPS(window.GetWidth() - 150, 20);
        window.EndDrawing();
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    //UnloadGame(); // Unload loaded data (textures, sounds, models...)

    CloseWindow(); // Close window and OpenGL context
    exit(0);
    //--------------------------------------------------------------------------------------

    return 0;
}

//napraviti da je perlin noise random
//napraviti da je fullscreen
