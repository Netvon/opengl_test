#pragma once
#include <glad.c>
#include "sdl.h"
#include "print.h"
#include "world.h"
#include "shader.h"
#include "globals.h"
#include "camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace opengl {

	// ============================================================================================================================
	void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		print_error(message);
	}
	// ============================================================================================================================

	void resize_viewport(int width, int height) {

		//float vdpi = 1.f;
		//float hdpi = 1.f;

		//if (SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(sdl::window_ptr), nullptr, &hdpi, &vdpi)) {
		//	print_error("Failed to obtain DPI info");
		//}

		//int w = static_cast<int>(static_cast<float>(width) * hdpi);
		//int h = static_cast<int>(static_cast<float>(height) * vdpi);

		glViewport(0, 0, width, height);

		sdl::data::window_height = width;
		sdl::data::window_width = width;

		print_info("window resized to: ", width, "x", height);
	}

	bool create_opengl(int width, int height) {
		// load all the OpenGL methods using glad
		// assert that everything went okay
		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
			print_error("GLAD failed to load");
			return false;
		}

		auto gl_major = GLVersion.major;
		auto gl_minor = GLVersion.minor;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_minor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#ifdef DEBUG
		SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG
		);

		// setup OpenGL to output all error messages using std::cerr
		glDebugMessageCallback((GLDEBUGPROC)opengl_debug_callback, nullptr);
		glDebugMessageControl(
			GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE
		);
#endif // DEBUG

		// enable depth testing and face culling
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		resize_viewport(width, height);

		return true;
	}

	// ============================================================================================================================



	// ============================================================================================================================

	template<typename T, typename Size>
	unsigned int create_shader_uniform_buffer(Size amount, const T& data) {
		unsigned int buffer_id;

		auto amount_sizet = static_cast<decltype(sizeof(T))>(amount) * sizeof(T);
		auto amount_gl = static_cast<GLsizeiptr>(amount_sizet);

		glGenBuffers(1, &buffer_id);
		glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);

		glBufferData(GL_UNIFORM_BUFFER, amount_gl, &data, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		return buffer_id;
	}

	template<typename T, typename Size>
	unsigned int create_shader_uniform_buffer(Size amount) {
		return create_shader_uniform_buffer<T, Size>(amount, nullptr);
	}

	template<typename T, typename Size>
	unsigned int create_shader_storage_buffer(Size amount, const T& data) {
		unsigned int buffer_id;

		auto amount_sizet = static_cast<decltype(sizeof(T))>(amount) * sizeof(T);
		auto amount_gl = static_cast<GLsizeiptr>(amount_sizet);

		glGenBuffers(1, &buffer_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);

		glBufferData(GL_SHADER_STORAGE_BUFFER, amount_gl, &data, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return buffer_id;
	}

	template<typename T, typename Size>
	unsigned int create_shader_storage_buffer(Size amount) {
		return create_shader_storage_buffer<T, Size>(amount, nullptr);
	}

	template<typename T>
	void update_shader_storage_buffer(unsigned int buffer_id, unsigned int index, const T& data) {

		auto offset = static_cast<GLintptr>(index * sizeof(T));
		auto size = static_cast<GLsizeiptr>(sizeof(T));

		if (glIsBuffer(buffer_id) == GL_TRUE) {
			glNamedBufferSubData(buffer_id, offset, size, &data);
		}
		else {
			print_error("Unknown buffer: ", buffer_id);
		}

	}

	// ============================================================================================================================

	void clear_screen(const glm::vec4& color, unsigned int clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) {
		glClearColor(color.x, color.y, color.z, color.w);
		glClear(clear_flags);
	}

	// ============================================================================================================================

	void draw(const world::mesh& mesh, unsigned int shader_id) {
		glBindVertexArray(mesh.vao());


		if (mesh.has_textures()) {
			mesh.for_each_texture([&](const std::size_t& i, const world::texture& tex) {

				auto tex_num = static_cast<image::texture_number>(static_cast<int>(i) + static_cast<int>(image::TEXTURE0));
				auto name = tex.name(i);

				image::bind(name.c_str(), tex.id, shader_id, tex_num);
			});
		}

		glDrawElements(global.draw_mode(), static_cast<int>(mesh.index_size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

	void draw(camera& use_camera, const world::model& model, unsigned int shader_id) {

		glUseProgram(static_cast<GLuint>(shader_id));

		//shader::set("another_name", shader_id, glm::mat4{ 1.f });
		shader::set("projection", shader_id, use_camera.projection(sdl::get_aspect_ratio()));
		shader::set("view", shader_id, use_camera.view());

		auto translate	= glm::translate(glm::mat4(1.f), model.position);

		auto rotate_y	= glm::rotate(translate, glm::radians(model.rotation.angle_y), glm::vec3(0.f, 1.f, 0.f));
		auto rotate_z	= glm::rotate(rotate_y, glm::radians(model.rotation.angle_z), glm::vec3(0.f, 0.f, 1.f));
		auto rotate_x	= glm::rotate(rotate_z, glm::radians(model.rotation.angle_x), glm::vec3(1.f, 0.f, 0.f));

		auto scale		= glm::scale(rotate_x, model.scale);

		shader::set("model", shader_id, scale);

		for (const auto & mesh : model) {
			draw(mesh, model.shader_id);
		}
	}

	void draw(camera& use_camera, const world::model& model) {
		draw(use_camera, model, model.shader_id);
	}

	template<typename T>
	void draw_instanced(camera& use_camera, const world::model& model, const T& instance_amount) {

		glUseProgram(model.shader_id);

		//shader::set("origin", model.shader_id, glm::mat4{ 1.f });
		shader::set("projection", model.shader_id, use_camera.projection(sdl::get_aspect_ratio()));
		shader::set("view", model.shader_id, use_camera.view());

		for (const auto& mesh : model) {

			if (mesh.has_textures()) {
				mesh.for_each_texture([&](const std::size_t& i, const world::texture& tex) {

					auto tex_num = static_cast<image::texture_number>(static_cast<int>(i) + static_cast<int>(image::TEXTURE0));
					auto name = tex.name(i);

					image::bind(name.c_str(), tex.id, model.shader_id, tex_num);
				});
			}

			auto size = static_cast<GLsizei>(mesh.index_size());

			glBindVertexArray(mesh.vao());
			glDrawElementsInstanced(global.draw_mode(), size, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instance_amount));
			glBindVertexArray(0);
		}
	}
}