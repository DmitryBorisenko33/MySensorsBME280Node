#pragma once
//=======CONFIGURATION=SECTION========
//#define MY_DEBUG
#define SERIAL_PRINT    //эта строка включает печать информации в serial
#define MY_NODE_ID 100  //здесь задается id ноды, если необходимо что бы id выдавал гейт, то нужно закомментировать данную строку
#define MY_RADIO_NRF5_ESB
#define MY_PASSIVE_NODE
//#define ACK_MODE
//====================================

#ifdef SERIAL_PRINT
#define MY_BAUD_RATE 115200
#endif

#include <MySensors.h>
#include <variant.h>
#include <Adafruit_BME280.h>

extern Adafruit_BME280 bme;

extern uint32_t sleepingPeriod;
extern uint16_t attamptsNumber;

extern void sendMsgFastAck(int ChildId, const mysensors_data_t dataType, float value, bool goToSleep);
extern void SerialPrintln(String text);
