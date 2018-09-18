/**
 * @file
 * @brief Implementation the main class for placeholder
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
#include <boost/regex.hpp>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "keytest.h"
#include "rtmidi/RtMidi.h"
#include "settings.h"

RtMidiIn *midiin;
RtMidiOut *midiout;
bool done;

static void finish(int){ done = true; }

int main(int argc, char* argv[]) {
	// Cases for all arguments
	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if((arg == "-c") || (arg == "--config")) {
			if(++i < argc) {
				// This should be integrated before this check area
				settings.commandline_config(argv[i]);
			}
			else {
				std::cerr << "No Configuration file was specified." << '\n';
			}
		}
		else if((arg == "-d") || (arg == "--delay")) {
			if(++i < argc) {

				try {
					unsigned int delay = std::stoi(argv[i]);
					if(delay > 0) {
						prog_settings::delay = delay;
					}
					else {
						std::cerr << "The delay specified must be greater than 0" << '\n';
					}
				}
				catch(...) {
					std::cerr << argv[i] << " is not a valid value for delay" << '\n';
					exit(EXIT_FAILURE);
				}
			}
			else {
				std::cerr << "No delay was specified." << '\n';
			}
		}
		else if((arg == "-h") || (arg == "--help")) {
			show_usage();
			exit(EXIT_SUCCESS);
		}
		else if((arg == "-i") || (arg == "--input")) {
			if(++i < argc) {
				input_scan(argv[i]);
				exit(EXIT_SUCCESS);
			}
			else {
				std::cerr << "No input device was specified." << '\n';
				exit(EXIT_FAILURE);
			}
		}
		else if((arg == "-l") || (arg == "--list")) {
			list_ports();
			exit(EXIT_SUCCESS);
		}
		else if((arg == "-q") || (arg == "--quiet")) {
			prog_settings::quiet = true;
		}
		else if((arg == "-s") || (arg == "--silent")) {
			prog_settings::silent = true;
		}
		else if((arg == "-v") || (arg == "--verbose")) {
			prog_settings::verbose = true;
		}
		// If the argument is not supported show_usage
		else {
			show_usage();
			exit(EXIT_SUCCESS);
		}
	}
	scan_ports();
	exit(EXIT_SUCCESS);
}

void scan_ports() {
	settings.read();

	try {
		midiin = new RtMidiIn();
		midiout = new RtMidiOut();
	}
	catch (RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	std::vector<unsigned char> message;

	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if(nPorts == 0) {
		std::cout << "No ports available!\n";
		goto cleanup;
	}
	// Go threw ports and open the configured device
	for(unsigned int i = 0; i < nPorts; i++) {
		std::string s = midiin->getPortName(i);
		s = s.substr(0, s.find_last_of(' '));

		if(s == settings.get_device()) {
			midiin->openPort(i);
			midiout->openPort(i);
		}
	}

	// Check to ensure a port was opened
	if(!midiin->isPortOpen()) {
		std::cerr << "Failed to open device \"" << settings.get_device() << "\"" << '\n';
		exit(EXIT_FAILURE);
	}
	// Set callback function
	// Make sure this always follows opening of port
	// to prevent messages ending up in queue
	midiin->setCallback(&midi_read);
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes(false, false, false);
	// Install an interrupt handler function.
	done = false;
	(void) signal(SIGINT, finish);
	// Start light_mode checker
	light_state_check();
	if(!prog_settings::quiet && !prog_settings::silent)
		std::cout << "Reading MIDI from port ... quit with Ctrl-C.\n";
	while(!done) {
		usleep(10000);
	}
	// Clean up
cleanup:
	delete midiin;
	delete midiout;
}

void midi_read(double, std::vector<unsigned char> *note_raw, void *) {
	Entry temp_entry(*note_raw, "");

	if(prog_settings::verbose) {
		std::cout << "Incoming note: " << temp_entry.get_note() << std::endl;
	}

	// Find matches to incoming note
	std::vector<Entry> matches;
	std::copy_if(settings.note_list.begin(), settings.note_list.end(), std::back_inserter(matches), [note_raw](Entry e){return e.contains(*note_raw);});

	// For each matching note in conf
	for(auto match: matches) {
		// Ensure all children are reaped
		signal(SIGCHLD, SIG_IGN);
		// Fork for command to be run
		pid_t pid = fork();
		if(pid < 0) {
			perror("Fork failed");
		}
		if(pid == 0) {
			if(prog_settings::quiet || prog_settings::silent) {
				int fd = open("/dev/null", O_WRONLY);
				dup2(fd, 1);
			}
			if(prog_settings::verbose) {
				std::cout << "Note: " << temp_entry.get_note() << "\nExecuting: " << match.action << std::endl;
			}
			// TODO: Should this be hardcoded with note[2]?
			std::string command = note_replace(match.action, (int)temp_entry.note[2]);

			// Do the action associated with the corresponding midi note
			execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
			_exit(0);
		}

		// Led handling
		if(match.light_mode != LightMode::NONE) {
			std::vector<unsigned char> message;

			switch(match.light_mode) {
				case LightMode::LIGHT_ON:
				case LightMode::LIGHT_PUSH: {
					note_send((int)note_raw->at(1), match.light_value);
					break;
				}
				case LightMode::LIGHT_OFF: {
					note_send((int)note_raw->at(1), 0);
					break;
				}
				case LightMode::LIGHT_WAIT: {
					note_send((int)note_raw->at(1), match.light_value);

					// Fork and wait for child executing command to exit
					pid_t pid_light = fork();
					if(pid_light < 0) {
						perror("Fork failed");
					}
					if(pid_light == 0) {
						// Wait for action to finish
						while(kill(pid, 0) == 0) {
							usleep(10000);
						}
						// Turn off led
						note_send((int)note_raw->at(1), 0);
						_exit(0);
					}
					break;
				}
				case LightMode::LIGHT_CHECK: {
					// Do nothing this mode is not checked on note received
					break;
				}
				case LightMode::LIGHT_VAR: {
					// Do nothing this mode is not checked on note received
					break;
				}
				default: {
					// Print error non conforming light_mode
					if(!prog_settings::silent)
						std::cerr << "Non-conforming light_mode found for note, " << match.get_note() << '\n';
					break;
				}
			}
		}
	}
}

void show_usage() {
	// Prints usage/help information
	std::cout 
		<< "Usage: placeholder [OPTION]...\n"
		<< "  -c, --config=FILE     Load alternate configuration file\n"
		<< "  -d, --delay=DELAY     Specify the delay time for LIGHT_CHECK\n"
		<< "                          Defaults to 50ms\n"
		<< "  -h, --help            Show this help message\n"
		<< "  -i, --input=DEVICE    Print incoming midi notes from DEVICE\n"
		<< "  -l, --list            List midi input/output ports\n"
		<< "  -q, --quiet           Suppress normal output when reading midi input\n"
		<< "  -s, --silent          Suppress normal output and suppress errors\n"
		<< "  -v, --verbose         Print extra information during reading input\n"
		<< "                          --quiet and --silent override --verbose\n"
		;
}

void list_ports() {
	RtMidiIn  *midiin = 0;
	RtMidiOut *midiout = 0;
	// RtMidiIn initialization
	try {
		midiin = new RtMidiIn();
	}
	catch (RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	// Check inputs.
	unsigned int nPorts = midiin->getPortCount();
	std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
	std::string portName;
	for (unsigned int i=0; i<nPorts; i++) {
		try {
			portName = midiin->getPortName(i);
		}
		catch (RtMidiError &error) {
			error.printMessage();
			goto cleanup;
		}
		std::cout << "  Input Port #" << i << ": " << portName << '\n';
	}
	// RtMidiOut initialization
	try {
		midiout = new RtMidiOut();
	}
	catch (RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	// Check outputs.
	nPorts = midiout->getPortCount();
	std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
	for (unsigned int i=0; i<nPorts; i++) {
		try {
			portName = midiout->getPortName(i);
		}
		catch (RtMidiError &error) {
			error.printMessage();
			goto cleanup;
		}
		std::cout << "  Output Port #" << i << ": " << portName << '\n';
	}
	std::cout << '\n';
	// Clean up
cleanup:
	delete midiin;
	delete midiout;
}

void input_scan(std::string device) {
	RtMidiIn  *midiin = 0;

	try {
		midiin = new RtMidiIn();
	}
	catch (RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
	std::vector<unsigned char> message;

	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if(nPorts == 0) {
		std::cout << "No ports available!\n";
		goto cleanup;
	}
	// Go threw ports and open the configured device
	for(unsigned int i = 0; i < nPorts; i++) {
		std::string s = midiin->getPortName(i);
		s = s.substr(0, s.find_last_of(' '));

		if(s == device) {
			midiin->openPort(i);
		}
	}

	// Check to ensure a port was opened
	if(!midiin->isPortOpen()) {
		std::cerr << "Failed to open device \"" << device << "\"" << '\n';
		exit(EXIT_FAILURE);
	}
	// Set callback function
	// Make sure this always follows opening of port
	// to prevent messages ending up in queue
	midiin->setCallback(&input_read);
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes(false, false, false);
	// Install an interrupt handler function.
	done = false;
	(void) signal(SIGINT, finish);
	std::cout << "Reading MIDI from port ... quit with Ctrl-C.\n";
	while(!done) {
		usleep(10000);
	}
	// Clean up
cleanup:
	delete midiin;
}

void input_read(double, std::vector<unsigned char> *note_raw, void *) {
	Entry temp_entry(*note_raw, "");
	// Print note received
	std::cout << "Note: " << temp_entry.get_note() << std::endl;
}

void light_state_check() {
	std::set<Entry> check_list;
	std::set<Entry> var_list;
	// Filter out entries with LIGHT_CHECK
	std::copy_if(settings.note_list.begin(), settings.note_list.end(), std::inserter(check_list, check_list.end()), [](const Entry e){return (e.light_mode == LightMode::LIGHT_CHECK);});

	// Filter out entries with LIGHT_VAR
	std::copy_if(settings.note_list.begin(), settings.note_list.end(), std::inserter(var_list, var_list.end()), [](const Entry e){return (e.light_mode == LightMode::LIGHT_VAR);});

	// Fork for checker child
	pid_t pid_checker = fork();

	if(pid_checker < 0) {
		perror("Fork failed");
	}
	if(pid_checker == 0) {
		// Ensure child exits upon parent death
		prctl(PR_SET_PDEATHSIG, SIGHUP);
		while(!done) {
			for(auto e: check_list) {
				int ret;
				std::vector<unsigned char> message;

				// Get exit status of light_command
				// TODO: Make this section consider verbosity
				ret = WEXITSTATUS(system(e.light_command.c_str()));

				// Positive response(grep found)
				if(ret == 0) {
					// Turn on led
					// TODO: Fix so that this makes more sense, check function for TODO below
					note_send(e.note[1], e.light_value);
				}
				// Negative response(grep failed to find)
				else if(ret == 1) {
					// Turn off led
					note_send(e.note[1], 0);
				}
				// Command exited with neither 1 nor 0
				else {
					std::cerr << "Unexpected return for light check on " << e.get_note() << '\n';
				}

			}

			for(auto e: var_list) {
				std::string data;
				FILE * stream;
				const int max_buffer = 256;
				char buffer[max_buffer];
				// Only get stdout
				std::string command = e.light_command.append(" 2>&1");

				stream = popen(command.c_str(), "r");
				if(stream) {
					while(!feof(stream))
						if(fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
					pclose(stream);
				}
				int new_light_value = std::stoi(data);

				note_send(e.note[1], new_light_value);
			}
			// wait for delay time
			usleep(prog_settings::delay * 1000);
		}
		if(done) {
			exit(EXIT_SUCCESS);
		}
	}
}

std::string note_replace(std::string s, unsigned int note) {
			// TODO: Break up regex_replace arguments for readability
			// Replace instances of note value label with current note value
			s = boost::regex_replace(s, boost::regex("(?<!\\\\)NOTE(?!%)", boost::regex::perl), std::to_string(note));
			s = boost::regex_replace(s, boost::regex("(?<!\\\\)NOTE%", boost::regex::perl), std::to_string(note * 100 / 127));
			s = boost::regex_replace(s, boost::regex("\\\\NOTE", boost::regex::perl), "NOTE");
			return s;
}

// TODO: Add support for more channel support than channel 1
// TODO: Should this use entry?
void note_send(unsigned char data_1, unsigned char data_2) {
	std::vector<unsigned char> message;
	message.push_back(144);
	message.push_back(data_1);
	message.push_back(data_2);
	midiout->sendMessage(&message);
}
/* vim: set ts=8 sw=8 tw=0 noet :*/
