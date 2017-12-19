#ifndef BMP180_H
#define BMP180_H

#include <stdint.h>

//
//	Defines the chip's register addresses
//
#define	BMP180_I2C_ADDRESS	0x77
#define BMP180_AC1			0xAA
#define BMP180_AC2			0xAC
#define BMP180_AC3			0xAE	
#define BMP180_AC4			0xB0	
#define BMP180_AC5			0xB2	
#define BMP180_AC6			0xB4
#define BMP180_B1			0xB6
#define BMP180_B2			0xB8
#define BMP180_MB			0xBA
#define BMP180_MC			0xBC
#define BMP180_MD			0xBE

#define BMP180_OSSREG		0xF4
#define BMP180_TEMPOSS		0x2E
#define BMP180_PRESOSS		0x34
#define BMP180_DATA			0xF6


////////////////////////////////////////////////////////////
//
//	bmp180
//
//	BMP180 chip manipulation
//
////////////////////////////////////////////////////////////
class bmp180
{
public:
	bmp180();

	double getTemp();

protected:
	//
	//	File descriptor for wiringPi library
	//
	int fd;

	//
	//	Static calibration data
	//
	int16_t		AC1, AC2, AC3;
	uint16_t	AC4, AC5, AC6;
	int16_t		B1, B2;
	int16_t		MB, MC, MD;

private:

	void initialize();
	uint16_t read16(int reg);
};
#endif // !BMP180_H

