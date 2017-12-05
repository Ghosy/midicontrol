/**
 * @file
 * @brief Implementation of settings class for placeholder
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "entry.h"
#include "settings.h"

config settings;

config::config() {
	config_file_path = "$HOME/.midicontrolrc";
	midi_device = "";
}

void config::read() {
	if(prog_settings::verbose) {
		std::cout << "Reading File: " << config_file_path << std::endl;
	}
	std::ifstream f(config_file_path.c_str());
	std::string line, name, value;

	if(!f.is_open())
		return;

	while(!f.eof()) {
		getline(f, line);

		if(!line.empty() && line[0] != '#') {
			int pos;
			pos = line.find("=");

			name = trim(line.substr(0, pos));
			value = trim(line.substr(pos + 1, line.length() - pos));

			if(name == "device") {
				midi_device = value;
			}
			else {
				std::string temp;
				char delim = ',';

				// Break up note and place into vectors
				auto vals = config::read_note(name);
				std::vector<unsigned char> lows = vals.first;
				std::vector<unsigned char> highs = vals.second;

				//TODO: Break this section into separate function for readability
				// Break up after =
				std::vector<std::string> entry_list;
				std::stringstream ss(value);
				while(getline(ss, temp, delim))
					entry_list.push_back(trim(temp));

				Entry new_entry;
				// If there are light settings
				if(entry_list.size() >= 2) {
					LightMode new_mode;
					unsigned char new_light_value;
					std::string new_light_command = "";

					// TODO: Program wraps to 255 value, but should this not add light data to entry?
					// TODO: Make compatible with light_var
					// Warn if light value not in range
					//int light_val = stoi(entry_list[2]);
					// if(light_val > 255 || light_val < 0) {
					// 	if(!prog_settings::silent) {
					// 		std::cerr << light_val << " is not a valid light value" << std::endl;
					// 	}
					// }
					// Find correct light mode
					if(entry_list[1] == "light_push") {
						new_mode = LightMode::LIGHT_PUSH;
						new_light_value = stoi(entry_list[2]);
					}
					else if(entry_list[1] == "light_on") {
						new_mode = LightMode::LIGHT_ON;
						new_light_value = stoi(entry_list[2]);
					}
					else if(entry_list[1] == "light_off") {
						new_mode = LightMode::LIGHT_OFF;
						new_light_value = 0;
					}
					else if(entry_list[1] == "light_wait") {
						new_mode = LightMode::LIGHT_WAIT;
						new_light_value = stoi(entry_list[2]);
					}
					else if(entry_list[1] == "light_check") {
						new_mode = LightMode::LIGHT_CHECK;
						new_light_value = stoi(entry_list[2]);
						new_light_command = entry_list[3];
					}
					else if(entry_list[1] == "light_var") {
						new_mode = LightMode::LIGHT_VAR;
						new_light_value = 0;
						new_light_command = entry_list[2];
					}
					else {
						// Print error light_mode not valid
						Entry err_entry(lows, highs, "");
						if(!prog_settings::silent) {
							std::cerr << entry_list[1] << " is not a valid value for light_mode" << std::endl;
							std::cerr << "light_mode is invalid for " << err_entry.get_note() << std::endl;
						}
					}

					new_entry = Entry(lows, highs, entry_list[0], new_mode, new_light_value, new_light_command);
				}
				else {
					new_entry = Entry(lows, highs, entry_list[0]);
				}

				// Store entry
				note_list.insert(new_entry);
			}
		}
	}

	// Look through all entries for LIGHT_PUSH missing LIGHT_OFF
	// TODO: Try and clean this up. It's a mess
	// TODO: Maybe reduce using range-based for loop
	for(auto it = note_list.begin(); it != note_list.end(); ++it) {
		if(it->light_mode == LightMode::LIGHT_PUSH) {
			// Sort through xx,00,00 of note
			for(unsigned char i = it->min[0]; i <= it->max[0]; ++i) {
				// Store value to begin range at to reduce new entries
				unsigned char next_ready = it->min[1];
				// Sort through 00,xx,00 of note
				for(unsigned char j = it->min[1]; j <= it->max[1]; ++j) {
					// Try to find midi note off, aka xx,xx,00
					Entry temp_entry({i,j,0}, {i,j,0}, "");
					std::set<Entry>::iterator it_find = settings.note_list.find(temp_entry);

					if(it_find != settings.note_list.end()) {
						// Create entry for LIGHT_OFF for range before found
						if(next_ready < j) {
							std::vector<unsigned char> low{i, next_ready, 0};
							// TODO: Static cast is done here to avoid warning. Is there a better way?
							std::vector<unsigned char> high{i, static_cast<unsigned char>(j - 1), 0};
							Entry new_entry(low, high, "", LightMode::LIGHT_OFF, 0);
							note_list.insert(new_entry);
						}
						// Modify entry for found
						if(it_find->light_mode == LightMode::NONE) {
							// Create modified version of found
							std::vector<unsigned char> low;
							for(unsigned char a : it->min)
								low.push_back(a);
							std::vector<unsigned char> high;
							for(unsigned char a : it->max)
								high.push_back(a);
							Entry new_entry(low, high, it->action, LightMode::LIGHT_OFF, it->light_value);
							// Remove found and insert modified
							note_list.erase(it_find);
							note_list.insert(new_entry);
						}
						else {
							// Print error light_mode used incorrectly
							// Use light_on instead of light_push
							// TODO: Make more verbose
							if(prog_settings::silent)
								std::cerr << "Light_mode set incorrectly, don't use a light_mode on note off of note using light_push" << std::endl;
						}

						next_ready = j + 1;
					}
				}
				if(next_ready == it->min[1]) {
					std::vector<unsigned char> low{i, next_ready, 0};
					std::vector<unsigned char> high{i, it->max[1], 0};
					Entry new_entry(low, high, "", LightMode::LIGHT_OFF, 0);
					note_list.insert(new_entry);
				}
			}
		}
	}
}

void config::commandline_config(const char* conf_path) {
	config_file_path = conf_path;
}

std::string config::getDevice() {
	return midi_device;
}

std::string config::trim(std::string s) {
	size_t startpos = s.find_first_not_of(" \n\r\t");
	s = s.substr(startpos);
	size_t endpos = s.find_last_not_of(" \n\r\t");
	s = s.substr(0, endpos + 1);
	return s;
}

std::pair<std::vector<unsigned char>, std::vector<unsigned char>> config::read_note(const std::string note) {
	std::vector<unsigned char> lows;
	std::vector<unsigned char> highs;
	std::string temp;
	char delim = ',';
	std::stringstream ss(note);

	while(getline(ss, temp, delim)) {
		int breakpos = temp.find("..");
		// If string contains ".."
		if(breakpos != -1) {
			int low = stoi(temp.substr(0, breakpos));
			int high = stoi(temp.substr(breakpos + 2));

			if(prog_settings::silent) {
				// Check for invalid values
				if(low > 255 || low < 0) {
					std::cerr << low << " is not a valid value for a note" << std::endl;
				}
				if( high > 255 || high < 0) {
					std::cerr << high << " is not a valid value for a note" << std::endl;
				}
			}

			lows.push_back((unsigned char)low);
			highs.push_back((unsigned char)high);
		}
		// If string doesn't contain ".."
		else {
			int val = stoi(temp);
			if(prog_settings::silent) {
				// Check for invalid value
				if(val > 255 || val < 0) {
					std::cerr << val << " is not a valid value for a note" << std::endl;
				}
			}

			lows.push_back((unsigned char)val);
			highs.push_back((unsigned char)val);
		}
	}
	return make_pair(lows, highs);
}

// I swear to god if these names aren't fix to be
// less confusing soon, someone will get hurt.
namespace prog_settings {
	bool quiet = false;
	bool silent = false;
	bool verbose = false;
	unsigned int delay = 50;
}
/* vim: set ts=8 sw=8 tw=0 noet :*/
