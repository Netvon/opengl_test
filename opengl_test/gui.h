#pragma once

#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include "sdl.h"
#include "opengl.h"

namespace gui {

	namespace data {
		SDL_Window* window_ptr = nullptr;
		bool show_demo = true;
		static bool show_metrics;
	}

	ImGuiIO& get_io() {
		return ImGui::GetIO();
	}

	void init(SDL_Window* window_ptr, SDL_GLContext gl_context) {

		data::window_ptr = window_ptr;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplSDL2_InitForOpenGL(window_ptr, gl_context);
		ImGui_ImplOpenGL3_Init("#version 450");
	}

	void process_event(const SDL_Event& event_) {
		ImGui_ImplSDL2_ProcessEvent(&event_);
	}

	void update() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(data::window_ptr);
		ImGui::NewFrame();
	}

	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void show_demo() {
		ImGui::ShowDemoWindow(&data::show_demo);
	}


	template<typename Callable>
	void show_window(Callable func) {
		ImGuiIO& io = ImGui::GetIO();

		func(io);
	}

	void show_fps_overlay() {

		static bool show_metrics = true;

		auto& io = ImGui::GetIO();

		ImGui::Begin("Hello, world!", &show_metrics, ImGuiWindowFlags_NoDecoration);
		ImGui::Text("fps: %.1f | frame time: %.3f", io.Framerate, 1000.f / io.Framerate);
		ImGui::End();
	}

	void show_logs(bool& show) {

		ImGui::Begin("Logs", &show, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Columns(2);

		for (std::size_t i = 0; i < std::min(50llu, main_logger.size()); i++)
		{
			auto& message = main_logger.at(i);
			ImGui::Text("%d", message.timestamp);
			ImGui::NextColumn();
			ImGui::Text(message.text.c_str());
			ImGui::NextColumn();
		}

		ImGui::Columns(1);
		ImGui::End();

	}
}