/*
 * a_modem_slave.c
 *
 *  Created on: Aug 1, 2014
 *      Author: root
 */


#include "acoustic_modem.h"

int main(){

	a_modem_open();
	a_modem_slave();
	a_modem_close();
	return 0;
}
