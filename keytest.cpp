#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "RtMidi.h"
#include <unistd.h>
#include <map>
#include <sstream>

using namespace std;
bool done;
static void finish(int ignore){ done = true; }
void midi_read(vector<unsigned char> note);
void scan_ports();
string int_to_string(int a);
void show_usage();
void list_ports();

int main(int argc, char* argv[]) {
	if(argc == 1) {
		scan_ports();
		return 0;
	}
	for(int i = 1; i < argc; ++i) {
		string arg = argv[i];
		if((arg == "-h") || (arg == "--help")) {
			show_usage();
			return 0;
		}
		else if((arg == "-l") || (arg == "--list")) {
			list_ports();
			return 0;
		}
		else {
			scan_ports();
			return 0;
		}
	}
}

void scan_ports() {
	RtMidiIn *midiin = new RtMidiIn();
	vector<unsigned char> message;
	//int nBytes;
	// Check available ports.
	unsigned int nPorts = midiin->getPortCount();
	if( nPorts == 0 ) {
		cout << "No ports available!\n";
		goto cleanup;
	}
	midiin->openPort(3);
	// Don't ignore sysex, timing, or active sensing messages.
	midiin->ignoreTypes( false, false, false );
	// Install an interrupt handler function.
	done = false;
	(void) signal(SIGINT, finish);
	// Periodically check input queue.
	cout << "Reading MIDI from port ... quit with Ctrl-C.\n";
	while( !done ) {
		midiin->getMessage( &message );
		//nBytes = message.size();
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
	map<string, const char*> note_list;
	note_list["144,0,127"] = "ncmpcpp prev";
	note_list["144,1,127"] = "ncmpcpp toggle";
	note_list["144,2,127"] = "ncmpcpp next";
	string note;
	for(unsigned int i = 0; i < note_raw.size(); i++) {
		note += int_to_string((int)note_raw[i]);
		if((i + 1) != note_raw.size())
			note += ",";
	}
	system(note_list[note]);
}

string int_to_string(int a) {
	ostringstream ss;
	ss << a;
	return ss.str();
}

void show_usage() {
	cout 
	<< "Usage: placeholder [OPTION]...\n"
	<< "  -h, --help   show this help message\n"
	<< "  -l, --list   list midi input/output ports\n"
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
