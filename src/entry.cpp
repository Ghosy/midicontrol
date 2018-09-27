/**
 * @file
 * @brief Implementation of entry class for placeholder
 * @author Zachary Matthews
 *
 * Copyright(c) 2017 Zachary Matthews.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#include "entry.h"

Entry::Entry() {
}

Entry::Entry(const std::vector<unsigned char> new_note, const std::string new_action)
	: Entry::Entry(new_note, new_action, LightMode::NONE, 0, "") {}

Entry::Entry(const std::vector<unsigned char> new_note, const std::string new_action, LightMode new_mode, unsigned char new_light_value)
	: Entry::Entry(new_note, new_action, new_mode, new_light_value, "") {}

Entry::Entry(const std::vector<unsigned char> new_note, const std::string new_action, LightMode new_mode, unsigned char new_light_value, const std::string new_light_command)
	: note(new_note),
	action(new_action),
	light_mode(new_mode),
	light_command(new_light_command),
	light_value(new_light_value) {}

// TODO: Replace with equality check?
bool Entry::contains(const std::vector<unsigned char>& other_note) const {
	for(int i = 0; i < 3; ++i) {
		if(other_note[i] < note[i] || other_note[i] > note[i])
			return false;
	}
	return true;
}

bool Entry::operator==(const Entry& other) const {
	for(int i = 0; i < 3; ++i) {
		if(other.note[i] != note[i])
			return false;
	}
	return true;
}

bool Entry::operator<(const Entry& other) const {
	return std::tie(note[0], note[1], note[2]) < std::tie(other.note[0], other.note[1], other.note[2]); 
}

std::string Entry::get_note() const {
	std::ostringstream ostream;

	for(int i = 0; i < 3; ++i) {
		ostream << (int)note[i];
		if(i < 2) {
			ostream << ",";
		}
	}
	return ostream.str();
}
/* vim: set ts=8 sw=8 tw=0 noet :*/
