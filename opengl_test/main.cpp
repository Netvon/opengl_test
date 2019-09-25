#include "gui.h"
#include "print.h"
#include "sdl.h"
#include "opengl.h"
#include "globals.h"
#include "world.h"
#include "shader.h"
#include "game.h"
#include "image.h"
//#include "objects/sprite.h"

#include <iostream>

// ============================================================================================================================
constexpr auto initial_view_width = 1920;
constexpr auto initial_view_height = 1080;
const glm::vec4 clear_color{ 0.f, 0.f, 0.f, 1.f };
// ============================================================================================================================

int main() {

	if (!sdl::create_window("OpenGL Test", 100, 100, initial_view_width, initial_view_height, false) ||
		!opengl::create_opengl(initial_view_width, initial_view_height)) {

		bool nop;
		std::cin >> nop;

		return 1;
	}
/*
	objects::sprite_sheet ss(glm::vec2(0.5f, 1.f), 4u, 8u, R"(assets/textures/sheet_01.png)");
	ss.load();
	ss.add_sprite(0, glm::vec3(0.f, 0.f, -5.f));
	ss.add_sprite(1, glm::vec3(0.f, 0.f, 5.f));
	ss.add_sprite(1, glm::vec3(0.f, 10.f, -5.f));
	ss.setup();*/

	gui::init(sdl::window_ptr, sdl::gl_context);

	// initialize any game data
	game::on_init();

	print_info("all okay!");

	// create an epmty SDL_Event variable to hold the current event while itterating below
	SDL_Event event_{};

	// begin the main event loop
	// keep running until should_quit becomes true
	while (!global.should_quit()) {

		opengl::clear_screen(clear_color);

		// update the global timer
		// this can be used to get the elapsed time between updates
		main_timer.update();

		bool had_mouse_update = false;
		bool had_key_up_update = false;

		// poll all SDL events - itterates over all events
		while (SDL_PollEvent(&event_) != 0) {

			gui::process_event(event_);

			if (event_.type == SDL_QUIT) {
				global.should_quit(true);
			}

			// update the OpenGL viewport when the window is resized
			else if (event_.type == SDL_WINDOWEVENT && event_.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				opengl::resize_viewport(event_.window.data1, event_.window.data2);
			} 

			else if (event_.type == SDL_MOUSEMOTION) {
				sdl::update_mouse_relative(event_.motion.xrel, event_.motion.yrel);
				had_mouse_update = true;
			}

			else if (event_.type == SDL_KEYUP) {
				sdl::update_key_up_state(event_.key.keysym);
				had_key_up_update = true;
			}
		}

		gui::update();
		gui::show_fps_overlay();

		if (!had_mouse_update) {
			sdl::update_mouse_relative(0, 0);
		}

		if (!had_key_up_update) {
			sdl::update_key_up_state({});
		}

		sdl::update_key_state();

		game::on_update();
		game::on_draw();

		gui::render();

		SDL_GL_SwapWindow(sdl::window_ptr);
	}

	sdl::destroy_window();

	return 0;
}

// ============================================================================================================================