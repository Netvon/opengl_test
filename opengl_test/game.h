#pragma once
#include "globals.h"
#include "world.h"
#include "shader.h"
#include "opengl.h"
#include "camera.h"
#include "random.h"
#include "gui.h"

#include <sstream>

namespace game 
{
	// ============================================================================================================================

	namespace data {
		std::size_t ship_index = 0llu;
		std::size_t cube_index = 0llu;

		unsigned int ship_shader_id = 0u;
		unsigned int cube_shader_id = 0u;

		camera main_camera;

		bool is_in_fullscreen = false;
		bool capture_mouse = true;
		bool show_ship_ui = true;

		std::vector<glm::mat4> locations;
		unsigned int instance_buffer;

		float ship_velocity = 0.f;
	}

	// ============================================================================================================================

	constexpr unsigned int default_load_flags = aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;
	constexpr unsigned int default_smooth_load_flags = aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

	// ============================================================================================================================

	void control_camera(camera& control_me/*, world::model& follow_me*/) {

		if (sdl::is_key_down("I")) {
			control_me.position += control_me.forward() * delta_time * 10.f;
		}
		else if (sdl::is_key_down("K")) {
			control_me.position -= control_me.forward() * delta_time * 10.f;
		}

		if (sdl::is_key_down("J")) {
			control_me.position -= control_me.right() * delta_time * 10.f;
		}
		else if (sdl::is_key_down("L")) {
			control_me.position += control_me.right() * delta_time * 10.f;
		}

		if (data::capture_mouse && sdl::data::mouse_relative_x != 0 && sdl::data::mouse_relative_y != 0) {
			control_me.rotation.angle_x += delta_time * static_cast<float>(sdl::data::mouse_relative_x) * 10.f;
			control_me.rotation.angle_y -= delta_time * static_cast<float>(sdl::data::mouse_relative_y) * 10.f;
		}
	}

	void control_ship(world::model& control_me) {

		auto velocity = std::clamp(delta_time * abs(data::ship_velocity) * 10.f, 1.f, 15.f);

		if (sdl::is_mod_down(KMOD_LSHIFT)) {
			data::ship_velocity += delta_time * 10.f;
		}
		else if (sdl::is_mod_down(KMOD_LCTRL)) {
			data::ship_velocity -= delta_time * 10.f;
		}

		if (sdl::is_key_down("A")) {
			control_me.rotation.angle_y -= velocity;
		}
		else if (sdl::is_key_down("D")) {
			control_me.rotation.angle_y += velocity;
		}

		if (sdl::is_key_down("W")) {
			control_me.rotation.angle_x += velocity;
		}
		else if (sdl::is_key_down("S")) {
			control_me.rotation.angle_x -= velocity;
		}

		control_me.position += control_me.forward() * delta_time * data::ship_velocity;

		if (data::ship_velocity > 0.f) {
			data::ship_velocity -= delta_time * 2.f;
		}
		else if (data::ship_velocity < 0.f) {
			data::ship_velocity += delta_time * 2.f;
		}
		else if (data::ship_velocity > 100.f) {
			data::ship_velocity = 100.f;
		}
		else if (data::ship_velocity < -100.f) {
			data::ship_velocity = -100.f;
		}
	}

	// ============================================================================================================================

	void create_cube_locations(std::size_t amount, float minx, float maxx, float miny, float maxy, float minz, float maxz) {

		assert(maxx > minx);
		assert(maxy > miny);
		assert(maxz > minz);

		for (std::size_t i = 0; i < amount; i++)
		{
			float x = random::next(minx, maxx);
			float y = random::next(miny, maxy);
			float z = random::next(minz, maxz);

			float rx = 0.f;//random::next(0.f, 360.f);
			float ry = 0.f;//random::next(0.f, 360.f);
			float rz = 0.f;//random::next(0.f, 360.f);

			auto rotate_x = glm::rotate(glm::mat4(1.f), glm::radians(rx), glm::vec3(1.f, 0.f, 0.f));
			auto rotate_y = glm::rotate(rotate_x, glm::radians(ry), glm::vec3(0.f, 1.f, 0.f));
			auto rotate_z = glm::rotate(rotate_y, glm::radians(rz), glm::vec3(0.f, 0.f, 1.f));
			auto translate = glm::translate(rotate_z, glm::vec3(x, y, z));

			data::locations.push_back(translate);
		}

		data::instance_buffer = opengl::create_shader_storage_buffer(
			data::locations.size(),
			data::locations[0]
		);
	}

	// ============================================================================================================================

