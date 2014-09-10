#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <map>

struct config {
	config();
	
	void read();
	void commandline_config(char*);
	std::map<std::string, std::string> note_list;
	std::string getDevice();

private:
	std::string config_file_path;
	std::string midi_device;
};

extern config settings;

#endif
