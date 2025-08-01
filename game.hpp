#pragma once

class PerlinNoise {
private:
    std::vector<int> permutation;

    std::vector<int>& FisherYatesShuffle(std::vector<int>& permutation);

    int repeat;

    Vec2<double> randomGradient(int ix, int iy);

    //dot prod distance and gradient vectors
    double dotGridGradient(int ix, int iy, double x, double y);

    double interpolate1(double t);

    double interpolate2(double a0, double a1, double w);

    double CPUhelper(double x, double y);

public:
    void CPU(std::vector<RGBA>& pixels, int width, int height);


    void SIMD();

    void GPU();

    std::vector<RGBA> pixels;

private:
    size_t nextPowerOf2(size_t n);

public:
    raylib::Texture2DUnmanaged texture;
    int width, height;

    PerlinNoise(raylib::Window& window);

    void draw();

    ~PerlinNoise();
};

template <typename _Ty>
class QuadTree {
private:
    std::vector<_Ty>& pixels;
    int width;

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

        QTNode();
    };

public:

    std::vector<QTNode> data;

private:
    bool isHomogenous(int x0, int y0, int x1, int y1);

    void recursiveInit(int index, int x0, int y0, int x1, int y1);

    //first is inside second
    inline bool inside(Rectangle first, Rectangle second);

public:
    std::vector<uint16_t> overlappedIndexes;


    QuadTree(std::vector<_Ty>& pixels, const int width, const _Ty& filter);

    QuadTree(PerlinNoise& perlinNoise);

    void overlapRec(const Rectangle& object, const _Ty& color);
};
