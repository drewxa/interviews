#pragma once

#include <iostream>
#include <string>
#include <map>
#include <thread>

#include "InfInt.h"

typedef std::string String;
#define ReleasePSEGMENT(x) { if(x) {delete[] x; x = NULL;} }