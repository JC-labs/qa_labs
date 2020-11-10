#include "doctest/doctest.h"
#include "exception_manager.hpp"

#include <random>
static std::mt19937_64 rgen(std::random_device{}());
static std::bernoulli_distribution bd(0.5);

template <typename manager_t>
inline void check_counters(manager_t const &manager,
						   size_t critical, size_t major,
						   size_t minor, size_t negligible,
						   size_t exception_count) {
	CHECK_EQ(manager[qal::exception_level::critical], critical);
	CHECK_EQ(manager[qal::exception_level::major], major);
	CHECK_EQ(manager[qal::exception_level::minor], minor);
	CHECK_EQ(manager[qal::exception_level::negligible], negligible);
	CHECK_EQ(manager->size(), exception_count);
}

struct comparison_visitor {
	bool operator()(bool lhs, bool rhs) { return lhs == rhs; }
	bool operator()(int lhs, int rhs) { return lhs == rhs; }
	template <typename lhs, typename rhs> bool operator()(lhs, rhs) { REQUIRE(false); return false; }
};

TEST_CASE("Trivially typed exception manager") {
	qal::counting_exception_manager<int, bool, std::string> manager;

	manager.update_exception_level<int>(qal::exception_level::critical);
	manager.update_exception_level<bool>(qal::exception_level::major);
	manager.update_exception_level<std::string>(qal::exception_level::major);

	SUBCASE("qal::exception_manager<...>::push()") {
		check_counters(manager, 0, 0, 0, 0, 0);

		manager.push(5);
		check_counters(manager, 1, 0, 0, 0, 1);

		manager(true);
		check_counters(manager, 1, 1, 0, 0, 2);

		manager(-11);
		check_counters(manager, 2, 1, 0, 0, 3);

		manager.push(4);
		check_counters(manager, 3, 1, 0, 0, 4);
	}
	SUBCASE("qal::exception_manager<...>::check<>()") {
		CHECK_EQ(manager.check<int>(), qal::exception_level::critical);
		CHECK_EQ(manager.check<bool>(), qal::exception_level::major);
		CHECK_EQ(manager.check<std::string>(), qal::exception_level::major);

		CHECK_EQ(manager.check<decltype(5)>(), qal::exception_level::critical);
		CHECK_EQ(manager.check<decltype(-11)>(), qal::exception_level::critical);
		CHECK_EQ(manager.check<decltype(false)>(), qal::exception_level::major);
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


TEST_CASE("Polymorhpically typed exception manager") {
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

	SUBCASE("qal::exception_manager<...>::push()") {
		check_counters(manager, 0, 0, 0, 0, 0);

		manager.push(negligible_exception{});
		check_counters(manager, 0, 0, 0, 1, 1);

		manager(minor_exception{});
		check_counters(manager, 0, 0, 1, 1, 2);

		manager(minor_exception{});
		check_counters(manager, 0, 0, 2, 1, 3);

		manager.push(critical_exception{});
		check_counters(manager, 1, 0, 2, 1, 4);

		manager.push(major_exception{});
		check_counters(manager, 1, 1, 2, 1, 5);
	}
	SUBCASE("qal::exception_manager<...>::check<>()") {
		CHECK_EQ(manager.check<critical_exception>(), qal::exception_level::critical);
		CHECK_EQ(manager.check<major_exception>(), qal::exception_level::major);
		CHECK_EQ(manager.check<minor_exception>(), qal::exception_level::minor);
		CHECK_EQ(manager.check<negligible_exception>(), qal::exception_level::negligible);
	}
}