#version 430

#define GOL_WIDTH 768

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 1) readonly restrict buffer golLayout {
    uint golBuffer[]; 
};

layout(std430, binding = 2) writeonly restrict buffer golLayout2 {
    uint golBufferDest[];  
};

#define fetchGol(x) ((((x) < 0) || ((x) > GOL_WIDTH) \
    ? (0) \
    : golBuffer[(x)])

#define setGol(x, value) golBufferDest[(x)] = value

void main()
{
    uint neighbourCount = 0;
    uint x = gl_GlobalInvocationID.x;

    neighbourCount += fetchGol(x - 1); 
    neighbourCount += fetchGol(x + 1);

    if (neighbourCount == 2) setGol(x, 1);
    else if (neighbourCount == 1);
    else setGol(x, 0);
}
