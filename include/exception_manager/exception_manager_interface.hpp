#pragma once
#include <array>
#include <filesystem>
#include <fstream>
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

		bool serialize(std::filesystem::path const &location);
		bool deserialize(std::filesystem::path const &location);

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

template<typename crtp_type, typename level_type, typename ...supported_exception_types>
inline bool qal::exception_manager_interface<crtp_type, level_type, supported_exception_types...>::serialize(std::filesystem::path const &location) {
	std::filesystem::create_directories(location.parent_path());
	std::ofstream stream(location);
	if (!stream)
		return false;

	stream << typeid(*this).name() << '\n';
	stream << typeid(*this).hash_code() << '\n';
	{
		std::shared_lock _(level_mutex);
		for (auto const &level : levels)
			stream << level << ' ';
	}
	return true;
}

template<typename crtp_type, typename level_type, typename ...supported_exception_types>
inline bool qal::exception_manager_interface<crtp_type, level_type, supported_exception_types...>::deserialize(std::filesystem::path const &location) {
	std::ifstream stream(location);
	if (!stream)
		return false;

	std::string temp;
	std::getline(stream, temp);
	if (temp != typeid(*this).name())
		return false;
	std::getline(stream, temp);
	if (temp != std::to_string(typeid(*this).hash_code()))
		return false;
	{
		std::shared_lock _(level_mutex);
		for (auto &level : levels)
			stream >> level;
	}
	return true;
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
inline std::ostream &operator<<(std::ostream &stream, qal::exception_level const &level) {
	switch (level) {
		case qal::exception_level::critical: return stream << "critical";
		case qal::exception_level::major: return stream << "major";
		case qal::exception_level::minor: return stream << "minor";
		case qal::exception_level::negligible: return stream << "negligible";
		default: return stream << "unsupported";
	}
}
inline std::istream &operator>>(std::istream &stream, qal::exception_level &level) {
	std::string temp;
	stream >> temp;
	if (temp == "critical")
		level = qal::exception_level::critical;
	else if (temp == "major")
		level = qal::exception_level::major;
	else if (temp == "minor")
		level = qal::exception_level::minor;
	else if (temp == "negligible")
		level = qal::exception_level::negligible;
	else {
		// error!
	}
	return stream;
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