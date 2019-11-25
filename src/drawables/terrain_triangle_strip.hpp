#pragma once

#include <limits>
#include <random>
#include <tuple>

#include "../drawable.hpp"
#include "../perlin.hpp"

namespace {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
    };

    using VertexData = std::vector<Vertex>;
    using TerrainData = std::tuple<VertexData, Indices, unsigned int>;
}

class TerrainTriangleStrip : public Drawable<TerrainTriangleStrip> {
public:
    explicit TerrainTriangleStrip(
        VertexArrayObject&& t_vao, 
        VertexBufferObject&& t_vbo, 
        VertexBufferObject&& t_ebo,
        unsigned int t_draw_count,
        Indices&& t_indices,
        unsigned int t_grid_size
    ) : Drawable(std::move(t_vao)), 
        vbo(std::move(t_vbo)), 
        ebo(std::move(t_ebo)),
        draw_count(t_draw_count),
        indices(std::move(t_indices)),
        grid_size(t_grid_size)
    {
    }

    static std::shared_ptr<TerrainTriangleStrip> create_impl(const unsigned int grid_size) {
        auto terrain_vao = VertexArrayObject();
        auto terrain_vbo = VertexBufferObject(VertexBufferType::ARRAY);
        auto terrain_ebo = VertexBufferObject(VertexBufferType::ELEMENT);

        auto [terrain_attributes, indices, draw_count] = generate_terrain(grid_size);
        terrain_vao.bind();

        terrain_vbo.bind();
        terrain_vbo.send_data(terrain_attributes, VertexDrawType::STATIC);

        terrain_vbo.enable_attribute_pointer(0, 3, VertexDataType::FLOAT, 9, 0);
        terrain_vbo.enable_attribute_pointer(1, 3, VertexDataType::FLOAT, 9, 3);
        terrain_vbo.enable_attribute_pointer(2, 3, VertexDataType::FLOAT, 9, 6);

        terrain_ebo.bind();
        terrain_ebo.send_data(indices, VertexDrawType::STATIC);

        terrain_vbo.unbind();
        terrain_vao.unbind();

        return std::make_shared<TerrainTriangleStrip>(
            std::move(terrain_vao), 
            std::move(terrain_vbo), 
            std::move(terrain_ebo),
            draw_count, 
            std::move(indices),
            grid_size
        );
    }

