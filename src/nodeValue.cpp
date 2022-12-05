
#include "nodeValue.h"

NodeValue::NodeValue(int childId, const mysensors_data_t dataType, int attamptsNumber, float trashhold) {
    firstInit = false;
    _childId = childId;
    _dataType = dataType;
    _attamptsNumber = attamptsNumber;
    _trashhold = trashhold;
}

NodeValue::~NodeValue() {
}

void NodeValue::setValue(float value) {
    _value = value;
}

float NodeValue::getValue() {
    return _value;
}

bool NodeValue::sendMsgFastAck() {
    MyMessage msg(_childId, _dataType);
    bool ret = true;
#ifdef ACK_MODE
    int attempts;
    while (!send(msg.set(_value, 2), false)) {  //если не отправилось
        attempts++;
        SerialPrintln("Msg " + String(_childId) + " not delivered, attempt: " + String(attempts));
        _transportSM.failedUplinkTransmissions = 0;
        wait(10);
        if (attempts >= _attamptsNumber) {
            attempts = 0;
            ret = false;
        }
    }

#else
    ret = send(msg.set(value, 2), false);
#endif
    return ret;
}

bool NodeValue::isValueChangedEnough() {
    bool ret = false;
    if (_value > (_prevValue + _trashhold / 2) || _value < (_prevValue - _trashhold / 2)) {
        ret = true;
        SerialPrintln("value " + String(_childId) + " changed");
        _prevValue = _value;
    }
    return ret;
}

void NodeValue::loop() {
}
