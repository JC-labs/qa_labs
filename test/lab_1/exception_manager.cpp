#include "doctest/doctest.h"
#include "exception_manager.hpp"

TEST_CASE("Successful test") {
	CHECK(qal::hello());
}

TEST_CASE("Unsuccessful test") {
	WARN(!qal::hello());
}