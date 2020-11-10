#pragma once
#include "exception_manager_interface.hpp"

namespace qal {
	template <typename ...supported_exception_types>
	class counting_exception_manager : public four_level_exception_manager_interface<counting_exception_manager<supported_exception_types...>, supported_exception_types...> {
		friend four_level_exception_manager_interface<counting_exception_manager<supported_exception_types...>, supported_exception_types...>;
	public:
		using four_level_exception_manager_interface<counting_exception_manager<supported_exception_types...>, supported_exception_types...>::four_level_exception_manager_interface;
		inline explicit counting_exception_manager()
			: four_level_exception_manager_interface<counting_exception_manager<supported_exception_types...>, supported_exception_types...>()
			, counters{ 0u } {}

		size_t get_counter(exception_level level) const {
			std::shared_lock _(counter_mutex);
			return counters[size_t(level)];
		}
		inline size_t operator[](exception_level level) const { return get_counter(level); }

	protected:
		template <typename exception_type>
		void on_push(exception_type const &, exception_level const &level) {
			std::scoped_lock _(counter_mutex);
			++counters[size_t(level)];
		}

	protected:
		std::array<size_t, size_t(exception_level::LAST) + 1> counters;
		mutable std::shared_mutex counter_mutex;
	};
}