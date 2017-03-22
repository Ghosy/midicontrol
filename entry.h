/*
 * placeholder
 * Header file for entry.cpp
 * Copyright(c) 2017 Zachary Matthews.
 */
#ifndef ENTRY_H_
#define ENTRY_H_

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

#endif
