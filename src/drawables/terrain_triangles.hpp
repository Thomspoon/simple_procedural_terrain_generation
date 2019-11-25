#pragma once

#include <cmath>
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

class TerrainTriangles : public Drawable<TerrainTriangles> {
public:
    explicit TerrainTriangles(
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

    static std::shared_ptr<TerrainTriangles> create_impl(const unsigned int grid_size) {
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

        return std::make_shared<TerrainTriangles>(
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
            VertexPrimitive::TRIANGLES,
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

        auto height_scale = 13.50;

        VertexData terrain_attributes(linear_grid_size, Vertex {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f)
        });

        Indices indices;
        indices.reserve(linear_grid_size * 2);

        auto curve_fit = [](float height) {
            return std::powf(height, 3) - 0.15 * std::pow(height, 2) + 0.15 * height;
        };

        auto index = 0;
        for(auto x = 0; x < grid_size; x++) {
            for(auto z = 0; z < grid_size; z++) {
                auto height = curve_fit(height_map[index]);

                terrain_attributes[index].position = glm::vec3(x, height * height_scale, z);

                // Calculate indices
                if(x < grid_size - 1 && z < grid_size - 1) {
                    indices.push_back(index);
                    indices.push_back(index + grid_size + 1);
                    indices.push_back(index + grid_size);

                    indices.push_back(index + grid_size + 1);
                    indices.push_back(index);
                    indices.push_back(index + 1);
                }
                index++;
            }
        }

        // TODO: Put this somewhere
        static auto heights = std::vector{0.3, 0.4, 0.45, 0.55, 0.6, 0.7, 0.9, 1.0};
        static auto colors  = std::vector {
            glm::vec3(0.12f, 0.29f, 0.72f),
            glm::vec3(0.13f, 0.30f, 0.76f),
            glm::vec3(0.77f, 0.80f, 0.28f),
            glm::vec3(0.20f, 0.55f, 0.0f),
            glm::vec3(0.14f, 0.36f, 0.0f),
            glm::vec3(0.30f, 0.20f, 0.17f),
            glm::vec3(0.23f, 0.18f, 0.16f),
            glm::vec3(1.0f, 1.0f, 1.0f),
        };

        auto height_centroid = [](float height1, float height2, float height3) {
            return (height1 + height2 + height3) / 3.0f;
        };

        auto va_index = 0;
        for(auto x = 0; x < grid_size - 1; x++) {
            for(auto z = 0; z < grid_size - 1; z++) {
                // Extract first triangle using same method to calculate indices
                auto& triangle1_va = terrain_attributes.at(va_index);
                auto& triangle1_vb = terrain_attributes.at(va_index + grid_size + 1);
                auto& triangle1_vc = terrain_attributes.at(va_index + grid_size);

                // Same with second triangle
                auto& triangle2_va = terrain_attributes.at(va_index + grid_size + 1);
                auto& triangle2_vb = terrain_attributes.at(va_index);
                auto& triangle2_vc = terrain_attributes.at(va_index + 1);

                // Recalculate surface normals
                auto triangle1_normal = glm::normalize(glm::cross(triangle1_vb.position - triangle1_va.position, triangle1_vc.position - triangle1_va.position));
                triangle1_va.normal = triangle1_normal;
                triangle1_vb.normal = triangle1_normal;
                triangle1_vc.normal = triangle1_normal;

                auto triangle2_normal = glm::normalize(glm::cross(triangle2_vb.position - triangle2_va.position, triangle2_vc.position - triangle2_va.position));
                triangle2_va.normal = triangle2_normal;
                triangle2_vb.normal = triangle2_normal;
                triangle2_vc.normal = triangle2_normal;

                // Recalculate colors
                //   Find triangle1 color
                // Extract first triangle using same method to calculate indices
                auto& triangle1_va_height = height_map[index];
                auto& triangle1_vb_height = height_map[va_index + grid_size + 1];
                auto& triangle1_vc_height = height_map[va_index + grid_size];

                // Same with second triangle
                auto& triangle2_va_height = height_map[va_index + grid_size + 1];
                auto& triangle2_vb_height = height_map[va_index];
                auto& triangle2_vc_height = height_map[va_index + 1];

                auto triangle1_centroid_height = height_centroid(triangle1_va_height, triangle1_vb_height, triangle1_vc_height);

                auto color = glm::vec3(1.0f, 1.0f, 1.0f);

                auto color_index = 0;
                auto height = triangle1_centroid_height;
                for(auto &segment_color : colors) {
                    if(height <= heights[color_index]) {
                        color = segment_color;
                        break;
                    }
                    color_index++;
                }

                triangle1_va.color = color;
                triangle1_vb.color = color;
                triangle1_vc.color = color;

                //   Find triangle2 color
                auto triangle2_centroid_height = height_centroid(triangle2_va_height, triangle2_vb_height, triangle2_vc_height);

                color_index = 0;
                height = triangle2_centroid_height;
                for(auto &segment_color : colors) {
                    if(height <= heights[color_index]) {
                        color = segment_color;
                        break;
                    }
                    color_index++;
                }

                triangle2_va.color = color;
                triangle2_vb.color = color;
                triangle2_vc.color = color;

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

				noise_map[index] = noise_height;
                index++;
			}
		}

        auto inverse_lerp = [](float a, float b, float x) {
            return (x - a) / (b - a);
        };

        index = 0;
		for (int y = 0; y < grid_size; y++) {
			for (int x = 0; x < grid_size; x++) {
				noise_map[index] = inverse_lerp(min_noise_height, max_noise_height, noise_map[index]);
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