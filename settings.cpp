#include <map>
#include <fstream>
#include <iostream>

#include "settings.h"

using namespace std;

config settings;

config::config() {
	config_file_path = "keyconf";
	midi_device = "";
}

void config::read() {
	ifstream f(config_file_path.c_str());
	string cline, name, value;
	
	if(!f.is_open())
		return;
	
	while(!f.eof()) {
		getline(f, cline);
		
		if(!cline.empty() && cline[0] != '#') {
			int pos;
			pos = cline.find("=");

			name = cline.substr(0, pos);
			value = cline.substr(pos + 1, cline.length() - pos);
			if(name == "device") {
				midi_device = value;
			}
			else {
				note_list[name] = value;
			}
		}
	}
}

void config::commandline_config(char* conf_path) {
	config_file_path = conf_path;
}

string config::getDevice() {
	return midi_device;
}
