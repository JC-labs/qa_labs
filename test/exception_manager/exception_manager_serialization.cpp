#include "doctest/doctest.h"
#include "exception_manager.hpp"

TEST_CASE("Trivially typed exception manager serialization") {
	qal::counting_exception_manager<int, bool, std::string> manager;

	manager.update_exception_level<int>(qal::exception_level::critical);
	manager.update_exception_level<bool>(qal::exception_level::major);
	manager.update_exception_level<std::string>(qal::exception_level::major);

	constexpr std::string_view filename = "exception_levels_trivial.txt";
	REQUIRE(manager.serialize(filename));

	decltype(manager) manager_copy;
	REQUIRE(manager_copy.deserialize(filename));

	return;
}

TEST_CASE("Polymorphically typed exception manager serialization") {
	class custom_exception : public std::exception {};
	class critical_exception : public custom_exception {};
	class major_exception : public custom_exception {};
	class minor_exception : public custom_exception {};
	class negligible_exception : public custom_exception {};

	qal::counting_exception_manager<
		custom_exception, critical_exception,
		major_exception, minor_exception,
		negligible_exception
	> manager;

	manager.update_exception_level<critical_exception>(qal::exception_level::critical);
	manager.update_exception_level<major_exception>(qal::exception_level::major);
	manager.update_exception_level<minor_exception>(qal::exception_level::minor);
	manager.update_exception_level<negligible_exception>(qal::exception_level::negligible);

	constexpr std::string_view filename = "exception_levels_poly.txt";
	REQUIRE(manager.serialize(filename));

	decltype(manager) manager_copy;
	REQUIRE(manager_copy.deserialize(filename));

	return;
}

TEST_CASE("Trivially typed exception manager serialization fail") {
	qal::counting_exception_manager<int, bool, std::string> manager;

	manager.update_exception_level<int>(qal::exception_level::critical);
	manager.update_exception_level<bool>(qal::exception_level::major);
	manager.update_exception_level<std::string>(qal::exception_level::major);

	constexpr std::string_view filename = "exception_levels_fail.txt";
	REQUIRE(manager.serialize(filename));

	qal::counting_exception_manager<int, bool, std::string, float> bigger_manager;
	REQUIRE_FALSE(bigger_manager.deserialize(filename));

	qal::counting_exception_manager<int, bool> smaller_manager;
	REQUIRE_FALSE(smaller_manager.deserialize(filename));

	qal::counting_exception_manager<int, std::string, bool> different_manager;
	REQUIRE_FALSE(different_manager.deserialize(filename));

	return;
}