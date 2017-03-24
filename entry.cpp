/*
 * placeholder
 * Implementation of entry class for placeholder
 * Copyright(c) 2017 Zachary Matthews.
 */
#include <iostream>
#include <vector>
#include <sstream>

#include "entry.h"

Entry::Entry() {
}

Entry::Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action)
	: action(new_action),
	light_mode(LightMode::NONE),
	light_value(0) {
		for(int i = 0; i < 3; ++i) {
			min[i] = lows[i];
			max[i] = highs[i];
		}
	}

Entry::Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action, LightMode new_mode, unsigned char new_light_value)
	: action(new_action),
	light_mode(new_mode),
	light_value(new_light_value) {
		for(int i = 0; i < 3; ++i) {
			min[i] = lows[i];
			max[i] = highs[i];
		}
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

std::string Entry::get_note() const {
	std::ostringstream ostream;

	for(int i = 0; i < 3; ++i) {
		if(min[i] == max[i]) {
			ostream << (int)min[i];
		}
		else {
			ostream << (int)min[i] << ".." << (int)max[i];
		}
		if(i < 2) {
			ostream << ",";
		}
	}
	return ostream.str();
}
