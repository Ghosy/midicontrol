/**
 * @file
 * @brief Header of settings class for placeholder
 * @author Zachary Matthews
 *
 * Copyright(c) 2017 Zachary Matthews.
 */
#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <set>

#include "entry.h"

struct config {
	config();

	void read();
	void commandline_config(const char*);
	std::set<Entry> note_list;
	std::string getDevice();

private:
	std::string config_file_path;
	std::string midi_device;
	std::string trim(std::string);
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>> read_note(const std::string);
};

extern config settings;

// I swear to god if these names aren't fix to be
// less confusing soon, someone will get hurt.
namespace prog_settings {
	extern bool quiet;
	extern bool silent;
	extern bool verbose;
	extern unsigned int delay;
}

#endif
