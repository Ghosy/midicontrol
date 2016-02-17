#ifndef KEYTEST_H_
#define KEYTEST_H_

#include <vector>

void midi_read(std::vector<unsigned char> note);
void scan_ports();
void show_usage();
void list_ports();

#endif
