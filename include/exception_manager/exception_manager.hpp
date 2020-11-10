#pragma once
#include "exception_manager_interface.hpp"

#include <concepts>
#include <functional>
#include <sstream>

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

	namespace detail {
		template <typename T>
		concept Serializable = requires (std::ostream & stream, T const &value) {
			{ stream << value } -> std::convertible_to<std::ostream &>;
		};
		template <typename T>
		concept Not_Serializable = !Serializable<T>;

		template <typename server_connection_type>
		struct server_callable {
			template <Serializable exception_type>
			void operator()(exception_type const &exception, bool severity)
				requires (std::is_invocable<server_connection_type, std::string>::value) {
				std::ostringstream message;
				message << "A new " << (severity ? "critical" : "negligible") 
					<< " exception: " << exception << '\n';
				if (severity)
					std::invoke(server_connection_type{}, message.str());
			}
			template <Not_Serializable exception_type>
			void operator()(exception_type const &, bool severity)
				requires (std::is_invocable<server_connection_type, std::string>::value) {
				std::ostringstream message;
				message << "A new " << (severity ? "critical" : "negligible") 
					<< " exception, which cannot be serialized\n";
				if (severity)
					std::invoke(server_connection_type{}, message.str());
			}
		};
	}
	template <typename server_connection_type, typename ...supported_exception_types>
	using uploading_exception_manager = callable_exception_manager<
		detail::server_callable<server_connection_type>,
		supported_exception_types...
	>;
}