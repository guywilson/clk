#pragma once

#include <string>
#include <utility>

#include "algorithm.h"

std::pair<uint8_t *, size_t> getKey(AlgorithmType & algorithm, bool generateKey, const std::string & keyFilename, size_t keyLength);
