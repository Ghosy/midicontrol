#ifndef KEYTEST_H_
#define KEYTEST_H_

#include <vector>

void midi_read(std::vector<unsigned char> note);
void scan_ports();
std::string int_to_string(const int a);
void show_usage();
void list_ports();

#endif
