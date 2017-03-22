#ifndef KEYTEST_H_
#define KEYTEST_H_

#include <vector>

void midi_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata);
void scan_ports();
void show_usage();
void list_ports();
void input_scan(std::string device);
void input_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata);

#endif
