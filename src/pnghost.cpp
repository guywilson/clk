#include <string>
#include <string.h>
#include <stdlib.h>

#include <png.h>

#include "binary.h"
#include "clk_error.h"
#include "pngrw.h"
#include "pnghost.h"
#include "cloak.h"
#include "xdump.h"

void PNGHost::addBlock(HostFileReader * reader, uint8_t * sourceBlock, size_t sourceBlockSize) {
    size_t numImageBytesRequired = CloakAlgorithm::calculateBlockHostBytesRequired(sourceBlockSize, security);

    PNGReader * pngReader = (PNGReader *)reader;
    uint8_t * imageBuffer = pngReader->getBlockPointer(numImageBytesRequired);

    CloakAlgorithm::mergeBlock(imageBuffer, sourceBlock, sourceBlockSize, security);
}

void PNGHost::extractBlock(HostFileReader * reader, uint8_t * targetBlock, size_t targetBlockSize) {
    size_t numImageBytesRequired = CloakAlgorithm::calculateBlockHostBytesRequired(targetBlockSize, security);

    PNGReader * pngReader = (PNGReader *)reader;
    uint8_t * imageBuffer = pngReader->getBlockPointer(numImageBytesRequired);

    CloakAlgorithm::extractBlock(imageBuffer, targetBlock, targetBlockSize, security);
}
