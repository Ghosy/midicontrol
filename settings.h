#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <map>

struct config {
	config();
	
	void read();
	void commandline_config(const char*);
	std::map<std::vector<unsigned char>, std::vector<std::string> > note_list;
	// std::vector<std::string> entry_list;
	std::string getDevice();

private:
	std::string config_file_path;
	std::string midi_device;
	std::string trim(std::string);
};

extern config settings;

#endif
