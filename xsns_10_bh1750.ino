/*
  xsns_10_bh1750.ino - BH1750 ambient light sensor support for Sonoff-Tasmota
*/

#ifdef USE_I2C
#ifdef USE_BH1750
/*********************************************************************************************\
 * BH1750 - Ambient Light Intensity
 *
 * I2C Address: 0x23 or 0x5C
\*********************************************************************************************/

#define BH1750_ADDR1         0x23
#define BH1750_ADDR2         0x5C

#define BH1750_CONTINUOUS_HIGH_RES_MODE 0x10 // Start measurement at 1lx resolution. Measurement time is approx 120ms.

uint8_t bh1750_address;
uint8_t bh1750_addresses[] = { BH1750_ADDR1, BH1750_ADDR2 };
uint8_t bh1750_type = 0;

uint16_t Bh1750ReadLux()
{
  Wire.requestFrom(bh1750_address, (uint8_t)2);
  byte msb = Wire.read();
  byte lsb = Wire.read();
  uint16_t value = ((msb << 8) | lsb) / 1.2;
  return value;
}

/********************************************************************************************/

void Bh1750Detect()
{
  if (bh1750_type) {
    return;
  }

  for (byte i = 0; i < sizeof(bh1750_addresses); i++) {
    bh1750_address = bh1750_addresses[i];
    Wire.beginTransmission(bh1750_address);
    Wire.write(BH1750_CONTINUOUS_HIGH_RES_MODE);
    if (!Wire.endTransmission()) {
      bh1750_type = 1;
      snprintf_P(log_data, sizeof(log_data), S_LOG_I2C_FOUND_AT, "BH1750", bh1750_address);
      AddLog(LOG_LEVEL_DEBUG);
      break;
    }
  }
}

#ifdef USE_WEBSERVER
const char HTTP_SNS_ILLUMINANCE[] PROGMEM =
  "%s{s}BH1750 " D_ILLUMINANCE "{m}%d " D_UNIT_LUX "{e}";  // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
#endif  // USE_WEBSERVER

void Bh1750Show(boolean json)
{
  if (bh1750_type) {
    uint16_t illuminance = Bh1750ReadLux();

    if (json) {
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s,\"BH1750\":{\"" D_JSON_ILLUMINANCE "\":%d}"), mqtt_data, illuminance);
#ifdef USE_DOMOTICZ
      if (0 == tele_period) DomoticzSensor(DZ_ILLUMINANCE, illuminance);
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
    } else {
      snprintf_P(mqtt_data, sizeof(mqtt_data), HTTP_SNS_ILLUMINANCE, mqtt_data, illuminance);
#endif  // USE_WEBSERVER
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

#define XSNS_10

boolean Xsns10(byte function)
{
  boolean result = false;

  if (i2c_flg) {
    switch (function) {
      case FUNC_PREP_BEFORE_TELEPERIOD:
        Bh1750Detect();
        break;
      case FUNC_JSON_APPEND:
        Bh1750Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_APPEND:
        Bh1750Show(0);
        break;
#endif  // USE_WEBSERVER
    }
  }
  return result;
}

#endif  // USE_BH1750
#endif  // USE_I2C
