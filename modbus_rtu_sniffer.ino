#include "modbus_rtu_sniffer.h"


void setup() {
  modbus_rtu_sniffer_init();
}

void loop() {
	modbus_rtu_sniffer_proc();
}
