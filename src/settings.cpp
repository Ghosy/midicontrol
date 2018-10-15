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
#include <algorithm>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "entry.h"
#include "settings.h"

config settings;

config::config() {
	if(getenv("XDG_CACHE_HOME") == nullptr) {
		log_path = std::string(getenv("HOME")) + "/.cache/midicontrol.log";
	}
	else {
		log_path = std::string(getenv("XDG_CACHE_HOME")) + "/midicontrol.log";
	}

	config_file_path.push_back(std::string(getenv("HOME")) + "/.midicontrolrc");
	if(getenv("XDG_CONFIG_HOME") == nullptr) {
		config_file_path.push_back(std::string(getenv("HOME")) + "/.config/midicontrol/config");
	}
	else {
		config_file_path.push_back(std::string(getenv("XDG_CONFIG_HOME")) + "/midicontrol/config");
	}
	midi_device = "";
}

void config::read() {
	// Get logger
	logger = spdlog::get("multi_sink");

	std::string config_file;
	// Check all possible config paths
	for(const auto &path: config_file_path) {
		std::ifstream f(path.c_str());
		// Check current file
		if(f.good()) {
			config_file = path;
			break;
		}
		logger->debug("{} cannot be read", path);
	}
	YAML::Node config;

	logger->debug("Reading Config File: {}", config_file);
	
	// Ensure the file trying to be loaded exists before trying
	if(!config_file.empty()) {
		config = YAML::LoadFile(config_file);
	}
	else {
		logger->error("No valid configuration file found");
		exit(EXIT_FAILURE);
	}

	// If no valid devices in config
	if(!config["devices"].IsSequence() && !config["devices"]) {
		logger->error("No valid devices found in configuration file");
		exit(EXIT_FAILURE);
	}

	// Each device in config
	for(YAML::Node device: config["devices"]) {
		midi_device = device["device"].as<std::string>();

		// If no valid notes for device
		if(!device["notes"].IsSequence() && !device["notes"]) {
			logger->error("No valid notes found for {}", device["device"].as<std::string>());
			exit(EXIT_FAILURE);
		}

		// Each note for device
		for(YAML::Node note: device["notes"]) {
			Entry new_entry;

			// Break up note and place into vectors
			auto vals = config::read_note(note["note"].as<std::string>());
			std::vector<unsigned char> lows = vals.first;
			std::vector<unsigned char> highs = vals.second;

			std::string new_command = note["command"].as<std::string>();

			// If there are light settings
			if(note["light_mode"]) {
				LightMode new_mode = LightMode::LIGHT_OFF;
				unsigned int new_light_value = 0;
				std::string new_light_command;

				std::string temp_mode = note["light_mode"].as<std::string>();
				// To lower temp_mode
				transform(temp_mode.begin(), temp_mode.end(), temp_mode.begin(), ::tolower);

				// Find correct light mode
				if(temp_mode == "light_push") {
					new_mode = LightMode::LIGHT_PUSH;
					new_light_value = note["light_value"].as<unsigned int>();
				}
				else if(temp_mode == "light_on") {
					new_mode = LightMode::LIGHT_ON;
					new_light_value = note["light_value"].as<unsigned int>();
				}
				else if(temp_mode == "light_off") {
					new_mode = LightMode::LIGHT_OFF;
					new_light_value = 0;
				}
				else if(temp_mode == "light_wait") {
					new_mode = LightMode::LIGHT_WAIT;
					new_light_value = note["light_value"].as<unsigned int>();
				}
				else if(temp_mode == "light_check") {
					new_mode = LightMode::LIGHT_CHECK;
					new_light_value = note["light_value"].as<unsigned int>();
					new_light_command = note["light_command"].as<std::string>();
				}
				else if(temp_mode == "light_var") {
					new_mode = LightMode::LIGHT_VAR;
					new_light_value = 0;
					new_light_command = note["light_command"].as<std::string>();
				}
				else {
					// Print error light_mode not valid
					logger->warn("The light_mode is invalid for {}", format_note(lows, highs));
					logger->warn("{} is not a valid value for light_mode", note["light_mode"].as<std::string>());
				}
				// Warn if light value not in range
				if(new_light_value > 255) {
					logger->warn("The light_value is invalid for {}", format_note(lows, highs));
					logger->warn("{} is not a valid light value", new_light_value);
				}


				insert_note(lows, highs, new_command, new_mode, (unsigned char)new_light_value, new_light_command);
			}
			else {
				insert_note(lows, highs, new_command, LightMode::NONE, 0, "");
			}
		}
	}

	// Look through all entries for LIGHT_PUSH missing LIGHT_OFF
	// TODO: Try and clean this up. It's a mess
	for(auto it = note_list.begin(); it != note_list.end(); ++it) {
		if(it->light_mode == LightMode::LIGHT_PUSH) {
			// Try to find midi note off, aka xx,xx,00
			Entry off_entry1({it->note[0], it->note[1], 0}, "");
			// Try to find midi note off, aka xx-16,xx,xx
			Entry off_entry2({static_cast<unsigned char>(it->note[0] - 16), it->note[1], it->note[2]}, "");

			std::vector<Entry> off_entries = {off_entry1, off_entry2};

			for(const auto &off_entry: off_entries) {
				auto it_find = settings.note_list.find(off_entry);

				// If off note found
				if(it_find != settings.note_list.end()) {
					// Modify entry for found
					if(it_find->light_mode == LightMode::NONE) {
						// Create modified version of found
						Entry new_entry(it_find->note, it_find->action, LightMode::LIGHT_OFF, (unsigned char)0);
						// Remove found and insert modified
						note_list.erase(it_find);
						note_list.insert(new_entry);
					}
					else {
						// Print error light_mode used incorrectly
						logger->warn("{} has a light mode with a corresponding note on, which has light_push", it_find->get_note());
						logger->warn("{} should not have a light_mode, if using light_push on a corresponding note", it_find->get_note());
					}
				}
				else {
					Entry new_entry(off_entry.note, "", LightMode::LIGHT_OFF, (unsigned char)0);
					note_list.insert(new_entry);
				}
			}
		}
	}
}

