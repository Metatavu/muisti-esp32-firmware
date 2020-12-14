#include <stdint.h>
#include <string.h>

enum MessageTypes {

  /* Get reader hardware version */
  GET_READER_HARDWARE_VERSION = 0x00,

  /* Get reader hardware version response */
  GET_READER_HARDWARE_VERSION_RESPONSE = 0x01,

  /* Get reader firmware version */
  GET_READER_FIRMWARE_VERSION = 0x02,

  /* Get reader firmware version response */
  GET_READER_FIRMWARE_VERSION_RESPONSE = 0x03,

  /* Get Unique ID */
  GET_UNIQUE_ID = 0x04,

  /* Get Unique ID response */
  GET_UNIQUE_ID_RESPONSE = 0x05,

  /* Reserve  */
  RESERVE = 0x06,

  /* Reserve  */
  RESERVE_0 = 0x0e,

  /* Heart hopping frame */
  HEART_HOPPING_FRAME = 0x0f,

  /* Set transmitting power */
  SET_TRANSMITTING_POWER = 0x10,

  /* Set transmitting power response */
  SET_TRANSMITTING_POWER_RESPONSE = 0x11,

  /* Get current transmitting power */
  GET_CURRENT_TRANSMITTING_POWER = 0x12,

  /* Get current transmitting power response */
  GET_CURRENT_TRANSMITTING_POWER_RESPONSE = 0x13,

  /* Frequency hopping setting */
  FREQUENCY_HOPPING_SETTING = 0x14,

  /* Frequency hopping setting response */
  FREQUENCY_HOPPING_SETTING_RESPONSE = 0x15,

  /* Get current equipment Frequency hopping setting status */
  GET_CURRENT_EQUIPMENT_FREQUENCY_HOPPING_SETTING_STATUS = 0x16,

  /* Get current equipment Frequency hopping setting status */
  GET_CURRENT_EQUIPMENT_FREQUENCY_HOPPING_SETTING_STATUS_0 = 0x17,

  /* response Set Gen2 data */
  RESPONSE_SET_GEN2_DATA = 0x20,

  /* Set current Gen2 data response */
  SET_CURRENT_GEN2_DATA_RESPONSE = 0x21,

  /* Get current Gen? data setting */
  GET_CURRENT_GEN_DATA_SETTING = 0x22,

  /* Get Gen2 data setting response */
  GET_GEN2_DATA_SETTING_RESPONSE = 0x23,

  /* CW setting */
  CW_SETTING = 0x24,

  /* CW setting response */
  CW_SETTING_RESPONSE = 0x25,

  /* Get current CW setting */
  GET_CURRENT_CW_SETTING = 0x26,

  /* Get current CW setting response */
  GET_CURRENT_CW_SETTING_RESPONSE = 0x27,

  /* Antenna setting */
  ANTENNA_SETTING = 0x28,

  /* Antenna setting response */
  ANTENNA_SETTING_RESPONSE = 0x29,

  /* Get current antenna setting */
  GET_CURRENT_ANTENNA_SETTING = 0x2a,

  /* Get current antenna setting response */
  GET_CURRENT_ANTENNA_SETTING_RESPONSE = 0x2b,

  /* Regional standard setting */
  REGIONAL_STANDARD_SETTING = 0x2c,

  /* Regional standard setting response */
  REGIONAL_STANDARD_SETTING_RESPONSE = 0x2d,

  /* Get Regional standard setting */
  GET_REGIONAL_STANDARD_SETTING = 0x2e,

  /* Get Regional standard setting response */
  GET_REGIONAL_STANDARD_SETTING_RESPONSE = 0x2f,

  /* Get port return loss */
  GET_PORT_RETURN_LOSS = 0x32,

  /* Get port return loss response */
  GET_PORT_RETURN_LOSS_RESPONSE = 0x33,

  /* Get current equipment temperature */
  GET_CURRENT_EQUIPMENT_TEMPERATURE = 0x34,

  /* Get current equipment temperature response */
  GET_CURRENT_EQUIPMENT_TEMPERATURE_RESPONSE = 0x35,

  /* Set temperature protection */
  SET_TEMPERATURE_PROTECTION = 0x38,

  /* Set temperature protection response */
  SET_TEMPERATURE_PROTECTION_RESPONSE = 0x39,

  /* Get temperature protection setting */
  GET_TEMPERATURE_PROTECTION_SETTING = 0x3a,

  /* Get temperature protection setting response */
  GET_TEMPERATURE_PROTECTION_SETTING_RESPONSE = 0x3b,

