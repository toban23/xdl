/***********************************************************
*
*	controller.cpp
*
*	Main controller logic
*
***********************************************************/

#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <syslog.h>
#include <string>
#include <sstream>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <libconfig.h++>
#include "controller.h"

#define	SWITCH_INTERVAL	3

using namespace libconfig;

////////////////////////////////////////////////////////////
//
//	controller
//
//	Controls the system based upon environment conditions.
//
////////////////////////////////////////////////////////////
controller::controller()
{
	initialize();
}

controller::~controller()
{
	setHeater(OFF);
	setDehumidifier(OFF);
}

/*
*	getInstance
*
*	Returns the one and only one instance of the controller object
*
*/
controller* controller::getInstance()
{
	//
	//	The only instance
	//	Guaranteed to be lazy initialized
	//	Guaranteed that it will be destroyed correctly
	//
	static controller instance;

	return &instance;
}

/*
*	initialize
*
*	Initializes system
*
*/
void controller::initialize()
{
	//
	//	Configure the GPIO for heater and dehumidifier
	//
	wiringPiSetup();

	pinMode(GPIO_HEAT, OUTPUT);
	pinMode(GPIO_DEHUM, OUTPUT);

	// 
	//	Set the initial state
	//
	setHeater(OFF);
	setDehumidifier(OFF);
}

/*
*	log
*
*	Logs system state information.
*
*/
void controller::logStatus(double temp, bool heaterState, bool dehumidifierState)
{
	std::ostringstream loginfo;

	//
	//	Create log message
	//
	loginfo << "Temperature: " << temp;

	loginfo << " Heater: ";

	if (heaterState)
		loginfo << "On as of " << ctime(&heaterTS);
	else
		loginfo << "Off\n";

	loginfo << " Dehumidifier: ";
	if (dehumidifierState)
		loginfo << "On as of " << ctime(&dehumidifierTS);
	else
		loginfo << "Off\n";

	loginfo << std::endl;

	//
	//	Write to log
	//
	openlog("XDL", LOG_PID, LOG_LOCAL5);
	syslog(LOG_INFO, loginfo.str().c_str());
	closelog();
}

void controller::getConfigData(double &lowTemp, double &highTemp, int &cycleRate)
{
	Config config;

	//
	//	Get configuration information
	//
	try
	{
		config.readFile("app.cfg");

		if (!config.lookupValue("application.lowTemp", lowTemp))
		{
			//
			//	Handle error
			//
			openlog("XDL", LOG_PID, LOG_LOCAL5);
			syslog(LOG_ERR, "Unable to find 'lowTemp' value in configuration file.");
			closelog();
		}

		if (!config.lookupValue("application.highTemp", highTemp))
		{
			//
			//	Handle error
			//
			openlog("XDL", LOG_PID, LOG_LOCAL5);
			syslog(LOG_ERR, "Unable to find 'highTemp' value in configuration file.");
			closelog();
		}

		if (!config.lookupValue("application.cycleRate", cycleRate))
		{
			//
			//	Handle error
			//
			openlog("XDL", LOG_PID, LOG_LOCAL5);
			syslog(LOG_ERR, "Unable to find 'cycleRate' value in configuration file.");
			closelog();
		}
	}
	catch (ConfigException e)
	{
		//
		//	Configuration file error - log error and set defaults
		//
		//
		//	Handle error
		//
		openlog("XDL", LOG_PID, LOG_LOCAL5);
		syslog(LOG_ERR, e.what());
		closelog();

		lowTemp = 5.0;			// 5 degrees celcius
		highTemp = 8.0;			// 8 degrees celcius
		cycleRate = 1800;		// Thirty minutes
	}
}

