#ifndef PT3_ORNAMENT_H_
#define PT3_ORNAMENT_H_

#include <cstdint>

class Ornament {
public:
	Ornament(const uint8_t * base);
	virtual ~Ornament();
	void setOrnament(const uint8_t * base);
	void setTickOffset(uint8_t offset);
	uint8_t getSemitoneOffset();
	void reset();
private:
	uint8_t loop;
	uint8_t length;
	const uint8_t * ornData;
	uint8_t position;
};

#endif /* PT3_ORNAMENT_H_ */