  /* Set continue inventory time */
  SET_CONTINUE_INVENTORY_TIME = 0x3c,

  /* Set continue inventory time response */
  SET_CONTINUE_INVENTORY_TIME_RESPONSE = 0x3d,

  /* Get continue inventory time setting */
  GET_CONTINUE_INVENTORY_TIME_SETTING = 0x3e,

  /* Get continue inventory time setting response */
  GET_CONTINUE_INVENTORY_TIME_SETTING_RESPONSE = 0x3f,

  /* Get error flag */
  GET_ERROR_FLAG = 0x40,

  /* Get error flag response */
  GET_ERROR_FLAG_RESPONSE = 0x41,

  /* Clear error flag */
  CLEAR_ERROR_FLAG = 0x42,

  /* Clear error flag response */
  CLEAR_ERROR_FLAG_RESPONSE = 0x43,

  /* Set GPIO */
  SET_GPIO = 0x46,

  /* Set GPIO response */
  SET_GPIO_RESPONSE = 0x47,

  /* Get GPIO */
  GET_GPIO = 0x48,

  /* Get GPIO response */
  GET_GPIO_RESPONSE = 0x49,

  /* Set working time of antenna */
  SET_WORKING_TIME_OF_ANTENNA = 0x4a,

  /* Set working time of antenna response */
  SET_WORKING_TIME_OF_ANTENNA_RESPONSE = 0x4b,

  /* Get working time of antenna */
  GET_WORKING_TIME_OF_ANTENNA = 0x4c,

  /* Get working time of antenna response */
  GET_WORKING_TIME_OF_ANTENNA_RESPONSE = 0x4d,

  /* Set idle time of switch antenna */
  SET_IDLE_TIME_OF_SWITCH_ANTENNA = 0x4e,

  /* Set idle time of switch antenna response */
  SET_IDLE_TIME_OF_SWITCH_ANTENNA_RESPONSE = 0x4f,

  /* Get idle time of switch antenna */
  GET_IDLE_TIME_OF_SWITCH_ANTENNA = 0x50,

  /* Get idle time of switch antenna response */
  GET_IDLE_TIME_OF_SWITCH_ANTENNA_RESPONSE = 0x51,

  /* Set recommend RF links */
  SET_RECOMMEND_RF_LINKS = 0x52,

  /* Set recommend RF links response */
  SET_RECOMMEND_RF_LINKS_RESPONSE = 0x53,

  /* Get recommend RF links */
  GET_RECOMMEND_RF_LINKS = 0x54,

  /* Get recommend RF links response */
  GET_RECOMMEND_RF_LINKS_RESPONSE = 0x55,

  /* Buzzer setting */
  BUZZER_SETTING = 0x56,

  /* Buzzer ringing response setting */
  BUZZER_RINGING_RESPONSE_SETTING = 0x57,

  /* Parameter setting of Ethernet interface */
  PARAMETER_SETTING_OF_ETHERNET_INTERFACE = 0x58,

  /* Parameter response setting of Ethernet interface */
  PARAMETER_RESPONSE_SETTING_OF_ETHERNET_INTERFACE = 0x59,

  /* Set WIFI parameter */
  SET_WIFI_PARAMETER = 0x5a,

  /* Set WIFI parameter response */
  SET_WIFI_PARAMETER_RESPONSE = 0x5b,

  /* FastID function setting */
  FASTID_FUNCTION_SETTING = 0x5c,

  /* FastID function response setting */
  FASTID_FUNCTION_RESPONSE_SETTING = 0x5d,

  /* Get FastID functional status response */
  GET_FASTID_FUNCTIONAL_STATUS_RESPONSE = 0x5f,

  /* Tagfocus function setting */
  TAGFOCUS_FUNCTION_SETTING = 0x60,

  /* Tagfocus function response setting */
  TAGFOCUS_FUNCTION_RESPONSE_SETTING = 0x61,

  /* Get tagfocus functional status */
  GET_TAGFOCUS_FUNCTIONAL_STATUS = 0x62,

  /* Get tagfocus functional status response */
  GET_TAGFOCUS_FUNCTIONAL_STATUS_RESPONSE = 0x63,

  /* Get environment RSSI value */
  GET_ENVIRONMENT_RSSI_VALUE = 0x64,

  /* Get environment RSSI value response */
  GET_ENVIRONMENT_RSSI_VALUE_RESPONSE = 0x65,

  /* Baud rate setting of modules */
  BAUD_RATE_SETTING_OF_MODULES = 0x66,

