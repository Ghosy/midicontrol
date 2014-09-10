#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <map>
#include <sstream>

#include "keytest.h"
#include "RtMidi.h"
#include "settings.h"

using namespace std;
bool done;
static void finish(int ignore){ done = true; }
// void midi_read(vector<unsigned char> note);
// void scan_ports();
// string int_to_string(int a);
// void show_usage();
// void list_ports();

int main(int argc, char* argv[]) {
	// If no arguements
	if(argc == 1) {
		scan_ports();
		return 0;
	}
	// Cases for all arguements
	for(int i = 1; i < argc; ++i) {
		string arg = argv[i];
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
				cout << "No Config file was specified." << endl;
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
	RtMidiIn *midiin;
	try {
		midiin = new RtMidiIn();
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}
	vector<unsigned char> message;
	
	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if( nPorts == 0 ) {
		cout << "No ports available!\n";
		goto cleanup;
	}
	for(unsigned int i = 0; i < nPorts; i++) {
		if(midiin->getPortName(i) == settings.getDevice()) {
			midiin->openPort(i);
		}
	}
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes( false, false, false );
	// Install an interrupt handler function.
	done = false;
	(void) signal(SIGINT, finish);
	// Periodically check input queue.
	cout << "Reading MIDI from port ... quit with Ctrl-C.\n";
	while( !done ) {
		midiin->getMessage( &message );
		if(message.size() > 0) {
			midi_read(message);
		}
		usleep(10000);
	}
	// Clean up
cleanup:
	delete midiin;
}

void midi_read(vector<unsigned char> note_raw) {
	// Map  midi notes = key  commands = values
	// map<string, const char*> note_list;
	// note_list["144,0,127"] = "ncmpcpp prev";
	// note_list["144,1,127"] = "ncmpcpp toggle";
	// note_list["144,2,127"] = "ncmpcpp next";
	// Converts vector to readable string
	string note;
	for(unsigned int i = 0; i < note_raw.size(); i++) {
		note += int_to_string((int)note_raw[i]);
		if((i + 1) != note_raw.size())
			note += ",";
	}
	system(settings.note_list[note].c_str());
}

string int_to_string(int a) {
	ostringstream ss;
	ss << a;
	return ss.str();
}

void show_usage() {
	// Prints usage/help information
	cout 
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
	catch ( RtMidiError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}
	// Check inputs.
	unsigned int nPorts = midiin->getPortCount();
	cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
	string portName;
	for ( unsigned int i=0; i<nPorts; i++ ) {
		try {
			portName = midiin->getPortName(i);
		}
		catch ( RtMidiError &error ) {
			error.printMessage();
			goto cleanup;
		}
		cout << "  Input Port #" << i << ": " << portName << '\n';
	}
	// RtMidiOut constructor
	try {
		midiout = new RtMidiOut();
	}
	catch ( RtMidiError &error ) {
		error.printMessage();
		exit( EXIT_FAILURE );
	}
	// Check outputs.
	nPorts = midiout->getPortCount();
	cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
	for ( unsigned int i=0; i<nPorts; i++ ) {
		try {
			portName = midiout->getPortName(i);
		}
		catch (RtMidiError &error) {
			error.printMessage();
			goto cleanup;
		}
		cout << "  Output Port #" << i << ": " << portName << '\n';
	}
	cout << '\n';
	// Clean up
cleanup:
	delete midiin;
	delete midiout;
}