void config::insert_note(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string action, LightMode mode, unsigned int light_value, std::string light_command) {
	Entry new_entry;
	for(unsigned char i = lows[0]; i <= highs[0]; ++i) {
		for(unsigned char j = lows[1]; j <= highs[1]; ++j) {
			for(unsigned char k = lows[2]; k <= highs[2]; ++k) {
				new_entry = Entry({i, j, k}, action, mode, (char)light_value, light_command);
				// Store entry
				note_list.insert(new_entry);
			}
		}
	}
}

std::string config::format_note(std::vector<unsigned char> lows, std::vector<unsigned char> highs) {
	std::ostringstream ostream;

	for(int i = 0; i < 3; ++i) {
		if(lows[i] == highs[i]) {
			ostream << (int)lows[i];
		}
		else {
			ostream << (int)lows[i] << ".." << (int)highs[i];
		}
		if(i < 2) {
			ostream << ",";
		}
	}
	return ostream.str();
}

void config::commandline_config(const char* conf_path) {
	// Remove default configuration paths
	config_file_path.clear();
	config_file_path.push_back(conf_path);
}

std::string config::get_device() {
	return midi_device;
}

std::string config::trim(std::string s) {
	size_t startpos = s.find_first_not_of(" \n\r\t");
	s = s.substr(startpos);
	size_t endpos = s.find_last_not_of(" \n\r\t");
	s = s.substr(0, endpos + 1);
	return s;
}

std::pair<std::vector<unsigned char>, std::vector<unsigned char>> config::read_note(const std::string &note) {
	std::vector<unsigned char> lows;
	std::vector<unsigned char> highs;
	std::string temp;
	char delim = ',';
	std::stringstream ss(note);

	while(getline(ss, temp, delim)) {
		int breakpos = temp.find("..");
		// If string contains ".."
		if(breakpos != -1) {
			int low = stoi_check(temp.substr(0, breakpos));
			int high = stoi_check(temp.substr(breakpos + 2));

			// Check for invalid values
			if(low > 255 || low < 0) {
				logger->warn("{} is not a valid value for a note", low);
			}
			if( high > 255 || high < 0) {
				logger->warn("{} is not a valid value for a note", high);
			}

			lows.push_back((unsigned char)low);
			highs.push_back((unsigned char)high);
		}
		// If string doesn't contain ".."
		else {
			int val = stoi_check(temp);

			// Check for invalid value
			if(val > 255 || val < 0) {
				logger->warn("{} is not a valid value for a note", val);
			}

			lows.push_back((unsigned char)val);
			highs.push_back((unsigned char)val);
		}
	}
	return make_pair(lows, highs);
}

unsigned int config::stoi_check(const std::string& s) {
	try {
		return std::stoi(s);
	}
	catch(std::invalid_argument& e) {
		logger->warn("{} is not a valid value", s);
		logger->warn("returning 0 instead");
	}
	catch(std::out_of_range& e) {
		logger->warn(" is not with range", s);
		logger->warn("returning 0 instead");
	}
	return 0;
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
