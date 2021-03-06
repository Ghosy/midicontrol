/**
 * @file
 * @brief Header of the main class for placeholder
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
#ifndef KEYTEST_H_
#define KEYTEST_H_

#include <vector>

#include "entry.h"

/**
 * @brief The callback function called from scan_ports
 */
void midi_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata);

/**
 * @brief Start loggers for midicontrol
 */
void init_logger();

/**
 * @brief Log all program settings before running
 */
void log_settings();

/**
 * @brief Open midi device to read incoming signals and convert to actions
 */
void scan_ports();

/**
 * @brief Display usage message to stdout
 */
void show_usage();

/**
 * @brief Display version message to stdout
 */
void show_version();

/**
 * @brief Display incoming and outgoing midi devices to stdout
 */
void list_ports();

/**
 * @brief Open midi device to read incoming signals and display those signals
 * @param device The name of the device input is being scanned from
 */
void input_scan(const std::string &device);

/**
 * @brief The callback function for handling incoming notes called from input_scan
 */
void input_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata);

/**
 * @brief Start the checker for notes with light_mode of LIGHT_CHECK and LIGHT_VAR
 */
void light_state_loop();

/**
 * @brief Replace occurrences of NOTE and NOTE% with corresponding value
 */
std::string note_replace(std::string s, unsigned int note);

/**
 * @brief Send midi note back to the device
 */
void note_send(const std::vector<unsigned char> &note);

/**
 * @brief Clean up allocated memory
 */
void clean_up();

#endif
/* vim: set ts=8 sw=8 tw=0 noet :*/
