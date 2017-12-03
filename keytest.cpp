/**
 * @file
 * @brief Implementation the main class for placeholder
 * @author Zachary Matthews
 *
 * Copyright(c) 2017 Zachary Matthews.
 */
#include <boost/regex.hpp>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "keytest.h"
#include "rtmidi/RtMidi.h"
#include "settings.h"

RtMidiIn *midiin;
RtMidiOut *midiout;
bool done;

static void finish(int ignore){ done = true; }

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
				std::cerr << "No Configuration file was specified." << std::endl;
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
						std::cerr << "The delay specified must be greater than 0" << std::endl;
					}
				}
				catch(...) {
					std::cerr << argv[i] << " is not a valid value for delay" << std::endl;
					exit(EXIT_FAILURE);
				}
			}
			else {
				std::cerr << "No delay was specified." << std::endl;
			}
		}
		else if((arg == "-h") || (arg == "--help")) {
			show_usage();
			return 0;
		}
		else if((arg == "-i") || (arg == "--input")) {
			if(++i < argc) {
				input_scan(argv[i]);
				return 0;
			}
			else {
				std::cerr << "No input device was specified." << std::endl;
			}
		}
		else if((arg == "-l") || (arg == "--list")) {
			list_ports();
			return 0;
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
			return 0;
		}
	}
	scan_ports();
	return 0;
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

		if(s == settings.getDevice()) {
			midiin->openPort(i);
			midiout->openPort(i);
		}
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

void midi_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata) {
	Entry temp_entry(*note_raw, *note_raw, "");
	auto it = settings.note_list.find(temp_entry);

	if(prog_settings::verbose) {
		std::cout << "Incoming note: " << temp_entry.get_note() << std::endl;
	}

	// If note is found in look up table
	if(it != settings.note_list.end()) {
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
				std::cout << "Note: " << temp_entry.get_note() << "\nExecuting: " << it->action << std::endl;
			}
			// TODO: Should this be hardcoded with min[2]?
			std::string command = note_replace(it->action, (int)temp_entry.min[2]);

			// Do the action associated with the corresponding midi note
			execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
			_exit(0);
		}

		// Led handling
		if(it->light_mode != LightMode::NONE) {
			std::vector<unsigned char> message;

			switch(it->light_mode) {
				case LightMode::LIGHT_ON:
				case LightMode::LIGHT_PUSH: {
					message.push_back(144);
					message.push_back((int)note_raw->at(1));
					message.push_back(it->light_value);
					midiout->sendMessage(&message);
					break;
				}
				case LightMode::LIGHT_OFF: {
					message.push_back(144);
					message.push_back((int)note_raw->at(1));
					message.push_back(0);
					midiout->sendMessage(&message);
					break;
				}
				case LightMode::LIGHT_WAIT: {
					message.push_back(144);
					message.push_back((int)note_raw->at(1));
					message.push_back(it->light_value);
					midiout->sendMessage(&message);

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
						message.pop_back();
						message.push_back(0);
						midiout->sendMessage(&message);
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
						std::cerr << "Non-conforming light_mode found for note, " << it->get_note() << std::endl;
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
		std::cout << "  Input Port #" << i << ": " << portName << std::endl;
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
		std::cout << "  Output Port #" << i << ": " << portName << std::endl;
	}
	std::cout << std::endl;
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
		std::cerr << "Failed to open device \"" << device << "\"" << std::endl;
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

void input_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata) {
	Entry temp_entry(*note_raw, *note_raw, "");
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
		// Install an interrupt handler function.
		(void) signal(SIGINT, finish);
		// Ensures this child exits when the rest of the program does
		while(!done) {
			for(auto e: check_list) {
				int ret;
				std::vector<unsigned char> message;

				// Get exit status of light_command
				// TODO: Make this section consider verbosity
				ret = WEXITSTATUS(system(e.light_command.c_str()));

				// Positive response(grep found)
				if(ret == 0) {
					// Ensure the full range is covered for the notes
					// Turn on leds
					for(int i = e.min[1]; i <= e.max[1]; ++i) {
						// Send midi note on, 144,xx,light_value
						message.push_back(144);
						message.push_back(i);
						message.push_back(e.light_value);
						midiout->sendMessage(&message);
						message.clear();
					}
				}
				// Negative response(grep failed to find)
				else if(ret == 1) {
					// Ensure the full range is covered for the notes
					// Turn off leds
					for(int i = e.min[1]; i <= e.max[1]; ++i) {
						// Send midi note off, 144,xx,00
						message.push_back(144);
						message.push_back(i);
						message.push_back(0);
						midiout->sendMessage(&message);
						message.clear();
					}
				}
				// Command exited with neither 1 nor 0
				else {
					std::cerr << "Unexpected return for light check on " << e.get_note() << std::endl;
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

				std::vector<unsigned char> message;
				for(int i = e.min[1]; i <= e.max[1]; ++i) {
					// Send midi note off, 144,xx,00
					message.push_back(144);
					message.push_back(i);
					message.push_back(new_light_value);
					midiout->sendMessage(&message);
					message.clear();
				}
			}
			// wait for delay time
			usleep(prog_settings::delay * 1000);
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
