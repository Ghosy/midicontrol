/**
 * @file
 * @brief Header of the main class for placeholder
 * @author Zachary Matthews
 *
 * Copyright(c) 2017 Zachary Matthews.
 */
#ifndef KEYTEST_H_
#define KEYTEST_H_

#include <vector>

/**
 * @brief The callback function called from scan_ports
 */
void midi_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata);

/**
 * @brief Open midi device to read incoming signals and convert to actions
 */
void scan_ports();

/**
 * @brief Display usage message to stdout
 */
void show_usage();

/**
 * @brief Display incoming and outgoing midi devices to stdout
 */
void list_ports();

/**
 * @brief Open midi device to read incoming signals and display those signals
 * @param device The name of the device input is being scanned from
 */
void input_scan(std::string device);

/**
 * @brief The callback function for handling incoming notes called from input_scan
 */
void input_read(double deltatime, std::vector<unsigned char> *note_raw, void *userdata);

#endif
