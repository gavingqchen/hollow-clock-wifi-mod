#include "EepromUtility.h"
#include "WifiUtility.h"

struct EEPROMStruct eepUserSet;

void InitializeEeprom()
{

    EEPROM.begin(sizeof(EEPROMStruct));
    EEPROM.get(eeprom_address0, eepUserSet);

    if (eepUserSet.isInitialized != 1)
    {
        eepUserSet.isInitialized = 1; 
        eepUserSet.clockCompMsPerMin = 0;     
        eepUserSet.outputPower = 18.0;         
        eepUserSet.clockCalInvervalHour = 12;         


        EEPROM.put(eeprom_address0, eepUserSet);
        EEPROM.commitReset(); 
        Serial.println("EEPROM put");
    }
    else
    {
        EEPROM.get(eeprom_address0, eepUserSet);
        if (eepUserSet.outputPower < 10)
            eepUserSet.outputPower = 18.0; 
        Serial.println("EEPROM get");
    }
    
    // Serial.print(EEPROM.percentUsed());
    // Serial.println("% Used space.");

    // Serial.print("eepUserSet.clockCompMsPerMin:");
    // Serial.println(eepUserSet.clockCompMsPerMin);
    // Serial.print("eepUserSet.outputPower:");
    // Serial.println(eepUserSet.outputPower);

}