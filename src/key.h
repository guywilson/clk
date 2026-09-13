#include <string>
#include <utility>

#include "algorithm.h"

#ifndef __INCL_KEY
#define __INCL_KEY

std::pair<uint8_t *, size_t> getKey(AlgorithmType & algorithm, bool generateKey, const std::string & keyFilename, size_t keyLength);

#endif