	void show_loc_rot_gui(world::model& for_model, bool& show) {

		if (show) {

			ImGui::Begin(for_model.path_to_file().c_str(), &show);

			ImGui::Text("Position");
			ImGui::DragScalarN("xyz", ImGuiDataType_Float, &for_model.position, 3, 0.1f, nullptr, nullptr, "%.1f");

			ImGui::Separator();

			ImGui::Text("Rotation");
			constexpr auto a_min = -360.f;
			constexpr auto a_max = -360.f;
			ImGui::DragScalarN("axyz", ImGuiDataType_Float, &for_model.rotation, 3, 0.1f, &a_min, &a_max, "%.1f deg");

			ImGui::Separator();

			ImGui::Columns(4);

			auto& up = for_model.up();
			auto& forward = for_model.forward();
			auto& right = for_model.right();

			ImGui::Text("name"); ImGui::NextColumn();
			ImGui::Text("x"); ImGui::NextColumn();
			ImGui::Text("y"); ImGui::NextColumn();
			ImGui::Text("z"); ImGui::NextColumn();

			ImGui::Separator();

			ImGui::Text("up"); ImGui::NextColumn();
			ImGui::Text("%.1f", up.x); ImGui::NextColumn();
			ImGui::Text("%.1f", up.y); ImGui::NextColumn();
			ImGui::Text("%.1f", up.z); ImGui::NextColumn();

			ImGui::Text("forward"); ImGui::NextColumn();
			ImGui::Text("%.1f", forward.x); ImGui::NextColumn();
			ImGui::Text("%.1f", forward.y); ImGui::NextColumn();
			ImGui::Text("%.1f", forward.z); ImGui::NextColumn();

			ImGui::Text("right"); ImGui::NextColumn();
			ImGui::Text("%.1f", right.x); ImGui::NextColumn();
			ImGui::Text("%.1f", right.y); ImGui::NextColumn();
			ImGui::Text("%.1f", right.z); ImGui::NextColumn();

			ImGui::Columns(1);
			if (ImGui::CollapsingHeader("meshes")) {
				ImGui::Text("%i total", for_model.meshes_size());
				
				std::stringstream ss("model_");
				ss << for_model.id() << "_" << for_model.path_to_file() << "_meshes";

				ImGui::BeginChild(ss.str().c_str(), ImVec2(0, 0), true);

				ImGui::Columns(3);

				ImGui::Text("name");
				ImGui::NextColumn();
				ImGui::Text("verts");
				ImGui::NextColumn();
				ImGui::Text("tris");
				ImGui::NextColumn();
				ImGui::Separator();

				for (const auto& mesh : for_model) {
					ImGui::Text(mesh.name().c_str());
					ImGui::NextColumn();
					ImGui::Text("%i", mesh.vertex_size());
					ImGui::NextColumn();
					ImGui::Text("%i", mesh.index_size());
					ImGui::NextColumn();
				}

				ImGui::EndChild();

				ImGui::Columns(1);
			}

			ImGui::End();
		}
	}

	void show_mat() {
		ImGui::Begin("Mat");

		for (std::size_t index = 0; index < 10llu; index++)
		{

			auto& mat = data::locations[index];

			ImGui::Text("Location %0", index);

			for (int i = 0; i < mat.length(); i++)
			{
				auto& column = mat[i];

				std::string name = "";
				name.append(std::to_string(index));
				name.append(" ");
				name.append(std::to_string(i));

				ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, &column, column.length(), 0.1f, nullptr, nullptr, "%.1f");
			}

			ImGui::Separator();
		}

		ImGui::End();
	}

	// ============================================================================================================================

	void on_init() {
		// load the shader to use for our models
		bool ship_shader_loaded = shader::load_shader(data::ship_shader_id, shader::basic_vert, shader::basic_frag);
		bool cube_shader_loaded = shader::load_shader(data::cube_shader_id, shader::basic_instance_vert, shader::basic_frag);

		// load a new model and set the shader to use
		world::load_model(data::ship_index, R"(assets\models\spaceship3.obj)", default_load_flags);
		world::model_set_shader(data::ship_index, data::ship_shader_id);

		create_cube_locations(
			150000llu,			// amount
			-1500.f, 1500.f,	// min, max x
			-1500.f, 1500.f,	// min, max y
			-1500.f, 1500.f		// min, max z
		);

		// load another model and set the shader to use
		world::load_model(data::cube_index, R"(assets\models\ico_low.obj)", default_load_flags);
		world::model_set_shader(data::cube_index, data::cube_shader_id);

		world::model& ship = world::model_get(data::ship_index);
		ship.position.z -= 5.f;
		ship.position.x += 1.f;

		// prepare the vertex data
		world::setup_model(data::ship_index);
		world::setup_model(data::cube_index);

		data::main_camera = follow_camera(ship);
	}

	void on_update() {

		if (sdl::is_key_down("Escape")) {
			global.should_quit(true);
		}

		if (sdl::is_key_up("F") && data::is_in_fullscreen) {
			data::is_in_fullscreen = false;
			sdl::disable_fullscreen();
		} else if (sdl::is_key_up("F") && !data::is_in_fullscreen) {
			data::is_in_fullscreen = true;
			sdl::enable_fullscreen();
		}

		if (sdl::is_key_up("M")) {
			data::capture_mouse = !data::capture_mouse;
			sdl::set_capture_mouse(data::capture_mouse);
		}

		world::model& ship = world::model_get(data::ship_index);
		ship.update_orientation();

		//control_camera(data::main_camera/*, ship*/);
		control_ship(ship);
	}

	void on_draw() {
		world::model& ship = world::model_get(data::ship_index);
		world::model& cubes = world::model_get(data::cube_index);

		// TODO: draw something...
		opengl::draw_instanced(data::main_camera, cubes, data::locations.size());
		opengl::draw(data::main_camera, ship);

		//gui::show_demo();
		//bool show_me = true;
		//gui::show_logs(show_me);
		show_loc_rot_gui(ship, data::show_ship_ui);

		
	}
}