/*
  xsns_01_counter.ino - Counter sensors (water meters, electricity meters etc.) sensor support for Sonoff-Tasmota
*/

/*********************************************************************************************\
 * Counter sensors (water meters, electricity meters etc.)
\*********************************************************************************************/

unsigned long last_counter_timer[MAX_COUNTERS]; // Last counter time in milli seconds

void CounterUpdate(byte index)
{
  unsigned long counter_debounce_time = millis() - last_counter_timer[index -1];
  if (counter_debounce_time > Settings.pulse_counter_debounce) {
    last_counter_timer[index -1] = millis();
    if (bitRead(Settings.pulse_counter_type, index -1)) {
      RtcSettings.pulse_counter[index -1] = counter_debounce_time;
    } else {
      RtcSettings.pulse_counter[index -1]++;
    }

//    snprintf_P(log_data, sizeof(log_data), PSTR("CNTR: Interrupt %d"), index);
//    AddLog(LOG_LEVEL_DEBUG);
  }
}

void CounterUpdate1()
{
  CounterUpdate(1);
}

void CounterUpdate2()
{
  CounterUpdate(2);
}

void CounterUpdate3()
{
  CounterUpdate(3);
}

void CounterUpdate4()
{
  CounterUpdate(4);
}

/********************************************************************************************/

void CounterSaveState()
{
  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      Settings.pulse_counter[i] = RtcSettings.pulse_counter[i];
    }
  }
}

void CounterInit()
{
  typedef void (*function) () ;
  function counter_callbacks[] = { CounterUpdate1, CounterUpdate2, CounterUpdate3, CounterUpdate4 };

  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      pinMode(pin[GPIO_CNTR1 +i], INPUT_PULLUP);
      attachInterrupt(pin[GPIO_CNTR1 +i], counter_callbacks[i], FALLING);
    }
  }
}

#ifdef USE_WEBSERVER
const char HTTP_SNS_COUNTER[] PROGMEM =
  "%s{s}" D_COUNTER "%d{m}%s%s{e}";  // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
#endif  // USE_WEBSERVER

void CounterShow(boolean json)
{
  char stemp[10];
  char counter[16];

  byte dsxflg = 0;
  byte header = 0;
  for (byte i = 0; i < MAX_COUNTERS; i++) {
    if (pin[GPIO_CNTR1 +i] < 99) {
      if (bitRead(Settings.pulse_counter_type, i)) {
        dtostrfd((double)RtcSettings.pulse_counter[i] / 1000, 3, counter);
      } else {
        dsxflg++;
        dtostrfd(RtcSettings.pulse_counter[i], 0, counter);
      }

      if (json) {
        if (!header) {
          snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s,\"COUNTER\":{"), mqtt_data);
          stemp[0] = '\0';
        }
        header++;
        snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s%s\"C%d\":%s"), mqtt_data, stemp, i +1, counter);
        strcpy(stemp, ",");
#ifdef USE_DOMOTICZ
        if ((0 == tele_period) && (1 == dsxflg)) {
          DomoticzSensor(DZ_COUNT, RtcSettings.pulse_counter[i]);
          dsxflg++;
        }
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
      } else {
        snprintf_P(mqtt_data, sizeof(mqtt_data), HTTP_SNS_COUNTER, mqtt_data, i +1, counter, (bitRead(Settings.pulse_counter_type, i)) ? " " D_UNIT_SECOND : "");
#endif  // USE_WEBSERVER
      }
    }
  }
  if (json) {
    if (header) {
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s}"), mqtt_data);
    }
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

#define XSNS_01

boolean Xsns01(byte function)
{
  boolean result = false;

  switch (function) {
    case FUNC_INIT:
      CounterInit();
      break;
    case FUNC_JSON_APPEND:
      CounterShow(1);
      break;
#ifdef USE_WEBSERVER
    case FUNC_WEB_APPEND:
      CounterShow(0);
      break;
#endif  // USE_WEBSERVER
    case FUNC_SAVE_BEFORE_RESTART:
      CounterSaveState();
      break;
  }
  return result;
}
