#include <iosfwd>
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
	// If no arguements
	if(argc == 1) {
		scan_ports();
		return 0;
	}
	// Cases for all arguements
	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if((arg == "-h") || (arg == "--help")) {
			show_usage();
		}
		else if((arg == "-l") || (arg == "--list")) {
			list_ports();
		}
		else if((arg == "-c") || (arg == "--config")) {
			if(++i <= argc) {
				// This should be integrated before this check area
				settings.commandline_config(argv[i]);
				scan_ports();
			}
			else {
				std::cout << "No Config file was specified." << std::endl;
			}
		}
		// If the arguement is not supported show_usage
		else {
			show_usage();
		}
		return 0;
	}
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
	for (auto it = settings.note_list.begin(); it != settings.note_list.end(); ++it) {
		if(it->first.contains(*note_raw)) {
			// Ensure all children are reaped
			signal(SIGCHLD, SIG_IGN);
			// Fork for command to be run
			pid_t pid = fork();
			if(pid < 0) {
				perror("Fork failed");
			}
			if(pid == 0) {
				// Do the action associated with the corresponding midi note
				execl("/bin/sh", "sh", "-c", it->second[0].c_str(), NULL);
				_exit(0);
			}

			// Led handling
			if(it->second.size() >= 2) {
				std::vector<unsigned char> message;
				// light_on
				if(it->second[1] == "light_on") {
					message.push_back(144);
					message.push_back((int)note_raw->at(1));
					message.push_back(stoi(it->second[2]));
					midiout->sendMessage(&message);
				}
				// light_off
				if(it->second[1] == "light_off") {
					message.push_back(144);
					message.push_back((int)note_raw->at(1));
					message.push_back(0);
					midiout->sendMessage(&message);
				}
				// light_wait
				if(it->second[1] == "light_wait") {
					message.push_back(144);
					message.push_back((int)note_raw->at(1));
					message.push_back(stoi(it->second[2]));
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
				}
			}
		}
	}
}

void show_usage() {
	// Prints usage/help information
	std::cout 
	<< "Usage: placeholder [OPTION]...\n"
	<< "  -h, --help      show this help message\n"
	<< "  -l, --list      list midi input/output ports\n"
	<< "  -c, --config    Load alternate configuration file\n"
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
