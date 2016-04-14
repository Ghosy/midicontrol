#include <vector>
#include <fstream>
#include <sstream>

#include <iostream>

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
					Note tempnote;
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

				Note midi_note(lows, highs);
				note_list[midi_note] = entry_list;
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

Note::Note(std::vector<unsigned char> lows, std::vector<unsigned char> highs) {
	for(int i = 0; i < 3; ++i) {
		min[i] = lows[i];
		max[i] = highs[i];
	}
}

Note::Note() {
}

bool Note::contains(const std::vector<unsigned char>& note) const {
	for(int i = 0; i < 3; ++i) {
		if(note[i] < min[i] || note[i] > max[i])
			return false;
	}
	return true;
}

//bool Note::operator<(const Note& n1, const Note& n2) const {
bool Note::operator<(const Note& other) const {
	for(int i = 0; i < 3; ++i) {
		if(min[i] < other.min[i] && max[i] < other.max[i])
			return true;
	}
	return false;
}
