#include "Arduino.h"
#include <ModbusRTU.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>

#define TABLE_MAX_SIZE 100

// typedef struct table_s {
// 	int reg_addr;
// 	int reg_value;
// 	struct table_s *next;
// } table_t;

typedef struct {
  uint16_t reg_addr;
  uint16_t reg_value;  
} record_t;

typedef struct {
  uint16_t records_quantity;
  record_t record[TABLE_MAX_SIZE];
} records_t;

bool modbus_rtu_sniffer_init();
void modbus_rtu_sniffer_proc();
