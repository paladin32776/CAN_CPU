#include "CAN_CPU.h"

CAN_CPU::CAN_CPU(uint16_t id)
{
  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NORMAL);
  Serial.printf("TX queue length: %d\n", g_config.tx_queue_len);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    Serial.println("Driver installed");
  else
  {
    Serial.println("Failed to install driver");
    return;
  }
  // Start TWAI driver
  if (twai_start() == ESP_OK)
    Serial.println("Driver started");
  else
  {
    Serial.println("Failed to start driver");
    return;
  }
  // Reconfigure alerts
  uint32_t alerts_to_enable = TWAI_ALERT_TX_IDLE | TWAI_ALERT_TX_SUCCESS |
    TWAI_ALERT_TX_FAILED | TWAI_ALERT_RX_DATA | TWAI_ALERT_RX_QUEUE_FULL |
    TWAI_ALERT_BUS_ERROR | TWAI_ALERT_ERR_PASS;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK)
    Serial.println("CAN Alerts reconfigured");
  else
  {
    Serial.println("Failed to reconfigure alerts");
    return;
  }
  // Set CAN id
  can_id = id;
  // TWAI driver is now successfully installed and started
  driver_installed = true;
  // Set board alive ping interval
  etp_board_alive = new EnoughTimePassed(BOARD_ALIVE);
}

void CAN_CPU::check()
{
  if (driver_installed)
  {
    // Check if alert happened
    twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));
    // Check if message is received
    twai_get_status_info(&twaistatus);
    // Serial.printf("RXbuf=%d | Rx alert: %d\n", twaistatus.msgs_to_rx, (alerts_triggered & TWAI_ALERT_RX_DATA));
    // if (alerts_triggered & TWAI_ALERT_RX_DATA)
    if (twaistatus.msgs_to_rx>0)
    {
      twai_get_status_info(&twaistatus);
      Serial.printf("RXbuf=%d | ", twaistatus.msgs_to_rx);
      // One or more messages received. Handle first one.
      twai_message_t message;
      if (twai_receive(&message, 0) == ESP_OK)
        handle_rx_message(message);
      twai_get_status_info(&twaistatus);
      Serial.printf(" | RXbuf2: %d\n", twaistatus.msgs_to_rx);
    }
  }
}

bool CAN_CPU::send_message(uint8_t cmd)
{
  twai_message_t message = {.extd = 0, .rtr = 0, .ss = 0, .dlc_non_comp = 0,
    .identifier = can_id, .data_length_code = 1, .data = {cmd}};
  if (driver_installed)
    return (twai_transmit(&message, pdMS_TO_TICKS(TRANSMIT_TIMEOUT)) == ESP_OK);
}

bool CAN_CPU::send_message(uint8_t cmd, uint8_t val)
{
  if (driver_installed)
  {
    // Send message
    twai_message_t message = {.extd = 0, .rtr = 0, .ss = 0, .dlc_non_comp = 0,
      .identifier = can_id, .data_length_code = 2, .data = {cmd, val}};
    // Queue message for transmission
    return (twai_transmit(&message, pdMS_TO_TICKS(TRANSMIT_TIMEOUT)) == ESP_OK);
  }
}

void CAN_CPU::handle_rx_message(twai_message_t& message)
{
  // Process received message
  if (message.extd)
    Serial.print("EXT | ");
  else
    Serial.print("STD | ");
  Serial.printf("ID %x | LEN %d | RTR %d | DATA ",
    message.identifier, message.data_length_code,  message.rtr);
  rx_length = message.data_length_code;
  if (!(message.rtr) && rx_length>0)
  {
    for (int i = 0; i < rx_length; i++)
    {
      Serial.printf("%02x ", message.data[i]);
      data[i] = message.data[i];
    }
    rx_available = true;
  }
  // Serial.println("");
}

void CAN_CPU::get_message(uint8_t& cmd)
{
  cmd = data[0];
  rx_available = false;
}

void CAN_CPU::get_message(uint8_t& cmd, uint8_t& para)
{
  cmd = data[0];
  para = data[1];
  rx_available = false;
}

void CAN_CPU::get_message(uint8_t& cmd, uint8_t& para, uint8_t& para2)
{
  cmd = data[0];
  para = data[1];
  para2 = data[2];
  rx_available = false;
}

uint8_t CAN_CPU::message_length()
{
  return rx_length;
}

bool CAN_CPU::message_available()
{
  return rx_available;
}

void CAN_CPU::clear_message()
{
  rx_available = false;
}
