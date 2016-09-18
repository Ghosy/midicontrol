#include <vector>
#include <fstream>
#include <sstream>

#include "settings.h"

config settings;

config::config() {
	config_file_path = "keyconf";
	midi_device = "";
}

void config::read() {
	std::ifstream f(config_file_path.c_str());
	std::string cline, name, value;
	
	if(!f.is_open())
		return;
	
	while(!f.eof()) {
		getline(f, cline);

		if(!cline.empty() && cline[0] != '#') {
			int pos;
			pos = cline.find("=");

			name = trim(cline.substr(0, pos));
			value = trim(cline.substr(pos + 1, cline.length() - pos));

			if(name == "device") {
				midi_device = value;
			}
			else {
				std::string temp;
				char delim = ',';

				// Break up note and place into vector
				std::vector<unsigned char> lows;
				std::vector<unsigned char> highs;
				std::stringstream ss(name);

				while(getline(ss, temp, delim)) {
					int breakpos = temp.find("..");
					// If string contains ".."
					if(breakpos != -1) {
						lows.push_back((unsigned char)stoi(temp.substr(0, breakpos)));
						highs.push_back((unsigned char)stoi(temp.substr(breakpos + 2)));
					}
					// If string doesn't contain ".."
					else {
						lows.push_back((unsigned char)stoi(temp));
						highs.push_back((unsigned char)stoi(temp));
					}
				}
				// Break up after =
				std::vector<std::string> entry_list;
				ss.clear();
				ss.str(value);
				while(getline(ss, temp, delim))
					entry_list.push_back(trim(temp));
				
				Entry new_entry;
				// If there are light settings
				if(entry_list.size() >= 2) {
					LightMode new_mode;
					unsigned char new_light_value;
					
					// Find correct light mode
					if(entry_list[1] == "light_on") {
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
					else {
						// Print error light_mode not valid
						// TODO: Add debug code
					}
					
					new_entry = Entry(lows, highs, entry_list[0], new_mode, new_light_value);
				}
				else {
					new_entry = Entry(lows, highs, entry_list[0]);
				}
				
				// Store entry
				note_list.insert(new_entry);
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

Entry::Entry() {
}

Entry::Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action) {
	for(int i = 0; i < 3; ++i) {
		min[i] = lows[i];
		max[i] = highs[i];
	}
	action = new_action;
}

Entry::Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action, LightMode new_mode, unsigned char new_light_value) {
	for(int i = 0; i < 3; ++i) {
		min[i] = lows[i];
		max[i] = highs[i];
	}
	
	action = new_action;
	light_mode = new_mode;
	light_value = new_light_value;
}

bool Entry::contains(const std::vector<unsigned char>& note) const {
	for(int i = 0; i < 3; ++i) {
		if(note[i] < min[i] || note[i] > max[i])
			return false;
	}
	return true;
}

bool Entry::operator<(const Entry& other) const {
	for(int i = 0; i < 3; ++i) {
		if(min[i] < other.min[i] && max[i] < other.max[i])
			return true;
	}
	return false;
}
