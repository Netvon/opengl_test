#pragma once
#include "opengl.h"
#include "sdl.h"
#include <string>
#include <map>
#include <iomanip>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "shader.h"

namespace opengl::image {

	struct texture_info {
		unsigned int id{};
		const char * name = nullptr;
		int width{};
		int height{};
	};

	namespace data {
		std::map<const char *, texture_info> loaded_textures;
		const texture_info empty_info{};
	}

	enum texture_number : int {
		TEXTURE0 = 0x84C0,
		TEXTURE1 = 0x84C1,
		TEXTURE2 = 0x84C2,
		TEXTURE3 = 0x84C3,
		TEXTURE4 = 0x84C4,
		TEXTURE5 = 0x84C5,
		TEXTURE6 = 0x84C6,
		TEXTURE7 = 0x84C7,
		TEXTURE8 = 0x84C8,
		TEXTURE9 = 0x84C9,
		TEXTURE10 = 0x84CA,
		TEXTURE11 = 0x84CB,
		TEXTURE12 = 0x84CC,
		TEXTURE13 = 0x84CD,
		TEXTURE14 = 0x84CE,
		TEXTURE15 = 0x84CF,
		TEXTURE16 = 0x84D0,
		TEXTURE17 = 0x84D1,
		TEXTURE18 = 0x84D2,
		TEXTURE19 = 0x84D3,
		TEXTURE20 = 0x84D4,
		TEXTURE21 = 0x84D5,
		TEXTURE22 = 0x84D6,
		TEXTURE23 = 0x84D7,
		TEXTURE24 = 0x84D8,
		TEXTURE25 = 0x84D9,
		TEXTURE26 = 0x84DA,
		TEXTURE27 = 0x84DB,
		TEXTURE28 = 0x84DC,
		TEXTURE29 = 0x84DD,
		TEXTURE30 = 0x84DE,
		TEXTURE31 = 0x84DF
	};

	bool load(unsigned int& texture_id, const char * path, const char * name) {
		stbi_set_flip_vertically_on_load(true);

		int x = 0;
		int y = 0;
		int comp = 0;

		unsigned char * image_data = stbi_load(path, &x, &y, &comp, 0);

		if (image_data == nullptr) {

			print_error("Failed to load image ", std::quoted(name));

			if (!std::filesystem::exists(path)) {
				print_error(std::quoted(path), " file not found");
			}

			stbi_image_free(image_data);
			return false;
		}

		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		data::loaded_textures.insert(std::make_pair(name, texture_info{ texture_id, name, x, y }));

		print_info("Loaded image ", std::quoted(path), " as ", std::quoted(name), ": [id:", texture_id, "][w:", x, ",h:", y, "]");

		stbi_image_free(image_data);

		return true;
	}

	bool load(const char * path, const char * name) {

		unsigned int local_id;
		return load(local_id, path, name);
	}

	void bind(const char * uniform_name, unsigned int texture_id, unsigned int shader_id, texture_number tex_num = TEXTURE0) {
		if (glIsTexture(texture_id) == GL_TRUE) {
			glActiveTexture(tex_num);

			auto data = static_cast<int>(tex_num - TEXTURE0);
			shader::set(uniform_name, shader_id, data);

			glBindTexture(GL_TEXTURE_2D, texture_id);
		}
		else {
			print_error("bind Unkown image id: ", texture_id);
		}
	}

	void bind(const char * uniform_name, const char * name, unsigned int shader_id, texture_number tex_num = TEXTURE0) {

		auto find = data::loaded_textures.find(name);

		if (find != data::loaded_textures.end()) {

			const auto& name_info_pair = *find;

			unsigned int texture_id = name_info_pair.second.id;
			bind(uniform_name, texture_id, shader_id, tex_num);
		}
		else {
			print_error("bind Unkown image: ", std::quoted(name));
		}
	}

	template<typename ... Args, std::enable_if_t<sizeof...(Args) <= 32llu && sizeof...(Args) >= 1llu && std::conjunction_v<std::is_same<const char *, Args>...>, int> = 0>
	void bind_multiple(Args ... args) {
		int index = TEXTURE0;

		(bind(args, static_cast<texture_number>(index++)), ...);
	}

	template<typename ... Args, std::enable_if_t<sizeof...(Args) <= 32llu && sizeof...(Args) >= 1llu && std::conjunction_v<std::is_same<unsigned int, Args>...>, int> = 0>
	void bind_multiple(Args&& ... args) {
		int index = TEXTURE0;

		(bind(args, static_cast<texture_number>(index++)), ...);
	}

	const texture_info& info(const char * name) {
		auto find = data::loaded_textures.find(name);

		if (find != data::loaded_textures.end()) {
			const auto& name_info_pair = *find;

			return name_info_pair.second;
		}
		
		print_error("info Unkown image: ", std::quoted(name));
		return data::empty_info;
	}

}