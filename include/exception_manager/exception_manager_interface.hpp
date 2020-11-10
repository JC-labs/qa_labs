#pragma once
#include <array>
#include <list>
#include <mutex>
#include <ostream>
#include <shared_mutex>
#include <variant>

namespace qal {
	template <typename crtp_type, typename level_type, typename ...supported_exception_types>
	class exception_manager_interface {
	public:
		inline explicit exception_manager_interface() : levels{ level_type(0) } {}
		virtual ~exception_manager_interface() {}

		template <typename exception_type> void update_exception_level(level_type const &level)
			requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value);
		template <typename exception_type> level_type const &check() const
			requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value);
		template <typename exception_type> level_type const &push(exception_type &&exception)
			requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value);
		template <typename exception_type> level_type const &operator()(exception_type &&exception)
			requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value) {
			return push(std::forward<exception_type>(exception));
		}
		inline std::list<std::variant<supported_exception_types...>> const &operator*() const { return queue; }
		inline std::list<std::variant<supported_exception_types...>> const *operator->() const { return &queue; }
		auto pop();

	protected:
		std::array<level_type, sizeof...(supported_exception_types)> levels;
		mutable std::shared_mutex level_mutex;

		std::list<std::variant<supported_exception_types...>> queue;
	};

	namespace detail {
		template <typename T, typename... Ts>
		struct parameter_pack_index;
		template <typename T, typename... Ts>
		struct parameter_pack_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};
		template <typename T, typename U, typename... Ts>
		struct parameter_pack_index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + parameter_pack_index<T, Ts...>::value> {};
	}
}

template<typename crtp_type, typename level_type, typename ...supported_exception_types>
template<typename exception_type>
inline void qal::exception_manager_interface<crtp_type, level_type, supported_exception_types...>::update_exception_level(level_type const &level)
requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value) {
	std::scoped_lock _(level_mutex);
	levels[detail::parameter_pack_index<exception_type, supported_exception_types...>::value] = level;
}
template<typename crtp_type, typename level_type, typename ...supported_exception_types>
template<typename exception_type>
inline level_type const &qal::exception_manager_interface<crtp_type, level_type, supported_exception_types...>::check() const
requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value) {
	std::shared_lock _(level_mutex);
	return levels[detail::parameter_pack_index<exception_type, supported_exception_types...>::value];
}
template<typename crtp_type, typename level_type, typename ...supported_exception_types>
template<typename exception_type>
inline level_type const &qal::exception_manager_interface<crtp_type, level_type, supported_exception_types...>::push(exception_type &&exception)
requires (std::disjunction<std::is_same<exception_type, supported_exception_types>...>::value) {
	auto const &level = check<exception_type>();
	static_cast<crtp_type *>(this)->on_push(exception, level);
	queue.emplace_back(exception);
	return level;
}
template<typename crtp_type, typename level_type, typename ...supported_exception_types>
inline auto qal::exception_manager_interface<crtp_type, level_type, supported_exception_types...>::pop() {
	auto out = queue.front();
	queue.pop_front();
	return out;
}

namespace qal {
	template <typename crtp_type, typename ...supported_exception_types>
	using two_level_exception_manager_interface = exception_manager_interface<crtp_type, bool, supported_exception_types...>;

	enum class exception_level : size_t {
		negligible = 0, minor = 1,
		major = 2, critical = 3,
		LAST = critical
	};
	template <typename crtp_type, typename ...supported_exception_types>
	using four_level_exception_manager_interface = exception_manager_interface<crtp_type, exception_level, supported_exception_types...>;

	template <typename crtp_type, typename ...supported_exception_types>
	using int_level_exception_manager_interface = exception_manager_interface<crtp_type, size_t, supported_exception_types...>;
}
inline std::ostream &operator<<(std::ostream &stream, qal::exception_level level) {
	switch (level) {
		case qal::exception_level::critical: return stream << "critical";
		case qal::exception_level::major: return stream << "major";
		case qal::exception_level::minor: return stream << "minor";
		case qal::exception_level::negligible: return stream << "negligible";
		default: return stream << "unsupported";
	}
}

namespace qal {
	template <typename on_push_callable_type, typename ...supported_exception_types>
	class callable_exception_manager : public two_level_exception_manager_interface<callable_exception_manager<on_push_callable_type, supported_exception_types...>, supported_exception_types...> {
		friend two_level_exception_manager_interface<callable_exception_manager<on_push_callable_type, supported_exception_types...>, supported_exception_types...>;
	public:
		using two_level_exception_manager_interface<callable_exception_manager<on_push_callable_type, supported_exception_types...>, supported_exception_types...>::two_level_exception_manager_interface;
		inline explicit callable_exception_manager(on_push_callable_type on_push_callable)
			: two_level_exception_manager_interface<callable_exception_manager<on_push_callable_type, supported_exception_types...>, supported_exception_types...>()
			, on_push_callable(on_push_callable) {}

	protected:
		template <typename exception_type>
		void on_push(exception_type const &exception, bool const &severity) {
			on_push_callable(exception, severity);
		}

	protected:
		on_push_callable_type on_push_callable;
	};
}