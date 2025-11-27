//
// Created by xardb on 11/27/25.
//
#pragma once
#include <string>
#include <fstream>

std::ofstream open_history();
void append_history(std::ofstream& file, const std::string& line);
