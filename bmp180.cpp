/***********************************************************
*
*	bmp180.cpp
*
***********************************************************/

#include <errno.h>
#include <syslog.h>
#include <stdio.h>
#include <cmath>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "bmp180.h"

////////////////////////////////////////////////////////////
//
//	bmp180
//
//	BMP180 chip manipulation
//
////////////////////////////////////////////////////////////
bmp180::bmp180()
{
	//
	//	Initialize class with calibration information
	//
	initialize();
}

/*
**	read16	
**
**	Helper method to manipulate bitshifts
*/
uint16_t bmp180::read16(int reg)
{
	return (wiringPiI2CReadReg8(fd, reg) << 8) | wiringPiI2CReadReg8(fd, reg + 1);
}

/*
**	initialize
**
**	Helper method to initialize I2C device and read calibration information
*/
void bmp180::initialize()
{
	//
	//	Create file descriptor
	//
	if (0 > (fd = wiringPiI2CSetup(BMP180_I2C_ADDRESS)))
	{
		//
		//	Write to log
		//
		openlog("XDL", LOG_PID, LOG_LOCAL5);
		syslog(LOG_INFO, "Unable to initialize BMP180.");
		closelog();

		fd = 0;
		return;
	}

	//
	//	Read calibration data
	//
	AC1 = read16(BMP180_AC1);
	AC2 = read16(BMP180_AC2);
	AC3 = read16(BMP180_AC3);
	AC4 = read16(BMP180_AC4);
	AC5 = read16(BMP180_AC5);
	AC6 = read16(BMP180_AC6);
	B1 = read16(BMP180_B1);
	B2 = read16(BMP180_B2);
	MB = read16(BMP180_MB);
	MC = read16(BMP180_MC);
	MD = read16(BMP180_MD);
}

/*
**	getTemp
**
**	Returns the sampled and calculated temperature from the bmp180 chip.
**
**	BUGBUG: If there is a problem with the sensor, getTemp returns 0, which is a valid temperature
*/
double bmp180::getTemp()
{
	int ut, x1, x2;

	//
	//	Sanity check
	//
	if (0 >= fd)
	{
		//
		//	Write to log
		//
		openlog("XDL", LOG_PID, LOG_LOCAL5);
		syslog(LOG_INFO, "BMP180 not initialized.");
		closelog();

		return 0;
	}

	//
	//	Start a temperature sensor reading
	//
	wiringPiI2CWriteReg8(fd, BMP180_OSSREG, BMP180_TEMPOSS);
	delay(5);

	//
	//	Read the raw data
	//
	ut = read16(BMP180_DATA);

	//
	// And calculate...
	//
	x1 = (ut - AC6) * AC5 / 32768;
	x2 = MC * 2048 / (x1 + MD);

	return (double)(x1 + x2 + 8)/160;
}