  /* Baud rate setting of modules response */
  BAUD_RATE_SETTING_OF_MODULES_RESPONSE = 0x67,

  /* Software reset */
  SOFTWARE_RESET = 0x68,

  /* Software reset response */
  SOFTWARE_RESET_RESPONSE = 0x69,

  /* Dual and Single mode setting */
  DUAL_AND_SINGLE_MODE_SETTING = 0x6a,

  /* Dual and Single mode setting response */
  DUAL_AND_SINGLE_MODE_SETTING_RESPONSE = 0x6b,

  /* Get Dual and Single mode */
  GET_DUAL_AND_SINGLE_MODE = 0x6c,

  /* Get Dual and Single mode response */
  GET_DUAL_AND_SINGLE_MODE_RESPONSE = 0x6d,

  /* Inventory filtering setting */
  INVENTORY_FILTERING_SETTING = 0x6e,

  /* Inventory filtering setting response */
  INVENTORY_FILTERING_SETTING_RESPONSE = 0x6f,

  /* Get the EPC and TID simultaneously mode setting */
  GET_THE_EPC_AND_TID_SIMULTANEOUSLY_MODE_SETTING = 0x70,

  /* Get the EPC and TID simultaneously mode setting response */
  GET_THE_EPC_AND_TID_SIMULTANEOUSLY_MODE_SETTING_RESPONSE = 0x71,

  /* Get the EPC and TID simultaneously mode setting status */
  GET_THE_EPC_AND_TID_SIMULTANEOUSLY_MODE_SETTING_STATUS = 0x72,

  /* Get the EPC and TID simultaneously mode setting status */
  GET_THE_EPC_AND_TID_SIMULTANEOUSLY_MODE_SETTING_STATUS_0 = 0x73,

  /* response Factory default setting */
  RESPONSE_FACTORY_DEFAULT_SETTING = 0x74,

  /* Factory default setting response */
  FACTORY_DEFAULT_SETTING_RESPONSE = 0x75,

  /* Set inventory mode */
  SET_INVENTORY_MODE = 0x76,

  /* Set inventory mode response */
  SET_INVENTORY_MODE_RESPONSE = 0x77,

  /* Get inventory mode status */
  GET_INVENTORY_MODE_STATUS = 0x78,

  /* Get inventory mode status response */
  GET_INVENTORY_MODE_STATUS_RESPONSE = 0x79,

  /* Set Sound and Light Mode */
  SET_SOUND_AND_LIGHT_MODE = 0x7a,

  /* Set Sound and Light Mode */
  SET_SOUND_AND_LIGHT_MODE_0 = 0x00,

  /* Set Sound and Light Mode response */
  SET_SOUND_AND_LIGHT_MODE_RESPONSE = 0x7b,

  /* Set Sound and Light Mode response */
  SET_SOUND_AND_LIGHT_MODE_RESPONSE_0 = 0x01,

  /* Get Sound and Light Mode setting status */
  GET_SOUND_AND_LIGHT_MODE_SETTING_STATUS = 0x7a,

  /* Get Sound and Light Mode setting status */
  GET_SOUND_AND_LIGHT_MODE_SETTING_STATUS_0 = 0x02,

  /* Get Sound and Light Mode setting status response */
  GET_SOUND_AND_LIGHT_MODE_SETTING_STATUS_RESPONSE = 0x7b,

  /* Get Sound and Light Mode setting status response */
  GET_SOUND_AND_LIGHT_MODE_SETTING_STATUS_RESPONSE_0 = 0x03,

  /* Set GPIO to trigger continue inventory */
  SET_GPIO_TO_TRIGGER_CONTINUE_INVENTORY = 0x7a,

  /* Set GPIO to trigger continue inventory */
  SET_GPIO_TO_TRIGGER_CONTINUE_INVENTORY_0 = 0x04,

  /* Set GPIO to trigger continue inventory response */
  SET_GPIO_TO_TRIGGER_CONTINUE_INVENTORY_RESPONSE = 0x7b,

  /* Set GPIO to trigger continue inventory response */
  SET_GPIO_TO_TRIGGER_CONTINUE_INVENTORY_RESPONSE_0 = 0x05,

  /* Get GPIO status to trigger continue inventory */
  GET_GPIO_STATUS_TO_TRIGGER_CONTINUE_INVENTORY = 0x7a,

  /* Get GPIO status to trigger continue inventory */
  GET_GPIO_STATUS_TO_TRIGGER_CONTINUE_INVENTORY_0 = 0x06,

