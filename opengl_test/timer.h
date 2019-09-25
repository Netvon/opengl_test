#pragma once
#include <chrono>
#include <mutex>
#include <shared_mutex>

namespace timer {
	using time_point = std::chrono::time_point<std::chrono::system_clock>;
	using duration = std::chrono::duration<float>;

	struct timer_impl {

		~timer_impl() = default;
		timer_impl(timer_impl&&) = default;
		timer_impl(const timer_impl&) = default;
		timer_impl() 
			:	start_( std::chrono::system_clock::now() ),
				end_( std::chrono::system_clock::now() ) { }

		timer_impl& operator=(timer_impl&&) = default;
		timer_impl& operator=(const timer_impl&) = default;

		void update() {
			std::unique_lock lock(mutex_);

			end_ = std::chrono::system_clock::now();
			elapsed_time_ = end_ - start_;
			start_ = end_;

			total_ += elapsed_time_.count();
			second_counter_ += elapsed_time_.count();
		}

		float elapsed_time() const {
			std::shared_lock lock(mutex_);

			return elapsed_time_.count();
		}

		float per_second() const {
			std::shared_lock lock(mutex_);

			return 1.f / elapsed_time();
		}

		const float& total() const {
			std::shared_lock lock(mutex_);

			return total_;
		}

	private:
		time_point start_;
		time_point end_;

		duration elapsed_time_;

		float total_ = 0.f;
		float second_counter_ = 1.f;

		mutable std::shared_mutex mutex_;
	};

	/*struct metronome {
		metronome(const timer_impl& timer, float interval) 
			: timer_(&timer), interval_(interval) {}

		void update() {
			if (timer_ != nullptr) {
				at_interval = false;

				counter_ += timer_->elapsed_time();

				if (counter_ >= interval_) {
					at_interval = true;
					counter_ = 0.f;
				}
			}
			else {
				at_interval = false;
			}
		}

		bool is_at_interval() const {
			return at_interval;
		}

	private:
		const timer_impl * timer_ = nullptr;

		float counter_{};
		float interval_{};

		bool at_interval = false;
	};*/

	timer_impl create() {
		return timer_impl();
	}
}