#include<catch2/catch.hpp>
#include<iostream>
#include "../src/entry.h"
#include "../src/settings.h"

TEST_CASE("Settings") {
	// TODO: This assumes the context the bin is run from and shouldn't
	settings.commandline_config("tests/conf/testconfig");
	settings.read();

	REQUIRE(settings.get_device() == "Test:Test MIDI 1");

	Entry e({144,1,127}, "");
	std::set<Entry>::iterator it = settings.note_list.find(e);

	// Ensure note was found
	REQUIRE(it != settings.note_list.end());

	// Test modified off of light_push entry
	e = Entry({144,1,0}, "");
	it = settings.note_list.find(e);

	REQUIRE(it != settings.note_list.end());
	REQUIRE(it->action == "test2");
	REQUIRE(it->light_mode == LightMode::LIGHT_OFF);

	// Test created off of light_push entry
	// 0 velocity off
	e = Entry({144,0,0}, "");
	it = settings.note_list.find(e);

	REQUIRE(it != settings.note_list.end());

	// Test created off of light_push entry
	// Midi note off
	e = Entry({128,0,127}, "");
	it = settings.note_list.find(e);

	REQUIRE(it != settings.note_list.end());
}
