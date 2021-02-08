#include "main.h"

String inMsg = "";
uint32_t sleepingPeriod = 30 * 60 * 1000;  //первое число - минуты
uint16_t attamptsNumber = 3;     //количество попыток повторных пересылок сообщений
long totalErrors = 0;
long fatalErrors = 0;

void preHwInit() {
    // pinMode(25, OUTPUT);
    // digitalWrite(25, HIGH);
    // pinMode(27, OUTPUT);
    // digitalWrite(27, HIGH);
}

void before() {
    //NRF_POWER->DCDCEN = 1; //включение режима оптимизации питания, расход снижается на 40%, но должны быть установленны емкости
    //NRF_NFCT->TASKS_DISABLE = 1; //останавливает таски, если они есть
    //NRF_UICR->NFCPINS = 0;  //отключает nfc и nfc пины становятся доступными для использования
    //NRF_NVMC->CONFIG = 0;   //
    //NRF_UART0->ENABLE = 0;  //отклчает uart, прежде чем использовать в коде не должно быть serial.print
}

void setup() {
    SerialPrintln("====================Started=====================");
}

void presentation() {
    sendSketchInfo("IoT Manager Node", "1.0.0");
    present(0, S_MULTIMETER);
    present(1, S_TEMP);
    present(2, S_HUM);
    present(3, S_BARO);
}

void loop() {
    float batteryVoltage = (float)hwCPUVoltage() / 1000.00;

    //sendMsgEchoAck(0, 0, V_VOLTAGE, batteryVoltage, false);
    //wait(5);
    //SerialPrintln("==============================================");
    //sendMsgEchoAck(0, 1, V_TEMP, random(1000, 1500), false);
    //wait(5);
    //SerialPrintln("==============================================");
    //sendMsgEchoAck(0, 2, V_TEMP, random(100, 150), false);
    //wait(5);
    //SerialPrintln("==============================================");
    //sendMsgEchoAck(0, 3, V_CUSTOM, fatalErrors, false);
    //wait(5);
    //SerialPrintln("==============================================");
    //sendMsgEchoAck(0, 4, V_CUSTOM, totalErrors, true);  // у последнего сообщения в loop должно стоять true, у остальных false
    //SerialPrintln("==============================================");

  
    sendMsgFastAck(0, V_VOLTAGE, batteryVoltage, false);
    sendMsgFastAck(1, V_TEMP, random(10, 30), false);
    sendMsgFastAck(2, V_HUM, random(100, 150), false);
    sendMsgFastAck(3, V_PRESSURE, random(0, 100), true);
    SerialPrintln("==============================================");
}

void sendMsgEchoAck(int nodeId, int ChildId, const mysensors_data_t dataType, float value, bool goToSleep) {
    static int attempts = 0;
    inMsg = "";
    MyMessage msg(ChildId, dataType);
    send(msg.setDestination(nodeId).setSensor(ChildId).set(value, 2), true);  //отправляем сообщение
    String outMsg = String(nodeId) + "," +                                    //формируем его уникальную сигнатуру в виде 0,0,12.5;
                    String(ChildId) + "," +
                    String(value) + ";";
    SerialPrintln("sended: " + outMsg);
    long prevMillis = millis();
    wait(1500, C_SET, dataType);  //ждем пока получим echo в функции receive
    long ackTime = millis() - prevMillis;
    if (inMsg == outMsg) {  //если сигнатура полученного эхо совпала с отправленным сообщением - сообщение было доставлено
        SerialPrintln("Msg " + String(ChildId) + " delivered, ack time = " + String(ackTime) + " ms");
        attempts = 0;
        if (goToSleep) sleep(sleepingPeriod);
    } else {  //если не совпала значит в эхо ничего не пришло
        attempts++;
        totalErrors++;
        _transportSM.failedUplinkTransmissions = 0;  //сбросим счетчик в ноль что бы нода не ушла в поиск сети
        SerialPrintln("Msg " + String(ChildId) + " not delivered, attempt: " + String(attempts));
        if (attempts >= attamptsNumber) {
            attempts = 0;
            fatalErrors++;
            SerialPrintln("Go to sleep, gate missing, try again after " + String(sleepingPeriod / 1000) + " sec");
            sleep(sleepingPeriod);
        }
    }
}

//void sendMsgFastAck(int ChildId, const mysensors_data_t dataType, float value, bool goToSleep) {
//    static int attempts = 0;
//    MyMessage msg(ChildId, dataType);
//    if (send(msg.set(value, 2), false)) {  //если отправилось
//        attempts = 0;
//        SerialPrintln("Msg " + String(ChildId) + " delivered, value = " + String(value));
//        if (goToSleep) sleep(sleepingPeriod);
//    } else {
//        attempts++;
//        totalErrors++;
//        SerialPrintln("Msg " + String(ChildId) + " not delivered, attempt: " + String(attempts));
//        wait(10);
//        if (attempts >= attamptsNumber) {
//            attempts = 0;
//            fatalErrors++;
//            SerialPrintln("Go to sleep, gate missing, try again after " + String(sleepingPeriod / 1000) + " sec");
//            sleep(sleepingPeriod);
//        }
//    }
//}

void sendMsgFastAck(int ChildId, const mysensors_data_t dataType, float value, bool goToSleep) {
    int attempts;
    MyMessage msg(ChildId, dataType);
    while (!send(msg.set(value, 2), false)) {  //если не отправилось
        attempts++;
        totalErrors++;
        SerialPrintln("Msg " + String(ChildId) + " not delivered, attempt: " + String(attempts));
        _transportSM.failedUplinkTransmissions = 0;
        wait(10);
        if (attempts >= attamptsNumber) {
            attempts = 0;
            fatalErrors++;
            SerialPrintln("Go to sleep, gate missing, try again after " + String(sleepingPeriod / 1000) + " sec");
            sleep(sleepingPeriod);
        }
    }
    SerialPrintln("Msg " + String(ChildId) + " delivered, value = " + String(value));
    if (goToSleep) sleep(sleepingPeriod);
}

