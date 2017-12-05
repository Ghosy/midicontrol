/**
 * @file
 * @brief Header of entry class for placeholder
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
#ifndef ENTRY_H_
#define ENTRY_H_

/**
 * @var LightMode::NONE
 * No interaction with light
 * @var LightMode::LIGHT_PUSH
 * Turn on light when note pressed and off when released
 * @var LightMode::LIGHT_ON
 * Turn on light when note pressed
 * @var LightMode::LIGHT_OFF
 * Turn off light when note pressed
 * @var LightMode::LIGHT_WAIT
 * Turn on light when note pressed and off when triggered action finishes
 * @var LightMode::LIGHT_CHECK
 * Turn on light when command has a return value of 0
 * @var LightMode::LIGHT_VAR
 * Change light level based on command return
 */
enum class LightMode {
	NONE,
	LIGHT_PUSH,
	LIGHT_ON,
	LIGHT_OFF,
	LIGHT_WAIT,
	LIGHT_CHECK,
	LIGHT_VAR
};

class Entry
{
	public:
		/**
		 * @brief Create empty Entry
		 */
		Entry();

		/**
		 * @brief Create Entry without light settings
		 * @param lows A vector containing the lower bound of the midi note
		 * @param highs A vector containing the upper bound of the midi note
		 * @param new_action A string containing the corresponding command for the defined midi note
		 */
		Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action);
		/**
		 * @brief Create Entry with light settings
		 * @param lows A vector containing the lower bound of the midi note
		 * @param highs A vector containing the upper bound of the midi note
		 * @param new_action A string containing the corresponding command for the defined midi note
		 * @param new_mode A light mode corresponding to the midi note
		 * @param new_light_value A value between 0-255 indicating the light value
		 */
		Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action, LightMode new_mode, unsigned char new_light_value);

		/**
		 * @brief Create Entry with light settings
		 * @param lows A vector containing the lower bound of the midi note
		 * @param highs A vector containing the upper bound of the midi note
		 * @param new_action A string containing the corresponding command for the defined midi note
		 * @param new_mode A light mode corresponding to the midi note
		 * @param new_light_value A value between 0-255 indicating the light value
		 * @param new_light_command A string containing the corresponding command for LIGHT_CHECK and LIGHT_VAR
		 */
		Entry(std::vector<unsigned char> lows, std::vector<unsigned char> highs, std::string new_action, LightMode new_mode, unsigned char new_light_value, std::string new_light_command);

		/**
		 * @brief Returns true if and only if the Entry contains the specified midi note
		 * @param note A vector containing the three segments of a midi note
		 * @return True, if the Entry contains note, false otherwise
		 */
		bool contains(const std::vector<unsigned char>& note) const;

		/**
		 * @brief Returns true if and only if midi note in the Entry is less than other's midi note
		 * @param other An Entry being compared to the current Entry
		 * @return True, if current Entry is less than other Entry, false otherwise
		 */
		bool operator<(const Entry& other) const;

		/**
		 * @brief Gets note value in human readable form (Example: 144,1,1)
		 * @return String containing human readable midi note (Example: 144,1,1)
		 */
		std::string get_note() const;

		/**
		 * @brief A string containing command associated with Entry's midi note
		 */
		std::string action;

		/**
		 * @brief A light mode corresponding to the midi note
		 */
		LightMode light_mode;

		/**
		 * @brief A string containing the corresponding command for LIGHT_CHECK and LIGHT_VAR
		 */
		std::string light_command;

		/**
		 * @brief A value between 0-255 indicating the light value
		 */
		unsigned char light_value;


		/**
		 * @brief An array containing the lower bounds of midi note
		 */
		// TODO: Should this really be stored as array or use more useful storage
		unsigned char min[3];

		/**
		 * @brief An array containing the upper bounds of midi note
		 */
		unsigned char max[3];
};

#endif
