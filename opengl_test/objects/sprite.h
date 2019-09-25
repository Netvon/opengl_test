#pragma once
#include "../shader.h"
#include "../opengl.h"
#include "../world.h"
#include "../image.h"
#include "../shader.h"

namespace objects {

	// ============================================================================================================================

	constexpr const char * sprite_frag =
		R"(#version 450 core
			layout(location = 0) out vec4 diffuseColor;
			
			in vec2 tex_coord;
			in vec2 tex_coord_offset;

			uniform sampler2D texture_diffuse_0;

			void main() 
			{
				vec2 offset = tex_coord * tex_coord_offset;
				vec4 tex_col = texture(texture_diffuse_0, tex_coord + offset);
				diffuseColor = vec4(tex_col.x, 1.0f, 1.0f, 1.0f);
			})";

	constexpr const char * sprite_vert =
		R"(#version 450 core
			layout(location = 0) in vec3 aPos;
			layout(location = 5) in vec2 aTexCoord;

			uniform mat4 projection;
			uniform mat4 view;
			uniform vec2 divisions;

			out vec2 tex_coord;
			out vec2 tex_coord_offset;

			layout (std430, binding = 0) buffer instance_sprite_info {
				float index[];
			};

			layout (std430, binding = 1) buffer instance_model_info {
				mat4 model[];
			};

			void main()
			{
				float i = floor(index[gl_InstanceID]) - 1.0f;
				if(i == 0.0f) {
					tex_coord_offset = vec2(0.0f, 0.0f);
				} else {
					tex_coord_offset = vec2(mod(i, divisions.x), floor(i / divisions.x));
				}

				vec3 pos = vec3(model[gl_InstanceID] * vec4(aPos, 1.0f));

				tex_coord = aTexCoord;

				gl_Position = projection * view * vec4(pos, 1.0f);
			})";

	// ============================================================================================================================
	struct sprite {
		unsigned int sprite_index = 0;
		glm::vec3 location{};
		glm::vec3 rotation{};
		glm::vec3 scale{};

		sprite(unsigned int sprite_index, const glm::vec3& location)
			: sprite_index(sprite_index), location(location)
		{

		}
	};


	struct sprite_sheet {

		sprite_sheet(const glm::vec2& sprite_size, int h_divisions, unsigned int w_divisions, const char * path_to_file)
			: sprite_size(sprite_size), h_divisions(h_divisions), w_divisions(w_divisions), path_to_file(path_to_file) {

			auto rect_x = 1.0f / static_cast<float>(w_divisions);
			auto rect_y = 1.0f - (1.0f / static_cast<float>(h_divisions));

			uv_rect.x = rect_x;
			uv_rect.y = rect_y;
		}

		bool load() {

			auto norm = glm::normalize(sprite_size);

			if (auto result = shader::load_shader(sprite_vert, sprite_frag)) {
				program_id = *result;
			}
			else {
				return false;
			}

			if (!opengl::image::load(texture_id, path_to_file, path_to_file)) {
				return false;
			}

			create_mesh();

			instance_sprite_info_index = glGetProgramResourceIndex(program_id, GL_SHADER_STORAGE_BLOCK, "instance_sprite_info");
			instance_model_info_index = glGetProgramResourceIndex(program_id, GL_SHADER_STORAGE_BLOCK, "instance_model_info");

			return instance_sprite_info_index != GL_INVALID_INDEX &&
				instance_model_info_index != GL_INVALID_INDEX;

		}

		std::size_t add_sprite(unsigned int sprite_index, const glm::vec3& location) {
			sprites_.emplace_back(sprite_index, location);
			return sprites_.size() - 1llu;
		}

		void setup() {
			glUseProgram(program_id);

			std::transform(sprites_.begin(), sprites_.end(), std::back_inserter(instance_sprite_info_list),
				[](const auto& s) { return static_cast<float>(s.sprite_index); });

			std::transform(sprites_.begin(), sprites_.end(), std::back_inserter(instance_model_info_list),
				[](const auto& s) { return glm::translate(glm::mat4(1.f), s.location); });
				 
			instance_sprite_info_buffer = opengl::create_shader_storage_buffer(instance_sprite_info_list.size(), instance_sprite_info_list[0]);
			instance_model_info_buffer = opengl::create_shader_storage_buffer(instance_model_info_list.size(), instance_model_info_list[0]);


			//glUniformBlockBinding(program_id, sprite_info_index, uniform_sprite_info_buffer);
			glShaderStorageBlockBinding(program_id, 0, instance_sprite_info_buffer);
			glShaderStorageBlockBinding(program_id, 1, instance_model_info_buffer);

			glUseProgram(0);
			
		}

		void draw() {
			glUseProgram(program_id);
			
			auto divisions = glm::vec2(static_cast<float>(w_divisions), static_cast<float>(h_divisions));
			shader::set("divisions", program_id, divisions);

			world::model& model = world::model_get(model_id);
			auto& use_camera = game::data::main_camera;
			
			//opengl::draw_instanced(game::data::main_camera, model, sprites_.size());

			shader::set("projection", model.shader_id, use_camera.projection(sdl::get_aspect_ratio()));
			shader::set("view", model.shader_id, use_camera.view());

			

			for (const auto& mesh : model) {

				if (mesh.has_textures()) {
					mesh.for_each_texture([&](const std::size_t& i, const world::texture& tex) {

						auto tex_num = static_cast<opengl::image::texture_number>(static_cast<int>(i) + static_cast<int>(opengl::image::TEXTURE0));
						auto name = tex.name(i);

						opengl::image::bind(name.c_str(), tex.id, model.shader_id, tex_num);
					});
				}

				auto size = static_cast<GLsizei>(mesh.index_size());

				glBindVertexArray(mesh.vao());
				glDrawElementsInstanced(global.draw_mode(), size, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(sprites_.size()));
				glBindVertexArray(0);
			}
		}

	private:

		void create_mesh() {
			std::string name = "plane";
			auto normal = glm::vec3(0.f, 0.f, 1.f);

			std::initializer_list<world::mesh> mesh{
				{	name,
					{
						{ // 0
							glm::vec3(sprite_size, 0.f),						// position	1.f, 2.f, 0.f
							normal,												// normal
							glm::vec2(0.f, 1.f)									// uv		0.f, 1.f
						},
						{ // 1
							glm::vec3(-sprite_size, 0.f),						// position	-1.f, -2.f, 0.f
							normal,												// normal
							uv_rect												// uv		0.125f, 0.75f
						},
						{ // 2
							glm::vec3(sprite_size.x, -sprite_size.y, 0.f),		// position 1.f, -2.f, 0.f
							normal,												// normal
							glm::vec2(0.0f, uv_rect.y)							// uv		0.0f, 0.75f
						},
						{ // 3
							glm::vec3(-sprite_size.x, sprite_size.y, 0.f),		// position	-1.f, 2.f, 0.f
							normal,												// normal
							glm::vec2(uv_rect.x, 1.f)							// uv		0.125f, 1.f
						}
					},
					{ 0u, 1u, 2u, 0u, 3u, 2u },									// indices
					{ { texture_id, world::texture_type::diffuse_texture } }
				}
			};

			world::create_model(model_id, mesh);
			world::model_set_shader(model_id, program_id);
			world::setup_model(model_id);
		}


		unsigned int program_id = 0;
		unsigned int texture_id = 0;

		unsigned int h_divisions = 1;
		unsigned int w_divisions = 1;

		unsigned int uniform_sprite_info_buffer;
		unsigned int instance_sprite_info_buffer;
		unsigned int instance_model_info_buffer;
		unsigned int sprite_info_index;
		unsigned int instance_sprite_info_index;
		unsigned int instance_model_info_index;

		glm::vec2 sprite_size;
		glm::vec2 uv_rect;

		std::size_t model_id = 0;

		const char * path_to_file;

		std::vector<sprite> sprites_;
		std::vector<float> instance_sprite_info_list;
		std::vector<glm::mat4> instance_model_info_list;
	};

}