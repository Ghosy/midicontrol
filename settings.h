#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <set>
#include <utility>

enum class LightMode {
	NONE,
	LIGHT_PUSH,
	LIGHT_ON,
	LIGHT_OFF,
	LIGHT_WAIT
};

class Entry
{
	public:
		Entry();
		Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action);
		Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action, LightMode new_mode, unsigned char new_light_value);
		bool contains(const std::vector<unsigned char>& note) const;
		bool operator<(const Entry& other) const;
		std::string get_note() const;
		
		std::string action;
		LightMode light_mode;
		unsigned char light_value;
		// TODO: Should this really be stored as array or use more useful storage
		unsigned char min[3];
		unsigned char max[3];
};

struct config {
	config();
	
	void read();
	void commandline_config(const char*);
	std::set<Entry> note_list;
	std::string getDevice();

private:
	std::string config_file_path;
	std::string midi_device;
	std::string trim(std::string);
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>> read_note(const std::string);
};

extern config settings;

// I swear to god if these names aren't fix to be
// less confusing soon, someone will get hurt.
namespace prog_settings {
	extern bool quiet;
	extern bool silent;
	extern bool verbose;
}

#endif
