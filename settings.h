#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <map>

class Note
{
	public:
		Note();
		Note(std::vector<unsigned char> lows, std::vector<unsigned char> highs);
		bool contains(const std::vector<unsigned char>& note) const;
		bool operator<(const Note& other) const;
	private:
		unsigned char min[3];
		unsigned char max[3];
};

struct config {
	config();
	
	void read();
	void commandline_config(const char*);
	std::map<Note, std::vector<std::string> > note_list;
	std::string getDevice();

private:
	std::string config_file_path;
	std::string midi_device;
	std::string trim(std::string);
};

extern config settings;

#endif
