#pragma once
#include <Arduino.h>
#include "main.h"

class NodeValue {
   public:
    NodeValue(int childId, const mysensors_data_t dataType, int attamptsNumber, float trashhold);

    ~NodeValue();

    void setValue(float value);
    float getValue();
    bool sendMsgFastAck();
    bool isValueChangedEnough();
    void loop();

   private:
    int _childId;
    mysensors_data_t _dataType;
    bool firstInit = true;
    float _value, _prevValue, _trashhold;
    int _attamptsNumber;
};

// extern NodeValue* vltValue;