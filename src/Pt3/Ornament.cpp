#include <Ornament.h>

Ornament::Ornament(const uint8_t * base) {
    setOrnament(base);
}

Ornament::~Ornament() {
}

void Ornament::setOrnament(const uint8_t * base) {
    loop = base[0];
    length = base[1];
    ornData = &base[2];
    position = 0;
}

void Ornament::setTickOffset(uint8_t offset) {
    position = offset;
}

uint8_t Ornament::getSemitoneOffset() {
    if (position >= length) {
        position = loop;
    }
    return ornData[position++];
}

void Ornament::reset() {
    position = 0;
}
