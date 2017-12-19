#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <ctime>
#include "bmp180.h"

//
// Define GPIO pins assigned to devices
//
#define GPIO_HEAT	0
#define GPIO_DEHUM	2
#define OFF	0
#define ON	1

////////////////////////////////////////////////////////////
//
//	controller
//
//	Controls the system based upon environment conditions.
//	The class is implemented as a singleton class, meaning 
//	that there will be only one instance of the object.
//
////////////////////////////////////////////////////////////
class controller
{
	//
	//	Temperature sensor
	//
	bmp180	sensor;

	//
	//	Timestamps to mark when an appliance was started
	//
	time_t	heaterTS;
	time_t	dehumidifierTS;

private:
	//
	//	Private construtor
	//
	controller();

	controller(controller const& copy);				// Not implemented
	controller& operator=(controller const& copy);	// Not implemented

protected:
	void initialize();
	void logStatus(double temp, bool heaterState, bool dehumidifierState);
	void getConfigData(double &lowTemp, double &highTemp, int &cycleRate);
	void poll();
	int getHeater();
	void setHeater(int state);
	int getDehumidifier();
	void setDehumidifier(int state);

public:
	~controller();

	static controller* getInstance();
	void commence();
};

#endif //CONTROLLER_H