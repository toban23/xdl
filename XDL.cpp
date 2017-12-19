/***********************************************************
* 
*	XDL.cpp
* 
***********************************************************/


#include <iostream>
#include "exceptions.h"
#include "stdtools.h"
#include "controller.h"

/*
 * main
 */
int main(int argc, char **argv)
{
	controller* _controller = controller::getInstance();

	_controller->commence();
	
	return 0;
}