/*
*	poll
*
*	Polls the temperature and makes. 
*
*/
void controller::poll()
{
	double temp;
	double lowTemp, highTemp;
	int cycleRate;

	//
	//	Get configuration data
	//
	getConfigData(lowTemp, highTemp, cycleRate);

	//
	//	Get current temp
	//
	temp = sensor.getTemp();

	//
	//	Determine action
	//
	if ( temp < lowTemp )
	{	
		//
		//	Temp is below freezing - only heater
		//
		if (OFF == getHeater())
		{
			setDehumidifier(OFF);
			sleep(SWITCH_INTERVAL);
			setHeater(ON);
		}
	}
	else if ( temp < highTemp )
	{
		//
		//	Temp is midzone - cycle between heater and dehumidifer
		//
		time_t now;

		time(&now);

		if ((OFF == getHeater()) && (OFF == getDehumidifier()))
		{
			//
			//	Heater and dehumidifer are both off, turn on heater
			//
			setHeater(ON);
			sleep(SWITCH_INTERVAL);
			setDehumidifier(OFF);
		}
		if ((ON == getHeater()) && (ON == getDehumidifier()))
		{
			//
			//	Heater and dehumidifer are both on, this is an error state. 
			//	To rectify, turn off dehumidifier.
			//
			setHeater(ON);
			sleep(SWITCH_INTERVAL);
			setDehumidifier(OFF);
		}
		else if (ON == getHeater())
		{
			//
			//	Heater is on. If it has had allocated quantum, switch to dehumidifier
			//
			if (cycleRate < (difftime(now, heaterTS)))
			{
				setHeater(OFF);
				sleep(SWITCH_INTERVAL);
				setDehumidifier(ON);
			}
		}
		else if ((ON == getDehumidifier()))
		{
			//
			//	Dehumidifier is on. If it has had allocated quantum, switch to heater
			//
			if (cycleRate < (difftime(now, dehumidifierTS)))
			{
				setDehumidifier(OFF);
				sleep(SWITCH_INTERVAL);
				setHeater(ON);
			}
		}
	}
	else // (temp >= highTemp)
	{
		//
		//	Temp is warm - dehumidifier only
		//
		if (OFF == getDehumidifier())
		{
			setHeater(OFF);
			sleep(SWITCH_INTERVAL);
			setDehumidifier(ON);
		}
	}

	//
	//	Log result
	//
	logStatus(temp, getHeater(), getDehumidifier());
}

/*
*	setHeater
*
*	Sets the heater to the defined state and initializes or clears the timestampe.
*
*/
void controller::setHeater(int state)
{
	switch (state)
	{
	case ON:
		time(&heaterTS); break;
	case OFF:
		heaterTS = 0; break;
	}

	digitalWrite(GPIO_HEAT, state);
}

/*
*	getHeater
*
*	Gets the state of the heater.
*
*/
int controller::getHeater()
{
	return digitalRead(GPIO_HEAT);
}

/*
*	setDehumidifier
*
*	Sets the dehumidifier to the defined state and initializes or clears the timestampe.
*
*/
void controller::setDehumidifier(int state)
{
	switch (state)
	{
	case ON:
		time(&dehumidifierTS); break;
	case OFF:
		dehumidifierTS = 0; break;
	}
	digitalWrite(GPIO_DEHUM, state);
}

/*
*	getDehumidifier
*
*	Gets the state of the dehumidifier.
*
*/
int controller::getDehumidifier()
{
	return digitalRead(GPIO_DEHUM);
}

void controller::commence()
{
	int pollRate;
	Config config;

	//
	//	Open configuration file
	//
	try
	{
		config.readFile("app.cfg");
	}
	catch (ConfigException e)
	{
		//
		//	Handle Error
		//
		openlog("XDL", LOG_PID, LOG_LOCAL5);
		syslog(LOG_ERR, e.what());
		closelog();
	}

	if (!config.lookupValue("application.pollRate", pollRate))
	{
		//
		//	Handle Error
		//
		openlog("XDL", LOG_PID, LOG_LOCAL5);
		syslog(LOG_ERR, "Unable to find 'pollRate' value in configuration file..");

		char cCurrentPath[FILENAME_MAX];
		if (!getcwd(cCurrentPath, sizeof(cCurrentPath))) 
			return;
		syslog(LOG_ERR, cCurrentPath);

		closelog();
	}

	while (true)
	{
		poll();
		sleep(pollRate);
	}
}
