#pragma once
#include "timer.h"

namespace globals {

	// the struct that holds all the globals
	// the globals should be accessed through the "get()" method
	struct globals_container {

		bool should_quit() const {
			return should_quit_;
		}

		void should_quit(bool new_value) {
			should_quit_ = new_value;
		}

		timer::timer_impl& main_timer() {
			return timer_;
		}

		const int& draw_mode() const {
			return draw_mode_;
		}

		void draw_mode(int new_value) {
			draw_mode_ = new_value;
		}

	private:
		bool should_quit_ = false;

		timer::timer_impl timer_ = {};
		int draw_mode_ = GL_TRIANGLES;
	};

	globals_container& get() {
		static globals_container container;

		return container;
	}
	
}

#define global globals::get()

// A shortcut to the applications Main Timer
#define main_timer globals::get().main_timer()

// Defines the the elapsed time between updates
#define delta_time main_timer.elapsed_time()

// Defines the the elapsed time between updates
#define fps main_timer.per_second()