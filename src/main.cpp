#include "main.h"

uint32_t sleepingPeriod = 30 * 60 * 1000;  //первое число - минуты
uint16_t attamptsNumber = 5;               //количество попыток повторных пересылок сообщений

long ticks = 0;

Adafruit_BME280 bme;

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

enum values {
    vlt,
    tmp,
    hum,
    prs,
};

float valueArr[prs + 1];
float prevValueArr[prs + 1];

void loop() {
    SerialPrintln("Tick: " + String(ticks));
    //получение данных для отправки
    valueArr[vlt] = (float)hwCPUVoltage() / 1000.00;
    valueArr[tmp] = bme.readTemperature();
    valueArr[hum] = bme.readHumidity();
    valueArr[prs] = bme.readPressure() / 1.333224 / 100;

    if (ticks == 0) {  //если первый раз приравняем массивы и отправляем значения как есть
        SerialPrintln("First loading ");
        for (int i = 0; i <= prs; i++) {
            prevValueArr[i] = valueArr[i];
        }
        sendValues();

    } else {  //если второй и более раз, то вначале проверяем изменились ли данные, и если одно из значений изменилось шлем все данные
        if (ifValueChangedEnough(vlt, 0.1) || ifValueChangedEnough(tmp, 1.0) || ifValueChangedEnough(hum, 1.0) || ifValueChangedEnough(prs, 1.0)) {
            sendValues();
        } else {
            SerialPrintln("Go to sleep, no enough value changes detected " + String(sleepingPeriod / 1000) + " sec");
            sleep(sleepingPeriod);
        }
    }

    ticks++;
}

//отправка сообщений. Посленняя отправка должна иметь флаг true, тогда нода уйдет в сон после последней отправки
void sendValues() {
    sendMsgFastAck(vlt, V_VOLTAGE, valueArr[vlt], false);
    sendMsgFastAck(tmp, V_TEMP, valueArr[tmp], false);
    sendMsgFastAck(hum, V_HUM, valueArr[hum], false);
    sendMsgFastAck(prs, V_PRESSURE, valueArr[prs], true);
}

//эта функция отправляет сообщения любого типа, функция предпринимает несколько попыток отправки в случае неудачи
//для подтверждения используется ack. Гарантируется доставка до ближайшего узла.
void sendMsgFastAck(int ChildId, const mysensors_data_t dataType, float value, bool goToSleep) {
    MyMessage msg(ChildId, dataType);
#ifdef ACK_MODE
    int attempts;
    while (!send(msg.set(value, 2), false)) {  //если не отправилось
        attempts++;
        SerialPrintln("Msg " + String(ChildId) + " not delivered, attempt: " + String(attempts));
        _transportSM.failedUplinkTransmissions = 0;
        wait(10);
        if (attempts >= attamptsNumber) {
            attempts = 0;
            SerialPrintln("Go to sleep, gate missing, try again after " + String(sleepingPeriod / 1000) + " sec");
            sleep(sleepingPeriod);
        }
    }
    SerialPrintln("Msg " + String(ChildId) + " delivered, value = " + String(value));
#else
    send(msg.set(value, 2), false);
#endif
    if (goToSleep) sleep(sleepingPeriod);
}

bool ifValueChangedEnough(int index, float trashhold) {
    bool ret = false;
    float prevValueUpper = prevValueArr[index] + trashhold;
    float prevValueLower = prevValueArr[index] - trashhold;

    float currentValue = valueArr[index];
    if (currentValue > prevValueUpper || currentValue < prevValueLower) {
        ret = true;
        SerialPrintln("value " + String(index) + " changed");
        prevValueArr[index] = valueArr[index];
    }
    return ret;
}

void SerialPrintln(String text) {
#ifdef SERIAL_PRINT
    Serial.println(text);
#endif
}