#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <map>

struct config {

	void read();
	std::map<std::string, std::string> note_list;

};

extern config settings;

#endif
