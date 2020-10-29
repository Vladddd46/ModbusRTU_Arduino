#include "modbus_rtu_sniffer.h"

records_t table;

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

    int index = 0;
    while(Serial.available() > 0) {
        if (index > 255) {
            break;
        }
        master_packet[index] = Serial.read();
        index += 1;
    }
    if (index == 0) {
        return false;
    }


    // Chesum calculating
    // int len = strlen((char *)master_packet);
    // int sum = master_packet[0];
    // for (int i = 1; i < len - 2; ++i) {
    //   sum = sum << 8;
    //   sum |= master_packet[i];
    // }
    // int checksum = master_packet[len - 2];
    // checksum = checksum << 8;
    // checksum |= master_packet[len - 1];
    // if (checksum != sum) {
    //   return false;
    // }

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

    // Checksum calculating.
    // int len = strlen((char *)salve_packet);
    // int sum = salve_packet[0];
    // for (int i = 1; i < len - 2; ++i) {
    //   sum = sum << 8;
    //   sum |= salve_packet[i];
    // }
    // int checksum = salve_packet[len - 2];
    // checksum = checksum << 8;
    // checksum |= salve_packet[len - 1];
    // if (checksum != sum) {
    //   return false;
    // }

    int len = strlen((char *)salve_packet);
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
            table.record[table.records_quantity].reg_addr = reg_addr;
        // }
    }
    
//    delay(1000);
    print_table();
}
