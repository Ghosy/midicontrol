/**
 * @file
 * @brief Header of settings class for placeholder
 * @author Zachary Matthews
 *
 * Copyright(c) 2017 Zachary Matthews.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
/* vim: set ts=8 sw=8 tw=0 noet :*/
