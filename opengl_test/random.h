#pragma once
#include <random>
#include <type_traits>
namespace random {
	// ============================================================================================================================

	namespace traits {
		template<typename T>
		struct is_valid_integeral
			: std::integral_constant<bool,
			std::is_same<T, short>::value ||
			std::is_same<T, int>::value ||
			std::is_same<T, long>::value ||
			std::is_same<T, long long>::value ||
			std::is_same<T, unsigned short>::value ||
			std::is_same<T, unsigned int>::value ||
			std::is_same<T, unsigned long>::value ||
			std::is_same<T, unsigned long long>::value
			>
		{ };

		template<typename T>
		struct is_valid_real
			: std::integral_constant<bool,
			std::is_same<T, float>::value ||
			std::is_same<T, double>::value ||
			std::is_same<T, long double>::value
			>
		{ };

		template<typename T>
		struct is_valid_type_for_random
			: std::integral_constant<bool,
			is_valid_integeral<T>::value ||
			is_valid_real<T>::value ||
			std::is_same<T, char>::value
			>
		{};
	}

	// ============================================================================================================================

	template<typename RandomEngine>
	struct random_impl {

		template<typename T, 
			typename std::enable_if_t<traits::is_valid_integeral<T>::value, int> = 0>
		T next(const T& min, const T& max) {
			return next_impl<T, std::uniform_int_distribution<T>>(min, max);
		}

		template<typename T,
			typename std::enable_if_t<traits::is_valid_real<T>::value, int> = 0>
		T next(const T& min, const T& max) {

			return next_impl<T, std::uniform_real_distribution<T>>(min, max);

		}

		template<typename T,
			typename std::enable_if_t<std::is_same<T, char>::value, int> = 0>
		T next(const T& min, const T& max) {

			return static_cast<T>(next(static_cast<int>(min), static_cast<int>(max)));
		}

		template<typename T, typename OutputIt,
			typename std::enable_if_t<traits::is_valid_integeral<T>::value, int> = 0>
		OutputIt range(const T& min, const T& max, std::size_t amount, OutputIt output) {

			return range_impl(min, max, amount, output);
		}

		template<typename T, typename OutputIt,
			typename std::enable_if_t<traits::is_valid_real<T>::value, int> = 0>
		OutputIt range(const T& min, const T& max, std::size_t amount, OutputIt output) {

			return range_impl(min, max, amount, output);
		}

		template<typename T, typename OutputIt,
			typename std::enable_if_t<std::is_same<T, char>::value, int> = 0>
		OutputIt range(const T& min, const T& max, std::size_t amount, OutputIt output) {

			return range_impl(min, max, amount, output);
		}

	private:

		template<typename T, typename Dist>
		T next_impl(const T& min, const T& max) {

			Dist dis(min, max);
			return dis(generator);
		}

		template<typename T, typename OutputIt>
		OutputIt range_impl(const T& min, const T& max, std::size_t amount, OutputIt output) {

			for (std::size_t i = 0; i < amount; i++) {
				*output++ = next(min, max);
			}

			return output;
		}

		std::random_device rd;
		RandomEngine generator{ rd() };
	};

	template<typename RandomEngine = std::mt19937, typename T, typename std::enable_if_t<traits::is_valid_type_for_random<T>::value, int> = 0>
	T next(const T& min, const T& max) {
		static random_impl<RandomEngine> rg;

		return rg.next(min, max);
	}

	template<typename RandomEngine = std::mt19937, typename T, typename OutputIt, typename std::enable_if_t<traits::is_valid_type_for_random<T>::value, int> = 0>
	OutputIt range(const T& min, const T& max, std::size_t amount, OutputIt output) {
		static random_impl<RandomEngine> rg;

		return rg.range(min, max, amount, output);
	}
}