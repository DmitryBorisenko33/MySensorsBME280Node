//#define MY_DEBUG
//#define MY_BAUD_RATE 115200
#define SERIAL_PRINT
//#define MY_PASSIVE_NODE
//#define MY_NODE_ID 100
#define MY_RADIO_NRF5_ESB
//#define MY_TRANSPORT_WAIT_READY_MS 10
#include <MySensors.h>
#include <variant.h>

extern uint32_t sleepingPeriod;
extern uint16_t attamptsNumber;
extern String inMsg;
extern long totalErrors;

extern void sendMsgEchoAck(int nodeId, int ChildId, const mysensors_data_t dataType, float value, bool goToSleep);
extern void sendMsgFastAck(int ChildId, const mysensors_data_t dataType, float value, bool goToSleep);
extern void receive(const MyMessage &message);
extern String parseToString(const MyMessage &message);
extern void SerialPrintln(String text);
