#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "world.h"

struct camera {
#undef far
#undef near

	glm::vec3 position{};
	world::rotation_values rotation{};
	
	float fov_degrees = 90.f;
	float near = 0.1f;
	float far = 1000.f;

	camera() {
		rotation.angle_y = -90.f;
	}

	virtual const glm::mat4& projection(float aspect_ratio) {

		if (fov_degrees != last_fov_degrees
			|| near != last_near
			|| far != last_far
			|| aspect_ratio != last_aspect_ratio) {


			last_projection_ = glm::perspective(glm::radians(fov_degrees), aspect_ratio, near, far);
			camera_projection_updates_++;
			
		}

		last_fov_degrees = fov_degrees;
		last_near = near;
		last_far = far;
		last_aspect_ratio = aspect_ratio;

		return last_projection_;
	}

	virtual const glm::mat4& view() {

		if (position != last_position
			|| rotation != last_rotation) {

			forward_.x = cos(glm::radians(rotation.angle_y)) * cos(glm::radians(rotation.angle_x));
			forward_.y = sin(glm::radians(rotation.angle_x));
			forward_.z = sin(glm::radians(rotation.angle_y)) * cos(glm::radians(rotation.angle_x));

			forward_ = glm::normalize(forward_);

			right_ = glm::cross(forward_, up);
			right_ = glm::normalize(right_);

			last_view = glm::lookAt(position, position + forward_, up);
			camera_view_updates_++;
		}

		last_position = position;
		last_rotation = rotation;

		return last_view;
	}

	const glm::vec3& forward() const {
		return forward_;
	}

	const glm::vec3& right() const {
		return right_;
	}

	const std::size_t& camera_projection_updates() const {
		return camera_projection_updates_;
	}

	const std::size_t& camera_view_updates() const {
		return camera_view_updates_;
	}

protected:
	glm::vec3 up{ 0.f, 1.f, 0.f };
	glm::vec3 forward_{ 0.f, 0.f, -1.f };
	glm::vec3 right_{ -1.f, 0.f, 0.f };

	float last_fov_degrees{}, last_near{}, last_far{}, last_aspect_ratio{};
	glm::mat4 last_projection_{ 1.f };

	glm::mat4 last_view{ 1.f };
	world::rotation_values last_rotation{};
	glm::vec3 last_position{};

	std::size_t camera_view_updates_{};
	std::size_t camera_projection_updates_{};
};

struct follow_camera : camera {

	follow_camera() = default;

	follow_camera(const world::model& follow_me)
		: model_ptr(&follow_me)
	{

	}

	const glm::mat4& view() override {

		if (model_ptr != nullptr) {

			const world::model& model = *model_ptr;

			if (model.position != last_position
				|| model.rotation != last_rotation) {

				auto pos = model.position - model.forward() * 10.f;

				last_view = glm::lookAt(pos, model.position, model.up());
				camera_view_updates_++;
			}


			last_position = model.position;
			last_rotation = model.rotation;
		}

		return last_view;
	}

private:
	const world::model* model_ptr = nullptr;
};