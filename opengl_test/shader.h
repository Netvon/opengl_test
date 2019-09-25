#pragma once
#include <glm/glm.hpp>
#include <type_traits>
#include <optional>
#include "print.h"
namespace shader {



	template<typename Given, typename Expected>
	using enable_if_same_t = typename std::enable_if_t<std::is_same<Given, Expected>::value, int>;

#define ENABLE_IF_SAME(given, expected) enable_if_same_t<given, expected> = 0

	constexpr const char * basic_frag =
		R"(#version 450 core
			layout(location = 0) out vec4 diffuseColor;
			
			in vec3 ourColor;
			in vec3 normal;
			in vec3 pos;
			in vec2 tex_coord;

			//uniform sampler2D texture_diffuse_0;

			void main() 
			{
				//vec2 tex_offset = vec2(1.0 / 8.0, 0);
				//vec3 tex_col = vec3(texture(texture_diffuse_0, tex_coord + tex_offset));
				vec3 ambient = 0.1 * vec3(1.0, 1.0, 1.0);

				vec3 norm = normal;
				vec3 light_dir = normalize(vec3(5.0, 5.0, 5.0) - pos);
				float dp = max(dot(norm, light_dir), 0.0);
				vec3 diffuse = dp * vec3(1.0, 1.0, 1.0);
				
				diffuseColor = vec4((ambient + diffuse) * ourColor, 1);
				//diffuseColor = vec4(pos, 1);
				//diffuseColor = vec4(l_normal, 1);
			})";

	constexpr const char * basic_vert =
		R"(#version 450 core
			layout(location = 0) in vec3 aPos;
			layout(location = 1) in vec3 aNormal;
			layout(location = 2) in vec3 aColor;
			layout(location = 3) in vec3 aTangent;
			layout(location = 4) in vec3 aBittangent;
			layout(location = 5) in vec2 aTexCoord;

			//uniform mat4 origin;
			uniform mat4 projection;
			uniform mat4 view;
			uniform mat4 model;

			out vec3 ourColor;
			out vec3 normal;
			out vec3 pos;
			out vec2 tex_coord;

			void main()
			{
				pos = vec3(model * vec4(aPos, 1.0));
				ourColor = aColor;
				normal = mat3(transpose(inverse(model))) * aNormal;
				tex_coord = aTexCoord;

				gl_Position = projection * view * vec4(pos, 1.0);
			})";

	constexpr const char * basic_instance_vert =
		R"(#version 450 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec3 aNormal;
			layout (location = 2) in vec3 aColor;

			//uniform mat4 origin;
			uniform mat4 projection;
			uniform mat4 view;

			layout(std430, binding = 0) buffer instance {
				mat4 model[];
			};

			out vec3 ourColor;
			out vec3 normal;
			out vec3 pos;

			void main()
			{
				pos = vec3(model[gl_InstanceID] * vec4(aPos, 1.0));
				ourColor = aColor;
				normal = mat3(transpose(inverse(model[gl_InstanceID]))) * aNormal;

				gl_Position = projection * view * vec4(pos, 1.0);
			})";

	constexpr const char * unlit_frag = 
		R"(#version 450 core
			layout(location = 0) out vec4 diffuseColor;
			
			in vec3 ourColor;
			in vec3 normal;
			in vec3 pos;

			void main() 
			{
				diffuseColor = vec4(ourColor, 1);
			})";


	bool is_program(unsigned int program_id) {
		return glIsProgram(program_id) == GL_TRUE;
	}

	bool check_shader_error(unsigned int shader_id) {

		if (glIsShader(shader_id) == GL_FALSE) {
			print_error("UNKNOWN SHADER id:", shader_id);
			return true;
		}

		char buffer[1024];
		int buffer_length = 0;
		GLint no_errors{ false };

		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &no_errors);
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &buffer_length);

		if (!(static_cast<bool>(no_errors))) {
			glGetShaderInfoLog(shader_id, sizeof(buffer), nullptr, buffer);

			print_error("SHADER COMPILE: ", buffer);
			return true;
		}

		return false;
	}

	bool check_program_error(unsigned int program_id) {
		char buffer[1024];
		GLint no_errors{ false };

		glGetProgramiv(program_id, GL_LINK_STATUS, &no_errors);

		if (!(static_cast<bool>(no_errors))) {
			glGetProgramInfoLog(program_id, sizeof(buffer), nullptr, buffer);

			print_error("PROGRAM LINK: ", buffer);
			return true;
		}

		return false;
	}

	bool load_shader(unsigned int& shader_id, const char * vertex_shader, const char * fragment_shader) {

		assert(vertex_shader != nullptr);
		assert(fragment_shader != nullptr);

		auto vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertex_shader, nullptr);
		glCompileShader(vertex);
		if (check_shader_error(vertex)) {
			return false;
		}

		auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragment_shader, nullptr);
		glCompileShader(fragment);
		if (check_shader_error(fragment)) {
			glDeleteShader(vertex);
			return false;
		}

		auto id_ = glCreateProgram();
		glAttachShader(id_, vertex);
		glAttachShader(id_, fragment);

		glLinkProgram(id_);
		if (check_program_error(id_)) {
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			return false;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		shader_id = id_;
		return true;
	}

	std::optional<unsigned int> load_shader(const char * vertex_shader, const char * fragment_shader) {

		unsigned int output = 0;
		if (load_shader(output, vertex_shader, fragment_shader)) {
			return std::make_optional(output);
		}

		return std::nullopt;
	}


	std::optional<int> get_location(const char* name, unsigned int program_id) {
		assert(name != nullptr);
		assert(strlen(name) > 0);

		if (!is_program(program_id)) {
			print_error("UNKNOWN SHADER id:", program_id);
			return std::nullopt;
		}

		auto location = glGetUniformLocation(program_id, name);

		if (location < 0) {
			print_error("UNKNOWN UNIFORM ", std::quoted(name), " on program: ", program_id, ". Uniform might have been optimized away");
			return std::nullopt;
		}

		return std::make_optional(location);
	}

	// sets a float value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, float)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform1f(*location, value);
		}
	}

	// sets an int value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, int)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform1i(*location, value);
		}
	}

	// sets a bool value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, bool)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform1i(location, value);
		}
	}

	// sets an unsigned int value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, unsigned int)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform1ui(*location, value);
		}
	}

	// sets a vec2 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::vec2)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform2fv(*location, 1, &value[0]);
		}
	}

	// sets a vec3 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::vec3)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform3fv(*location, 1, &value);
		}
	}

	// sets a vec4 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::vec4)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniform4fv(*location, 1, &value);
		}
	}

	// sets a mat2 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat2)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix2fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat2x3 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat2x3)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix2x3fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat2x4 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat2x4)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix2x4fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat3 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat3)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix3fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat3x2 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat3x2)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix3x2fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat3x4 value for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat3x4)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix3x4fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat4 for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat4)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix4fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat4x2 for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat4x2)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix4x2fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}

	// sets a mat4x3 for program 'id' at 'location'
	template<typename TValue, ENABLE_IF_SAME(TValue, glm::mat4x3)>
	void set(const char* name, unsigned int id, const TValue& value) {
		if (auto location = get_location(name, id)) {
			glUniformMatrix4x3fv(*location, 1, GL_FALSE, &value[0][0]);
		}
	}
}