    DrawType draw_impl() {
        vao.bind();
        auto draw_type = DrawElements {
            VertexPrimitive::TRIANGLE_STRIP,
            draw_count,
            VertexDataType::UNSIGNED_INT,
            indices
        };

        return DrawType(draw_type);
    }

private:
    static TerrainData generate_terrain(const unsigned int grid_size) {
        auto linear_grid_size = grid_size * grid_size;

        auto height_map = generate_height_map(grid_size, 0xDEADBEEF, 25.0, 5, 0.5, 2.0, glm::vec2(0.0, 0.0));

        VertexData terrain_attributes;
        terrain_attributes.reserve(linear_grid_size);
        auto index = 0;
        for(auto x = 0; x < grid_size; x++) {
            for(auto z = 0; z < grid_size; z++) {
                auto color = glm::vec3(1.0f, 1.0f, 1.0f);
                auto height = height_map[index];
                auto heights = std::vector{0.3, 0.4, 0.45, 0.55, 0.6, 0.7, 0.9, 1.0};
                auto colors  = std::vector{
                    glm::vec3(0.12f, 0.29f, 0.72f),
                    glm::vec3(0.13f, 0.30f, 0.76f),
                    glm::vec3(0.77f, 0.80f, 0.28f),
                    glm::vec3(0.20f, 0.55f, 0.0f),
                    glm::vec3(0.14f, 0.36f, 0.0f),
                    glm::vec3(0.30f, 0.20f, 0.17f),
                    glm::vec3(0.23f, 0.18f, 0.16f),
                    glm::vec3(1.0f, 1.0f, 1.0f),
                };

                auto color_index = 0;
                for(auto &segment_color : colors) {
                    if(height <= heights[color_index]) {
                        color = segment_color;
                        break;
                    }
                    color_index++;
                }

                terrain_attributes.push_back(Vertex { 
                    glm::vec3(x, 0.0, z),      // position
                    glm::vec3(0.0, -1.0, 0.0), // normal
                    color   // color
                }); 
                index++;
            }
        }

        Indices indices;
        indices.reserve(linear_grid_size * 2);
        for (auto x = 0; x < grid_size - 1; x++) {
            if (x % 2 == 0) {
                for (auto z = 0; z < grid_size; z++) {
                    indices.push_back((z + x * grid_size));
                    indices.push_back((z + (x + 1) * grid_size));
                }
            } else {
                for (auto z = grid_size - 1; z > 0; z--) {
                    indices.push_back((z + (x + 1) * grid_size));
                    indices.push_back((z - 1 + (x * grid_size)));
                }
            }
        }

        std::vector<unsigned int> normal_indices;
        normal_indices.reserve(18);

        auto calculate_normal = [&](unsigned int num_triangles) {
            auto normal = glm::vec3(0.0, 0.0, 0.0);
            for (auto triangle = 0; triangle < num_triangles; triangle++) {
                auto vertex_attribute_index = triangle * 3;
                auto va = terrain_attributes[normal_indices[vertex_attribute_index]].position;
                auto vb = terrain_attributes[(normal_indices[vertex_attribute_index + 1])].position;
                auto vc = terrain_attributes[(normal_indices[vertex_attribute_index + 2])].position;

                normal += glm::cross(vb - va, vc - va);

            }

            normal_indices.clear();

            return glm::normalize(normal / static_cast<float>(num_triangles));
        };

        auto va_index = 0;
        for(auto x = 0; x < grid_size; x++) {
            for(auto z = 0; z < grid_size; z++) {

                int current_row_start = x * grid_size;
                int next_row_start = (x + 1) * grid_size;
                int next_next_row_start = (x + 2) * grid_size; // lol
                int prev_row_start = (x - 1) * grid_size;

                int current_vertex = x * (grid_size) + z;
                int vertex_left_adjacent = current_vertex - 1;
                int vertex_right_adjacent = current_vertex + 1;
                int vertex_top_adjacent = current_vertex + grid_size;
                int vertex_top_left_adjacent = current_vertex + grid_size - 1;
                int vertex_top_right_adjacent = current_vertex + grid_size + 1;
                int vertex_bottom_adjacent = current_vertex - grid_size;
                int vertex_bottom_left_adjacent = current_vertex - grid_size - 1;
                int vertex_bottom_right_adjacent = current_vertex - grid_size + 1;

                // Indices are interleaved left->right then right->left we need to account for the different
                // orientations of triangles
                if (x % 2 != 0) {
                    auto num_triangles = 0;

                    // Catch out of bounds errors for:
                    // 1. vertex_left_adjacent leading to the previous row or outside the grid
                    // 2. vertex_top_adjacent leading outside the grid
                    if(vertex_left_adjacent >= 0 && vertex_left_adjacent >= current_row_start && vertex_top_adjacent < linear_grid_size) {
                        // top left
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_top_adjacent);
                        normal_indices.push_back(vertex_left_adjacent);
                        num_triangles++;
                    }
                    
                    // Catch out of bounds errors for:
                    // 1. vertex_bottom_adjacent leading outside the grid
                    // 2. vertex_left_adjacent leading to the previous row or outside the grid
                    if(vertex_bottom_adjacent >= 0 && vertex_left_adjacent >= 0 && vertex_left_adjacent >= current_row_start) {
                        // bottom left
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_left_adjacent);
                        normal_indices.push_back(vertex_bottom_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_bottom_adjacent leading outside the grid
                    // 2. vertex_bottom_right_adjacent leading to the previous row or outside the grid
                    if(vertex_bottom_adjacent >= 0 && vertex_bottom_right_adjacent >= 0 && vertex_bottom_right_adjacent < current_row_start) {
                        // bottom left
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_bottom_adjacent);
                        normal_indices.push_back(vertex_bottom_right_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_right_adjacent leading to wrong row or outside the grid
                    // 2. vertex_bottom_right_adjacent leading to the previous row or outside the grid
                    if(vertex_bottom_right_adjacent >= 0 && vertex_bottom_right_adjacent < current_row_start && vertex_right_adjacent < next_row_start) {
                        // bottom left
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_bottom_right_adjacent);
                        normal_indices.push_back(vertex_right_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_top_right_adjacent leading to the wrong vertex or outside the grid
                    // 2. vertex_right_adjacent leading to wrong row or outside the grid
                    if(vertex_right_adjacent < next_row_start && vertex_top_right_adjacent < linear_grid_size && vertex_top_right_adjacent < next_next_row_start) {
                        // bottom left
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_right_adjacent);
                        normal_indices.push_back(vertex_top_right_adjacent);
                        num_triangles++;
                    }


                    // 1. vertex_top_right_adjacent leading to the wrong vertex or outside the grid
                    // 2. vertex_right_adjacent leading to wrong row or outside the grid
                    if(vertex_top_adjacent < linear_grid_size && vertex_top_right_adjacent < linear_grid_size && vertex_top_right_adjacent < next_next_row_start) {
                        // bottom left
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_top_right_adjacent);
                        normal_indices.push_back(vertex_top_adjacent);
                        num_triangles++;
                    }

                    terrain_attributes[va_index].normal = calculate_normal(num_triangles);
                } else {
                    auto num_triangles = 0;

                    // Catch out of bounds errors for:
                    // 1. vertex_top_left_adjacent leading to the previous row
                    // 2. vertex_top_adjacent leading outside the grid
                    if(vertex_top_adjacent < linear_grid_size && vertex_top_left_adjacent >= next_row_start && vertex_top_left_adjacent < next_next_row_start) {
                        // left top
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_top_adjacent);
                        normal_indices.push_back(vertex_top_left_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_left_adjacent leading outside the grid
                    // 2. vertex_left_adjacent leading into wrong row
                    // 3. vertex_top_adjacent leading outside the grid
                    if(vertex_left_adjacent >= current_row_start && vertex_top_left_adjacent >= next_row_start && vertex_top_left_adjacent < next_next_row_start) {
                        // left upper middle
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_top_left_adjacent);
                        normal_indices.push_back(vertex_left_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_bottom_left_adjacent leading outside the grid
                    // 2. vertex_left_adjacent leading into wrong row
                    if(vertex_left_adjacent >= current_row_start && vertex_bottom_left_adjacent >= 0 && vertex_bottom_left_adjacent > prev_row_start) {
                        // left lower middle
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_left_adjacent);
                        normal_indices.push_back(vertex_bottom_left_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_bottom_left_adjacent leading outside the grid
                    // 2. vertex_bottom_adjacent leading outside the grid
                    if(vertex_bottom_left_adjacent >= 0 && vertex_bottom_left_adjacent > prev_row_start && vertex_bottom_adjacent >= 0) {
                        // left bottom
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_bottom_left_adjacent);
                        normal_indices.push_back(vertex_bottom_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_top_left_adjacent leading to the previous row
                    // 2. vertex_top_adjacent leading outside the grid
                    if(vertex_right_adjacent < next_row_start && vertex_top_adjacent < linear_grid_size) {
                        // right top
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_right_adjacent);
                        normal_indices.push_back(vertex_top_adjacent);
                        num_triangles++;
                    }

                    // Catch out of bounds errors for:
                    // 1. vertex_bottom_adjacent leading outside the grid
                    // 2. vertex_right_adjacent leading to wrong row or outside the grid
                    if(vertex_bottom_adjacent >= 0 && vertex_right_adjacent < next_row_start) {
                        // right bottom
                        normal_indices.push_back(current_vertex);
                        normal_indices.push_back(vertex_bottom_adjacent);
                        normal_indices.push_back(vertex_right_adjacent);
                        num_triangles++;
                    }

                    terrain_attributes[va_index].normal = calculate_normal(num_triangles);
                }

                va_index++;
            }
        }

        return std::tuple(terrain_attributes, indices, indices.size());
    }

    static std::vector<float> generate_height_map(
        const unsigned int grid_size,
        const unsigned int seed, 
        const float scale, 
        const int octaves, 
        const float persistance, 
        const float lacunarity, 
        const glm::vec2 offset)
    {
		std::vector<float> noise_map(grid_size * grid_size);

        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dis(-100000, 100000);
		std::vector<glm::vec2> octave_offsets(octaves);
		for (int octave = 0; octave < octaves; octave++) {
			float offset_x = dis(gen) + offset.x;
			float offset_y = dis(gen) + offset.y;
			octave_offsets[octave] = glm::vec2(offset_x, offset_y);
		}

		float max_noise_height = std::numeric_limits<float>::min();
		float min_noise_height = std::numeric_limits<float>::max();

		float half_width = grid_size / 2.0f;
		float half_height = grid_size / 2.0f;

        auto inverse_lerp = [](float a, float b, float x) {
            return (x - a) / (b - a);
        };

        auto index = 0;
		for (int y = 0; y < grid_size; y++) {
			for (int x = 0; x < grid_size; x++) {
		
				float amplitude = 1.0f;
				float frequency = 1.0f;
				float noise_height = 0.0f;

				for (int i = 0; i < octaves; i++) {
					float sample_x = (x - half_width) / scale * frequency + octave_offsets[i].x;
					float sample_y = (y - half_height) / scale * frequency + octave_offsets[i].y;

					float perlin_value = Perlin::noise (sample_x, sample_y, sample_x + sample_y) * 2 - 1;
					noise_height += perlin_value * amplitude;

					amplitude *= persistance;
					frequency *= lacunarity;
				}

				if (noise_height > max_noise_height) {
					max_noise_height = noise_height;
				} else if (noise_height < min_noise_height) {
					min_noise_height = noise_height;
				}

				noise_map[index] = inverse_lerp(min_noise_height, max_noise_height, noise_height);
                index++;
			}
		}

		return noise_map;
    }

    VertexBufferObject vbo;
    VertexBufferObject ebo;
    unsigned int draw_count;
    Indices indices;
    unsigned int grid_size;
};