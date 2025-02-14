
#define SERIAL_PACKAGE_MAX_LENGTH 33
char SerialBuffer[SERIAL_PACKAGE_MAX_LENGTH];

const char HELP_STRING [] PROGMEM = "F - set Frequency in Hz (100000 - 500000000)\n"
          "H - Set HSTL Output: 0 - OFF, 1 - ON, 2 - Doubler\n"
          "C - Set CMOS Output: 0 - OFF, 1 - ON\n"
          "D - Set CMOS Divider (1 - 65353)\n"
          "P - Set Output Power in dBm (-7 - +4)\n"
          "M - Get Model\n"
          "V - Get Firmware Version\n"
          "E - Enable Outputs (ALL)\n"
          "S - Shut down Outputs (ALL)\n"
          "h - This Help\n"
          "; - Commands Separator"
          "\n"
          "Example:\n"
          "F100000;P-2\n"
          "Set Frequency to 100 kHz, and Output Power to -2 dBm.\n"
          "Any number of commands in any order is allowed.";

void ReadSerialCommands()
{
  if (!Serial.available()) return;
  int RcvCounter=0;
  RcvCounter = Serial.readBytesUntil('\n', SerialBuffer, 110);
  if (RcvCounter == 0) return;
  SerialBuffer[RcvCounter]='\0';

  int32_t value=0;
  char command;

  GParser data(SerialBuffer, ';');
  int commandsCounter = data.split();

  for (int i=0; i < commandsCounter; i++)
  {
    sscanf(data[i], "%c%ld", &command, &value);
    switch (command)
    {
      case 'F': //RF Frequency
        if (inRange(value, LOW_FREQ_LIMIT, HIGH_FREQ_LIMIT))
        {
          Serial.print(F("Set freq.: "));
          Serial.println(value);
          H = value % 1000;
          value = value / 1000;
          K = value % 1000;
          value = value / 1000;
          M = value; 
        } else Serial.println("Frequency is OUT OF RANGE (" + String(LOW_FREQ_LIMIT) + " - " + String(HIGH_FREQ_LIMIT) + ")");
      break;

      case 'H': //HSTL 0 - OFF, 1 - ON, 2 - Doubler
      if (inRange(value, 0, 2))
        {
          Serial.print(F("Set HSTL: "));
          Serial.println(value);
          HSTL_State = value;
        } else Serial.println(F("HSTL is OUT OF RANGE (0 - 2)"));
      break;

      case 'C': //CMOS 0 - OFF, 1 - ON
        if (inRange(value, 0, 1))
        {
          Serial.print(F("Set CMOS: "));
          Serial.println(value);
          CMOS_State = value;
        } else Serial.println(F("CMOS is OUT OF RANGE (0 - 1)"));
      break;

      case 'D': //Divder for CMOS output 1 - 65535
        if (inRange(value, CMOS_DIVIDER_MIN_VALUE, CMOS_DIVIDER_MAX_VALUE))
        {
          Serial.print(F("Set Divider for CMOS: "));
          Serial.println(value);
          CMOS_Divider_Hz = value % 1000;
          value = value / 1000;
          CMOS_Divider_KHz = value;
        } else Serial.println("Divider for CMOS is OUT OF RANGE (" + String(CMOS_DIVIDER_MIN_VALUE) + " - " + String(CMOS_DIVIDER_MAX_VALUE) + ")");
      break;

      case 'P': //Power, dBm -7 - +4
        if (inRange(value, -7, 4))
        {
          Serial.print(F("Set Power.: "));
          Serial.println(value);
          A = value + 7; //index shift in array
        } else Serial.println("Power is OUT OF RANGE (-7 - +4)");
      break;

      case 'S': //Shutdown
        Serial.println(F("Outputs was Shut down"));
        digitalWrite(PWR_DOWN_PIN, HIGH);
        isPWR_DWN = true;
      break;

      case 'E':
        Serial.println(F("Outputs Enabled"));
        digitalWrite(PWR_DOWN_PIN, LOW);
        isPWR_DWN = false;
      break;

      case 'V': //Firmware Version request
        Serial.println(FIRMWAREVERSION);
        //Serial.println(value);
      break;

      case 'M': //Model request
        Serial.println(F("DDS9912 v1.3"));
        //Serial.println(value);
      break;

      case 'h': //Model request
        Serial.println((const __FlashStringHelper *) HELP_STRING);
      break;

      default:
      Serial.print(F("Unknown command:"));
      Serial.println(command);
      Serial.println((const __FlashStringHelper *) HELP_STRING);
    } //switch
  } //for

MakeOut();
}

bool inRange(int32_t val, int32_t minimum, int32_t maximum)
{
  return ((minimum <= val) && (val <= maximum));
}