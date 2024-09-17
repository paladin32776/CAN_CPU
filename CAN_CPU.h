#include "Arduino.h"
#include "driver/twai.h"
#include "EnoughTimePassed.h"

// Pins used to connect to CAN bus transceiver:
#define RX_PIN 4
#define TX_PIN 5

// Interval:
#define TRANSMIT_TIMEOUT 100
#define POLLING_RATE_MS 1
#define BOARD_ALIVE 300

// Commands:
#define DATA_CMD_BYTE 0x00
#define CTRL_CMD_BYTE 0x01
#define BOARDS_CMD_BYTE 0xFC
#define PONG_CMD_BYTE 0xFD
#define PING_CMD_BYTE 0xFE
#define RESET_CMD_BYTE 0xFF

// Board IDs:
#define BOARD_ID_ALU 1
#define BOARD_ID_CONTROL 2
#define BOARD_ID_PROGRAM 4
#define BOARD_ID_MANUAL 8

class CAN_CPU
{
	private:
		uint16_t can_id;
    bool driver_installed = false;
    uint32_t alerts_triggered;
    twai_status_info_t twaistatus;
    EnoughTimePassed* etp_board_alive;
    uint8_t data[8];
    uint8_t rx_length;
    bool rx_available = false;
    void handle_rx_message(twai_message_t& message);
	public:
		CAN_CPU(uint16_t id);
		void check();
    bool send_message(uint8_t cmd);
    bool send_message(uint8_t cmd, uint8_t val);
    void get_message(uint8_t& cmd);
    void get_message(uint8_t& cmd, uint8_t& para);
    void get_message(uint8_t& cmd, uint8_t& para, uint8_t& para2);
    uint8_t message_length();
    bool message_available();
    void clear_message();
};
