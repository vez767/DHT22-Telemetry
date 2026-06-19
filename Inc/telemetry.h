/*
 * telemetry.h
 *
 *  Created on: 12 Jun 2026
 *      Author: vez767
 */

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include <stdint.h>

typedef struct {
    float Temperature;
    float Humidity;
    uint8_t Sensor_Status;
} Climate_Payload_t;




#endif /* TELEMETRY_H_ */
