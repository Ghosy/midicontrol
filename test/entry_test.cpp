#include<catch2/catch.hpp>
#include "../src/entry.h"

TEST_CASE("Entry") {
	Entry test_entry({144,1,127}, "echo \"test\"", LightMode::LIGHT_WAIT, 60, "sleep 5");
	std::vector<unsigned char> test = {144,1,127};

	REQUIRE(test_entry.note == test);
	REQUIRE(test_entry.action == "echo \"test\"");
	REQUIRE(test_entry.light_mode == LightMode::LIGHT_WAIT);
	REQUIRE(test_entry.light_value == 60);
	REQUIRE(test_entry.light_command == "sleep 5");

	REQUIRE(test_entry.get_note() == "144,1,127");

	Entry comp_entry({144,1,127}, "");
	REQUIRE(test_entry == comp_entry);

	comp_entry = Entry({144,0,127}, "");
	REQUIRE(comp_entry < test_entry);
}
