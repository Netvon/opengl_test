#pragma once
#include <SDL.h>
#include "print.h"

// undef the main from <sdl>
#undef main

namespace sdl {
	// ============================================================================================================================
	SDL_Window * window_ptr = nullptr;
	SDL_GLContext gl_context = nullptr;
	// ============================================================================================================================
	namespace data {
		int window_height = 0;
		int window_width = 0;

		int key_amount = 0;
		const unsigned char * keyboard_state = nullptr;

		int key_mod = KMOD_NONE;

		int mouse_relative_x = 0;
		int mouse_relative_y = 0;

		SDL_Keysym key_up_state{};
	}
	// ============================================================================================================================

	void print_sdl_error(const char * message) {
		print_error(message, " - ", SDL_GetError());
	}

	// ============================================================================================================================

	bool enable_fullscreen() {
		return SDL_SetWindowFullscreen(window_ptr, SDL_WINDOW_FULLSCREEN) < 0;
	}

	bool disable_fullscreen() {
		return SDL_SetWindowFullscreen(window_ptr, 0) < 0;
	}

	void set_title(const char * title) {
		SDL_SetWindowTitle(window_ptr, title);
	}

	void set_title(const std::string& title) {
		SDL_SetWindowTitle(window_ptr, title.c_str());
	}

	bool set_capture_mouse(bool enable = true) {

		SDL_bool set = SDL_TRUE;

		if (!enable) {
			set = SDL_FALSE;
		}

		// try to set capture mouse
		if (SDL_SetRelativeMouseMode(set) < 0) {
			print_sdl_error("Failed to set capture mouse");
			return false;
		}

		return true;
	}

	// ============================================================================================================================

	void update_key_state() {
		SDL_PumpEvents();

		data::key_mod = SDL_GetModState();
		data::keyboard_state = SDL_GetKeyboardState(&data::key_amount);
	}

	void update_key_up_state(const SDL_Keysym& state) {
		data::key_up_state = state;
	}

	template<typename ... T, std::enable_if_t<std::conjunction_v<std::is_same<T, const char*>...>, int> = 0>
	bool is_key_down(T ... key) {
		return (data::keyboard_state[SDL_GetScancodeFromName(key)], ...);
	}

	bool is_key_up(const char* key) {
		return data::key_up_state.scancode == SDL_GetScancodeFromName(key);
	}

	bool is_mod_down(int key_mod) {
		return data::key_mod & key_mod;
	}

	void update_mouse_relative(int x, int y) {
		data::mouse_relative_y = x;
		data::mouse_relative_x = y;
	}

	// ============================================================================================================================

	void destroy_window() {

		// clean up SDL before closing the application
		// we check gl_context and window_ptr for nullptr to prevent errors

		if (gl_context != nullptr) {
			SDL_GL_DeleteContext(gl_context);
		}

		if (window_ptr != nullptr) {
			SDL_DestroyWindow(window_ptr);
			window_ptr = nullptr;
		}

		SDL_Quit();
	}

	bool create_window(const char * title, int x, int y, int width, int height, bool full_screen) {
		// initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			print_sdl_error("SDL init failed");
			return false;
		}

		// create a window, enable opengl on it
		window_ptr = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

		if (window_ptr == nullptr) {
			print_sdl_error("Failed to create window");
			return false;
		}

		// save the width and height of the window so others can also use it
		data::window_width = width;
		data::window_height = width;

		// make the window resizable
		SDL_SetWindowResizable(window_ptr, SDL_TRUE);

		set_capture_mouse();

		if (full_screen) {

			// try to enable fullscreen
			if (SDL_SetWindowFullscreen(window_ptr, SDL_WINDOW_FULLSCREEN) < 0) {
				print_sdl_error("Failed to enable fullscreen");
				return false;
			}
		}

		// create the opengl context
		gl_context = SDL_GL_CreateContext(window_ptr);

		// enable updates for every X frames
		// set to 0 to immediate
		// set to 1 to match screen refresh rate
		if (SDL_GL_SetSwapInterval(1) < 0) {
			print_sdl_error("Failed to set vsync");
		}

		if (gl_context == nullptr) {
			print_sdl_error("Failed to create OpenGL context");
			return false;
		}

		return true;
	}

	float get_aspect_ratio() {

		if (window_ptr != nullptr) {
			// update the screen_width and screen_height params with the current values
			SDL_GetWindowSize(window_ptr, &data::window_width, &data::window_height);

			return static_cast<float>(data::window_width) / static_cast<float>(data::window_height);
		}

		return 1.0;
	}

}