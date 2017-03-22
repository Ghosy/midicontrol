#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sstream>

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
		if((arg == "-h") || (arg == "--help")) {
			show_usage();
			return 0;
		}
		else if((arg == "-l") || (arg == "--list")) {
			list_ports();
			return 0;
		}
		else if((arg == "-c") || (arg == "--config")) {
			if(++i <= argc) {
				// This should be integrated before this check area
				settings.commandline_config(argv[i]);
			}
			else {
				std::cout << "No Config file was specified." << std::endl;
			}
		}
		else if((arg == "-q") || (arg == "--quiet")) {
			prog_settings::quiet = true;
		}
		else if((arg == "-s") || (arg == "--silent")) {
			prog_settings::silent = true;
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
			// Do the action associated with the corresponding midi note
			execl("/bin/sh", "sh", "-c", it->action.c_str(), NULL);
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
		<< "  -c, --config    Load alternate configuration file\n"
		<< "  -h, --help      show this help message\n"
		<< "  -l, --list      list midi input/output ports\n"
		<< "  -q, --quiet     Supress normal output when reading midi input\n"
		<< "  -s, --silent    Supress normal output and suppress errors\n"
		;
}

void list_ports() {
	RtMidiIn  *midiin = 0;
	RtMidiOut *midiout = 0;
	// RtMidiIn constructor
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
	// RtMidiOut constructor
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
