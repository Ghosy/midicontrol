#include <map>
#include <fstream>
#include <iostream>

#include "settings.h"

using namespace std;

config settings;

void config::read() {
	ifstream f("keyconf");
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
			note_list[name] = value;
		}
	}
}
