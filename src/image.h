#pragma once

#include "hostfile.h"

class Image : public HostFile {
    protected:
        size_t width;
        size_t height;
};
