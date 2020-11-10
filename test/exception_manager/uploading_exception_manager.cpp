#include "doctest/doctest.h"
#include "exception_manager.hpp"

#include <random>
#include <vector>
static std::mt19937_64 rgen(std::random_device{}());
static std::bernoulli_distribution bd(0.5);

struct mock_server_connection_type {
	static std::string expected_message;
	static bool should_be_called;
	static bool was_called;
	static bool active;
	void operator()(std::string const &message) {
		was_called = true;
		if (active)
			CHECK(expected_message == message);
	};
};
std::string mock_server_connection_type::expected_message = "";
bool mock_server_connection_type::should_be_called = false;
bool mock_server_connection_type::was_called = false;
bool mock_server_connection_type::active = false;

template <typename manager_t, typename value_t>
inline void check_push(manager_t &manager, value_t &&value,
					   bool should_be_called = false, std::string const &expected_message = "") {
	mock_server_connection_type::expected_message = expected_message;
	mock_server_connection_type::should_be_called = should_be_called;
	mock_server_connection_type::was_called = false;
	mock_server_connection_type::active = true;
	manager.push(std::move(value));
	CHECK(mock_server_connection_type::should_be_called == mock_server_connection_type::was_called);
	mock_server_connection_type::active = false;
}
struct comparison_visitor {
	bool operator()(bool lhs, bool rhs) { return lhs == rhs; }
	bool operator()(int lhs, int rhs) { return lhs == rhs; }
	template <typename lhs, typename rhs> bool operator()(lhs, rhs) { REQUIRE(false); return false; }
};

TEST_CASE("Trivially typed uploading exception manager") {
	qal::uploading_exception_manager<mock_server_connection_type, int, bool, std::string> manager;

	manager.update_exception_level<int>(true);
	manager.update_exception_level<bool>(false);
	manager.update_exception_level<std::string>(true);

	SUBCASE("qal::exception_manager<...>::push()") {
		check_push(manager, 5, true, "A new critical exception: 5\n");
		check_push(manager, true, false, "A new critical exception: true\n");
		check_push(manager, -11, true, "A new critical exception: -11\n");
		check_push(manager, 4, true, "A new critical exception: 4\n");
	}
	SUBCASE("qal::exception_manager<...>::check<>()") {
		CHECK_EQ(manager.check<int>(), true);
		CHECK_EQ(manager.check<bool>(), false);
		CHECK_EQ(manager.check<std::string>(), true);

		CHECK_EQ(manager.check<decltype(5)>(), true);
		CHECK_EQ(manager.check<decltype(-11)>(), true);
		CHECK_EQ(manager.check<decltype(false)>(), false);
	}
	SUBCASE("qal::exception_manager<...>::operator*()") {
		constexpr size_t iteration_count = 100;
		std::vector<std::variant<bool, int>> data;
		data.reserve(iteration_count);
		for (size_t i = 0; i < iteration_count; i++)
			if (bd(rgen)) {
				auto value = int(i);
				data.emplace_back(value);
				manager(std::move(value));
			} else {
				bool value = bd(rgen);
				data.emplace_back(value);
				manager(std::move(value));
			}
		
		size_t counter = 0;
		REQUIRE_EQ(data.size(), manager->size());
		for (auto const &exception : *manager)
			CHECK(std::visit(comparison_visitor{}, data[counter++], exception));
	}
}

class custom_exception : public std::exception {};
class critical_exception : public custom_exception {};
class major_exception : public custom_exception {};
class minor_exception : public custom_exception {};
class negligible_exception : public custom_exception {};

class serializable_major_exception : public major_exception {};
inline std::ostream &operator<<(std::ostream &stream, serializable_major_exception const &) {
	return stream << "major exception #42";
}
TEST_CASE("Polymorhpically typed exception manager") {
	qal::uploading_exception_manager<mock_server_connection_type,
		custom_exception, critical_exception,
		major_exception, minor_exception,
		negligible_exception,
		serializable_major_exception
	> manager;

	manager.update_exception_level<critical_exception>(true);
	manager.update_exception_level<major_exception>(true);
	manager.update_exception_level<minor_exception>(true);
	manager.update_exception_level<negligible_exception>(false);
	manager.update_exception_level<serializable_major_exception>(true);

	SUBCASE("qal::exception_manager<...>::push()") {
		check_push(manager, negligible_exception{}, false, "A new negligible exception, which cannot be serialized\n");
		check_push(manager, minor_exception{}, true, "A new critical exception, which cannot be serialized\n");
		check_push(manager, minor_exception{}, true, "A new critical exception, which cannot be serialized\n");
		check_push(manager, critical_exception{}, true, "A new critical exception, which cannot be serialized\n");
		check_push(manager, major_exception{}, true, "A new critical exception, which cannot be serialized\n");
		check_push(manager, serializable_major_exception{}, true, "A new critical exception: major exception #42\n");
	}
	SUBCASE("qal::exception_manager<...>::check<>()") {
		CHECK_EQ(manager.check<critical_exception>(), true);
		CHECK_EQ(manager.check<major_exception>(), true);
		CHECK_EQ(manager.check<minor_exception>(), true);
		CHECK_EQ(manager.check<negligible_exception>(), false);
	}
}