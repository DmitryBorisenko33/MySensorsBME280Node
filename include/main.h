#pragma once
//=======CONFIGURATION=SECTION========
//#define MY_DEBUG
#define SERIAL_PRINT    //эта строка включает печать информации в serial
#define MY_NODE_ID 100  //здесь задается id ноды, если необходимо что бы id выдавал гейт, то нужно закомментировать данную строку
#define MY_RADIO_NRF5_ESB
//#define MY_PASSIVE_NODE  //включение пассивного режима ноды, в этом режиме нода не будет ждать подтвержения получения сообщения

//=====================================

#ifndef MY_PASSIVE_NODE
#define ACK_MODE
#endif

#ifdef SERIAL_PRINT
#define MY_BAUD_RATE 115200
#endif

#include <MySensors.h>
#include <variant.h>
#include <Adafruit_BME280.h>

extern Adafruit_BME280 bme;

extern uint32_t sleepingPeriod;
extern uint16_t attamptsNumber;

extern long ticks;

extern void SerialPrintln(String text);

//класс датчика===============================================================================================
class NodeValue {
   public:
    NodeValue(int childId, const mysensors_data_t dataType, int attamptsNumber, float trashhold, bool goToSleep);

    ~NodeValue();

    void handleValue(float value);
    float getValue();
    bool sendMsgFastAck();
    bool isValueChangedEnough();
    void sendMsgAndGoToSleep();

   private:
    bool _goToSleep;
    int _childId;
    mysensors_data_t _dataType;
    bool firstInit = true;
    float _value, _prevValue, _trashhold;
    int _attamptsNumber;
};

extern NodeValue* vltValue;
extern NodeValue* tmpValue;
extern NodeValue* humValue;
extern NodeValue* prsValue;