  /* Get GPIO status to trigger continue inventory respone */
  GET_GPIO_STATUS_TO_TRIGGER_CONTINUE_INVENTORY_RESPONE = 0x7b,

  /* Get GPIO status to trigger continue inventory respone */
  GET_GPIO_STATUS_TO_TRIGGER_CONTINUE_INVENTORY_RESPONE_0 = 0x07,

  /* Reserve  */
  RESERVE_1 = 0x7c,

  /* Reserve  */
  RESERVE_2 = 0x7f,

  /* Inventory for once */
  INVENTORY_FOR_ONCE = 0x80,

  /* Inventory for once response */
  INVENTORY_FOR_ONCE_RESPONSE = 0x81,

  /* Continue inventory */
  CONTINUE_INVENTORY = 0x82,

  /* Continue inventory response */
  CONTINUE_INVENTORY_RESPONSE = 0x83,

  /* Stop continue inventory */
  STOP_CONTINUE_INVENTORY = 0x8c,

  /* Stop continue inventory response */
  STOP_CONTINUE_INVENTORY_RESPONSE = 0x8d,

  /* Read data */
  READ_DATA = 0x84,

  /* Read data response */
  READ_DATA_RESPONSE = 0x85,

  /* Write data */
  WRITE_DATA = 0x86,

  /* Write data response */
  WRITE_DATA_RESPONSE = 0x87,

  /* Lock tag */
  LOCK_TAG = 0x88,

  /* Lock tag response */
  LOCK_TAG_RESPONSE = 0x89,

  /* Kill tag */
  KILL_TAG = 0x8a,

  /* Kill tag response */
  KILL_TAG_RESPONSE = 0x8b,

  /* Reserve */
  RESERVE_3 = 0x8e,

  /* Reserve */
  RESERVE_4 = 0x8f,

  /* Time frame inventory */
  TIME_FRAME_INVENTORY = 0x90,

  /* Time frame inventory response */
  TIME_FRAME_INVENTORY_RESPONSE = 0x91,

  /* Get time frame inventory result */
  GET_TIME_FRAME_INVENTORY_RESULT = 0x92,

  /* Block write tags */
  BLOCK_WRITE_TAGS = 0x93,

  /* Block write tags response */
  BLOCK_WRITE_TAGS_RESPONSE = 0x94,

  /* Block erase tags */
  BLOCK_ERASE_TAGS = 0x95,

  /* Block erase tags response */
  BLOCK_ERASE_TAGS_RESPONSE = 0x96,

  /* Set QT command parameter */
  SET_QT_COMMAND_PARAMETER = 0x97,

  /* Set QT command parameter response */
  SET_QT_COMMAND_PARAMETER_RESPONSE = 0x98,

  /* Get QT command parameter */
  GET_QT_COMMAND_PARAMETER = 0x99,

  /* Get QT command parameter response */
  GET_QT_COMMAND_PARAMETER_RESPONSE = 0x9a,

  /* QT read operation */
  QT_READ_OPERATION = 0x9b,

  /* QT read operation response */
  QT_READ_OPERATION_RESPONSE = 0x9c,

  /* QT write operation */
  QT_WRITE_OPERATION = 0x9d,

  /* QT write operation response */
  QT_WRITE_OPERATION_RESPONSE = 0x9e,

  /* BlockPermalock operation */
  BLOCKPERMALOCK_OPERATION = 0x9f,

  /* BlockPermalock Operation response */
  BLOCKPERMALOCK_OPERATION_RESPONSE = 0xa0,

  /* Untraceable operation */
  UNTRACEABLE_OPERATION = 0xa1,

  /* Untraceable operation response */
  UNTRACEABLE_OPERATION_RESPONSE = 0xa2,

  /* Authenticat operation */
  AUTHENTICAT_OPERATION = 0xa3,

  /* Authenticat operation response */
  AUTHENTICAT_OPERATION_RESPONSE = 0xa4,

  /* Reserve  */
  RESERVE_5 = 0xa5,

  /* Reserve  */
  RESERVE_6 = 0xf3,

  /* Antenna detection */
  ANTENNA_DETECTION = 0xf4,

  /* Antenna detection response */
  ANTENNA_DETECTION_RESPONSE = 0xf5,

  /* Reserve  */
  RESERVE_7 = 0xf6,

  /* Reserve  */
  RESERVE_8 = 0xfe,

  /* Operation fail response */
  OPERATION_FAIL_RESPONSE = 0xff
};