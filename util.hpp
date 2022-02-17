#pragma once

#include <string>

double factorial(double num);

void error(const std::string& e);

void error(const std::string& s, int i);

void error(const std::string& s, const std::string& s2);

bool narrow_cast(const double a);

std::string &ltrim(std::string &str);

std::string &rtrim(std::string &str);

std::string trim(std::string str);