void receive(const MyMessage &message) {
    if (message.isEcho()) {
        inMsg = String(message.getSender()) + "," +
                String(message.getSensor()) + "," +
                parseToString(message) + ";";

        SerialPrintln("received: " + inMsg);
    }
}

String parseToString(const MyMessage &message) {
    String value = "error";
    switch (message.getPayloadType()) {
        case 0:  //Payload type is string
            value = message.getString();
            return value;
        case 1:  //Payload type is byte
            value = String(message.getByte());
            return value;
        case 2:  //Payload type is INT16
            value = String(message.getInt());
            return value;
        case 3:  //Payload type is UINT16
            value = String(message.getUInt());
            return value;
        case 4:  //Payload type is INT32
            value = String(message.getInt());
            return value;
        case 5:  //Payload type is UINT32
            value = String(message.getUInt());
            return value;
        case 6:  //Payload type is binary
            value = String(message.getBool());
            return value;
        case 7:  //Payload type is float32
            value = String(message.getFloat());
            return value;
        default:
            return value;
    }
}

void SerialPrintln(String text) {
#ifdef SERIAL_PRINT
    Serial.println(text);
#endif
}

//S_DOOR					  	     Door sensor, V_TRIPPED, V_ARMED
//S_MOTION				  	         Motion sensor, V_TRIPPED, V_ARMED
//S_SMOKE					  	     Smoke sensor, V_TRIPPED, V_ARMED
//S_BINARY				  	         Binary light or relay, V_STATUS, V_WATT
//S_LIGHT					  	     deprecated Same as S_BINARY
//S_DIMMER				  	         Dimmable light or fan device, V_STATUS (on/off), V_PERCENTAGE (dimmer level 0-100), V_WATT
//S_COVER					  	     Blinds or window cover, V_UP, V_DOWN, V_STOP, V_PERCENTAGE (open/close to a percentage)
//S_TEMP					  	     Temperature sensor, V_TEMP
//S_HUM					  	         Humidity sensor, V_HUM
//S_BARO					  	     Barometer sensor, V_PRESSURE, V_FORECAST
//S_WIND					  	     Wind sensor, V_WIND, V_GUST
//S_RAIN					  	     Rain sensor, V_RAIN, V_RAINRATE
//S_UV					  	         Uv sensor, V_UV
//S_WEIGHT				  	         Personal scale sensor, V_WEIGHT, V_IMPEDANCE
//S_POWER					  	     Power meter, V_WATT, V_KWH, V_VAR, V_VA, V_POWER_FACTOR
//S_HEATER				  	         Header device, V_HVAC_SETPOINT_HEAT, V_HVAC_FLOW_STATE, V_TEMP
//S_DISTANCE				  	     Distance sensor, V_DISTANCE
//S_LIGHT_LEVEL			  	         Light level sensor, V_LIGHT_LEVEL (uncalibrated in percentage),  V_LEVEL (light level in lux)
//S_ARDUINO_NODE			  	     Used (internally) for presenting a non-repeating Arduino node
//S_ARDUINO_REPEATER_NODE	  	     Used (internally) for presenting a repeating Arduino node
//S_LOCK					  	     Lock device, V_LOCK_STATUS
//S_IR					  	         IR device, V_IR_SEND, V_IR_RECEIVE
//S_WATER					  	     Water meter, V_FLOW, V_VOLUME
//S_AIR_QUALITY			  	         Air quality sensor, V_LEVEL
//S_CUSTOM				  	         Custom sensor
//S_DUST					  	     Dust sensor, V_LEVEL
//S_SCENE_CONTROLLER		  	     Scene controller device, V_SCENE_ON, V_SCENE_OFF.
//S_RGB_LIGHT				  	     RGB light. Send color component data using V_RGB. Also supports V_WATT
//S_RGBW_LIGHT			  	         RGB light with an additional White component. Send data using V_RGBW. Also supports V_WATT
//S_COLOR_SENSOR			  	     Color sensor, send color information using V_RGB
//S_HVAC					  	     Thermostat/HVAC device. V_HVAC_SETPOINT_HEAT, V_HVAC_SETPOINT_COLD, V_HVAC_FLOW_STATE, V_HVAC_FLOW_MODE, V_TEMP
//S_MULTIMETER			  	         Multimeter device, V_VOLTAGE, V_CURRENT, V_IMPEDANCE
//S_SPRINKLER				  	     Sprinkler, V_STATUS (turn on/off), V_TRIPPED (if fire detecting device)
//S_WATER_LEAK			  	         Water leak sensor, V_TRIPPED, V_ARMED
//S_SOUND					  	     Sound sensor, V_TRIPPED, V_ARMED, V_LEVEL (sound level in dB)
//S_VIBRATION				  	     Vibration sensor, V_TRIPPED, V_ARMED, V_LEVEL (vibration in Hz)
//S_MOISTURE				  	     Moisture sensor, V_TRIPPED, V_ARMED, V_LEVEL (water content or moisture in percentage?)
//S_INFO					  	     LCD text device / Simple information device on controller, V_TEXT
//S_GAS					  	         Gas meter, V_FLOW, V_VOLUME
//S_GPS					  	         GPS Sensor, V_POSITION
//S_WATER_QUALITY			  	     V_TEMP, V_PH, V_ORP, V_EC, V_STATUS
