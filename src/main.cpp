#include "main.h"

String inMsg = "";
uint32_t sleepingPeriod = 10000;  //первое число - минуты
uint16_t attamptsNumber = 5;      //количество попыток повторных пересылок сообщений

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
    sendSketchInfo("IoT Manager BME280 Node", "1.0.2");
    present(0, S_MULTIMETER);
    present(1, S_TEMP);
    present(2, S_HUM);
    present(3, S_BARO);
    wait(50);
    SerialPrintln("Presentation completed");
}

void loop() {
    //получение данных для отправки
    float batteryVoltage = (float)hwCPUVoltage() / 1000.00;
    float tmp = bme.readTemperature();
    float hum = bme.readHumidity();
    float prs = bme.readPressure();
    prs = prs / 1.333224 / 100;
    //отправка сообщений. Посленняя отправка должна иметь флаг true, тогда нода уйдет в сон после последней отправки
    sendMsgFastAck(0, V_VOLTAGE, batteryVoltage, false);
    sendMsgFastAck(1, V_TEMP, tmp, false);
    sendMsgFastAck(2, V_HUM, hum, false);
    sendMsgFastAck(3, V_PRESSURE, prs, true);
    SerialPrintln("==============================================");
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

void SerialPrintln(String text) {
#ifdef SERIAL_PRINT
    Serial.println(text);
#endif
}