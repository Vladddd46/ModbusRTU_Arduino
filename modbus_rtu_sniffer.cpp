#include "modbus_rtu_sniffer.h"

#define PRESENT_SINGLE_REGISTER_FUNC_CODE    0x06
#define PRESENT_MULTIPLE_REGISTERS_FUNC_CODE 0x10
#define READ_HOLDING_REGISTERS_FUNC_CODE     0x03

records_t table;

static uint16_t modbus_crc_calculation(uint8_t *buf, uint8_t cNumberByte) {
  uint16_t iCRC;
  uint8_t cBitCounter, i = 0;
  iCRC = 0xFFFF; 

  while (cNumberByte > 0) {
    iCRC ^= buf[i++] ;
    cBitCounter = 0; 

    while (cBitCounter < 8) {
      if (iCRC & 0x0001) {
        iCRC >>= 1; 
        iCRC ^= 0xA001;
      }
      else {
        iCRC >>=1;
      }
      cBitCounter++;
    }
    cNumberByte--;
  }
  return (iCRC);
}



bool modbus_rtu_sniffer_init() {
  Serial.begin(9600);
  Serial1.begin(9600);
  table.records_quantity = 0;
  return true;
}



/*
 * Receives packet from master.
 * Retrieves register address from packet.
 * Stores retrieved value in reg_address.
 */
bool master_read(uint16_t &reg_address) {
    uint8_t master_packet[256];
    bzero(master_packet, 256);

    unsigned long time_ms = millis();
    int num_of_ms = 0;
    int index = 0;
    while(1) {
        if (time_ms >= 4) {
            break;
        }

        if (Serial.available() > 0) {
            time_ms = millis();

            if (index > 255) {
                break;
            }
            master_packet[index] = Serial.read();
            index += 1;
        }
        else {
            if (time_ms - millis() >= 1) {
                num_of_ms += 1;
            }
        }
    }

    if (num_of_ms > 1 && num_of_ms < 4) {
        return false;
    }

    if (index == 0) {
        return false;
    }

    // validating checksum
    int len = strlen(master_packet);
    uint16_t checksum = modbus_crc_calculation((uint8_t *)master_packet, (uint8_t)len);
    uint8_t arr[2];
    arr[0] = checksum & 0xff;
    arr[1] = checksum >> 8;
    if (arr[0] == checksum[len - 1] || arr[1] == checksum[len - 2]) {
        return false;
    }

    if (master_packet[1]    != PRESENT_SINGLE_REGISTER_FUNC_CODE 
        && master_packet[1] != PRESENT_MULTIPLE_REGISTERS_FUNC_CODE 
        && master_packet[1] != READ_HOLDING_REGISTERS_FUNC_CODE) {
        return false;
    }

    uint16_t addr = master_packet[2];
    addr = addr << 8;
    addr |= master_packet[3];
    reg_address = addr;
    return true;
}


/*
 * Receives packet from slave.
 * Retrieves register value from packet.
 * Stores retrieved value in reg_value.
 */
bool slave_read(uint16_t &reg_value) {
    uint8_t salve_packet[256];
    bzero(salve_packet, 256);
    int index = 0;

    while(Serial1.available() > 0) {
        if (index > 255) {
            break;
        }
        salve_packet[index] = Serial1.read();
        index += 1;
    }
    if (index == 0) {
        return false;
    }


    // validating checksum
    int len = strlen((char *)salve_packet);
    uint16_t checksum = modbus_crc_calculation ((uint8_t *)salve_packet, (uint8_t)len);
    uint8_t arr[2];
    arr[0] = checksum & 0xff;
    arr[1] = checksum >> 8;
    if (arr[0] == checksum[len - 1] || arr[1] == checksum[len - 2]) {
        return false;
    }

    if (salve_packet[1]    != PRESENT_SINGLE_REGISTER_FUNC_CODE 
        && salve_packet[1] != PRESENT_MULTIPLE_REGISTERS_FUNC_CODE 
        && salve_packet[1] != READ_HOLDING_REGISTERS_FUNC_CODE) {
        return false;
    }

    uint16_t value = salve_packet[4]; 
    for (int i = 5; i < len - 2; ++i) {
      value = value << 8;
      value |= salve_packet[i];
    }
    reg_value = value;
    return true;
}


void print_table() {
    int i = 0;
    while(i < table.records_quantity) {
        printf("reg_addr: %d; reg_value: %d\n", (int)table.record[i].reg_addr, 
                                                (int)table.record[i].reg_value);
        i++;
    }
}

void modbus_rtu_sniffer_proc() {
    uint16_t reg_addr;
    uint16_t reg_value;
    bool err = master_read(reg_addr);

    // if (err == 1) {
        // err = slave_read(reg_value);
        if (err && table.records_quantity < TABLE_MAX_SIZE) {
            table.records_quantity += 1;
            table.record[table.records_quantity].reg_addr  = reg_addr;
            table.record[table.records_quantity].reg_value = reg_value;
        // }
    }
    
//    delay(1000);
    print_table();
}
