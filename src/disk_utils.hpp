//
// Created by xardb on 11/27/25.
//
#pragma once
#include <iostream>
#include <string>
using namespace std;

void dump_sector_hex(const unsigned char* buf);
bool read_gpt(const string& device);
void check_mbr(const string& path);
