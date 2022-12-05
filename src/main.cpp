#include "main.h"

uint32_t sleepingPeriod = 10000;  // 30 * 60 * 1000;  //первое число - минуты
uint16_t attamptsNumber = 5;      //количество попыток повторных пересылок сообщений

long ticks = 0;

Adafruit_BME280 bme;

NodeValue* vltValue;
NodeValue* tmpValue;
NodeValue* humValue;
NodeValue* prsValue;

void preHwInit() {
}

void before() {
    NRF_POWER->DCDCEN = 1;        //включение режима оптимизации питания, расход снижается на 40%, но должны быть установленны емкости (если нода сделана на модуле https://a.aliexpress.com/_mKN3t2f то нужно раскомментировать эту строку)
    NRF_NFCT->TASKS_DISABLE = 1;  //останавливает таски, если они есть
    // NRF_NVMC->CONFIG = 1;   //разрешить запись
    NRF_UICR->NFCPINS = 0;  //отключает nfc и nfc пины становятся доступными для использования
    // NRF_NVMC->CONFIG = 0;  //запретить запись
#ifdef SERIAL_PRINT
    // NRF_UART0->ENABLE = 1;
#else
    // NRF_UART0->ENABLE = 0;
#endif
}

void setup() {
    SerialPrintln("====================Started=====================");
    bme.getTemperatureSensor();
    bme.getPressureSensor();
    bme.getHumiditySensor();
    bme.begin(0x76);

    vltValue = new NodeValue(0, V_VOLTAGE, 5, 0.1, false);
    tmpValue = new NodeValue(1, V_TEMP, 5, 1, false);
    humValue = new NodeValue(2, V_HUM, 5, 1, false);
    prsValue = new NodeValue(3, V_PRESSURE, 5, 1, true);
}

void presentation() {
    SerialPrintln("Presentation");
    sendSketchInfo("IoT Manager BME280 Node", "1.0.3");
    present(0, S_MULTIMETER);
    present(1, S_TEMP);
    present(2, S_HUM);
    present(3, S_BARO);
    wait(50);
    SerialPrintln("Presentation completed");
}

void loop() {
    SerialPrintln("==================== Tick: " + String(ticks) + " ====================");

    vltValue->handleValue((float)hwCPUVoltage() / 1000.00);
    tmpValue->handleValue(bme.readTemperature());
    humValue->handleValue(bme.readHumidity());
    prsValue->handleValue(bme.readPressure() / 1.333224 / 100);

    ticks++;
}

void SerialPrintln(String text) {
#ifdef SERIAL_PRINT
    Serial.println(text);
#endif
}

//класс датчика===============================================================================================

NodeValue::NodeValue(int childId, const mysensors_data_t dataType, int attamptsNumber, float trashhold, bool goToSleep) {
    firstInit = true;
    _childId = childId;
    _dataType = dataType;
    _attamptsNumber = attamptsNumber;
    _trashhold = trashhold;
    _goToSleep = goToSleep;
}

NodeValue::~NodeValue() {
}

void NodeValue::handleValue(float value) {
    _value = value;
    //если устройство было включено первый раз
    if (firstInit) {
        firstInit = false;
        SerialPrintln("First loading");
        //приравняем предыдущее значение и полученное что бы начать отсчет
        _prevValue = _value;
        //отправим значение
        sendMsgAndGoToSleep();
    } else {
        //если устройство вышло из сна, проверим изменилось ли значение достаточно
        //здесь нужен перебор вектора
        if (isValueChangedEnough()) {
            sendMsgAndGoToSleep();
        } else {
            sleep(sleepingPeriod);
        }
    }
}

void NodeValue::sendMsgAndGoToSleep() {
    if (sendMsgFastAck()) {
        SerialPrintln("Msg " + String(_childId) + " delivered, value = " + String(_value));
    } else {
        //если значение не отправилось с нескольких попыток то гейт выключен - идем в сон
        SerialPrintln("Go to sleep, gate missing, try again after " + String(sleepingPeriod / 1000) + " sec");
        sleep(sleepingPeriod);
    }
    //если эта величина последняя для отправки - то уходим в сон
    if (_goToSleep) {
        sleep(sleepingPeriod);
    }
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
