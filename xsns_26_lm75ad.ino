/*
  xsns_26_lm75ad.ino - Support for I2C LM75AD Temperature Sensor
*/

#ifdef USE_I2C
#ifdef USE_LM75AD

/*********************************************************************************************\
 * LM75AD - Temperature
 *
 * Docs at https://www.nxp.com/docs/en/data-sheet/LM75A.pdf
 *
 * I2C Address: 0x48 - 0x4F
\*********************************************************************************************/

#define LM75AD_ADDRESS1					0x48
#define LM75AD_ADDRESS2					0x49
#define LM75AD_ADDRESS3					0x4A
#define LM75AD_ADDRESS4					0x4B
#define LM75AD_ADDRESS5					0x4C
#define LM75AD_ADDRESS6					0x4D
#define LM75AD_ADDRESS7					0x4E
#define LM75AD_ADDRESS8					0x4F

#define LM75_TEMP_REGISTER      0x00
#define LM75_CONF_REGISTER      0x01
#define LM75_THYST_REGISTER     0x02
#define LM75_TOS_REGISTER       0x03

uint8_t lm75ad_type = 0;
uint8_t lm75ad_address;
uint8_t lm75ad_addresses[] = { LM75AD_ADDRESS1, LM75AD_ADDRESS2, LM75AD_ADDRESS3, LM75AD_ADDRESS4, LM75AD_ADDRESS5, LM75AD_ADDRESS6, LM75AD_ADDRESS7, LM75AD_ADDRESS8 };

void LM75ADDetect()
{
  uint8_t buffer;

  if (lm75ad_type) { return; }

  for (byte i = 0; i < sizeof(lm75ad_addresses); i++) {
    lm75ad_address = lm75ad_addresses[i];
    if (I2cValidRead8(&buffer, lm75ad_address, LM75_CONF_REGISTER)) {
      lm75ad_type = 1;
      snprintf_P(log_data, sizeof(log_data), S_LOG_I2C_FOUND_AT, "LM75AD", lm75ad_address);
      AddLog(LOG_LEVEL_DEBUG);
      break;
    }
  }
}

float LM75ADGetTemp() {
  int16_t sign = 1;

  uint16_t t = I2cRead16(lm75ad_address, LM75_TEMP_REGISTER);
  if (t & 0x8000) { // we are getting a negative temperature value
    t = (~t) +0x20;
    sign = -1;
  }
  t = t >> 5; // shift value into place (5 LSB not used)
  return ConvertTemp(sign * t * 0.125);
}

void LM75ADShow(boolean json)
{
  if (lm75ad_type) {
    char temperature[10];

    float t = LM75ADGetTemp();
    dtostrfd(t, Settings.flag2.temperature_resolution, temperature);

    if (json) {
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s,\"LM75AD\":{\"" D_JSON_TEMPERATURE "\":%s}"), mqtt_data, temperature);
#ifdef USE_DOMOTICZ
      if (0 == tele_period) DomoticzSensor(DZ_TEMP, temperature);
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
    } else {
      snprintf_P(mqtt_data, sizeof(mqtt_data), HTTP_SNS_TEMP, mqtt_data, "LM75AD", temperature, TempUnit());
#endif  // USE_WEBSERVER
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

#define XSNS_26

boolean Xsns26(byte function)
{
  boolean result = false;

  if (i2c_flg) {
    switch (function) {
      case FUNC_PREP_BEFORE_TELEPERIOD:
        LM75ADDetect();
        break;
      case FUNC_JSON_APPEND:
        LM75ADShow(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_APPEND:
        LM75ADShow(0);
        break;
#endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif  // USE_LM75AD
#endif  // USE_I2C
