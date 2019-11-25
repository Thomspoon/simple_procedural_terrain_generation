// 
// C++ implementation of Ken Perlin's "Improved Noise reference implementation"
// Located here: https://mrl.nyu.edu/~perlin/noise/
//
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace
{
    // Ken Perlin's original permutation table
    static constexpr uint8_t perm[512] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233,
        7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
        190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219,
        203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174,
        20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27,
        166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230,
        220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25,
        63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
        200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173,
        186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118,
        126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
        189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163,
        70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19,
        98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246,
        97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162,
        241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181,
        199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150,
        254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128,
        195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, 131,
        13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69,
        142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75,
        0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74,
        165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229,
        122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
        102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132,
        187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86,
        164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124,
        123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59,
        227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119,
        248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172,
        9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178,
        185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210,
        144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
        107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176,
        115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114,
        67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };
}

// Enable SFINAE to only allow floating point entry-points to functions
class Perlin
{
public:
    template <typename Float, typename = std::enable_if_t<std::is_floating_point_v<Float>>>
    static constexpr Float noise(
        Float x = 0.0,
        Float y = 0.0,
        Float z = 0.0)
    {
        // calculate early to save a calculation
        const auto floor_x = floor(x);
        const auto floor_y = floor(y);
        const auto floor_z = floor(z);

        // unit cube that contains point
        const auto unit_x = static_cast<int>(floor_x) & 255;
        const auto unit_y = static_cast<int>(floor_y) & 255;
        const auto unit_z = static_cast<int>(floor_z) & 255;

        // relative (x, y, z) of point in cube
        x -= floor_x;
        y -= floor_y;
        z -= floor_z;

        // Compute fade curves for each (x, y, z)
        const auto u = fade(x);
        const auto v = fade(y);
        const auto w = fade(z);

        // hash coordinates of the 8 cube coordinates
        const auto a = perm[unit_x] + unit_y;
        const auto aa = perm[a] + unit_z;
        const auto ab = perm[a + 1] + unit_z;
        const auto b = perm[unit_x + 1] + unit_y;
        const auto ba = perm[b] + unit_z;
        const auto bb = perm[b + 1] + unit_z;

        // add blended results form 8 corners of cube
        return lerp(w,
                    lerp(
                        v,
                        lerp(
                            u,
                            grad(
                                perm[aa],
                                x,
                                y,
                                z),
                            grad(
                                perm[ba],
                                x - 1,
                                y,
                                z)),
                        lerp(
                            u,
                            grad(
                                perm[ab],
                                x,
                                y - 1,
                                z),
                            grad(
                                perm[bb],
                                x - 1,
                                y - 1,
                                z))),
                    lerp(
                        v,
                        lerp(
                            u,
                            grad(
                                perm[aa + 1],
                                x,
                                y,
                                z - 1),
                            grad(
                                perm[ba + 1],
                                x - 1,
                                y,
                                z - 1)),
                        lerp(
                            u,
                            grad(
                                perm[ab + 1],
                                x,
                                y - 1,
                                z - 1),
                            grad(
                                perm[bb + 1],
                                x - 1,
                                y - 1,
                                z - 1))));
    }

private:
    template <typename Float, typename = std::enable_if_t<std::is_floating_point_v<Float>>>
    static constexpr Float fade(const Float f)
    {
        return pow(f, 3) * (f * (f * 6 - 15) + 10);
    }

    template <typename Float, typename = std::enable_if_t<std::is_floating_point_v<Float>>>
    static constexpr Float lerp(
        const Float t,
        const Float a,
        const Float b)
    {
        return a + t * (b - a);
    }

    template <typename Float, typename = std::enable_if_t<std::is_floating_point_v<Float>>>
    static constexpr Float grad(
        const int hash,
        const Float x,
        const Float y,
        const Float z)
    {
        // convert low 4 bits of hash code into 12 gradient directions
        const auto h = hash & 15;
        const auto u = h < 8 ? x : y,
                   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};