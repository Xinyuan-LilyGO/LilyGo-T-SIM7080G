/**
 * @file      SY6970_Example.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-08-05
 * @note      SY6970 If the power supply is a separate USB power supply, VSYS can only provide a maximum load current of 500mA. 
 *            If it is connected to a battery, the discharge current depends on the maximum discharge current of the battery.
 *
 */
#include <XPowersLib.h>

PowersSY6970 PMU;


#ifndef CONFIG_PMU_SDA
#define CONFIG_PMU_SDA 0
#endif

#ifndef CONFIG_PMU_SCL
#define CONFIG_PMU_SCL 1
#endif

#ifndef CONFIG_PMU_IRQ
#define CONFIG_PMU_IRQ 28
#endif

const uint8_t i2c_sda = CONFIG_PMU_SDA;
const uint8_t i2c_scl = CONFIG_PMU_SCL;
const uint8_t pmu_irq_pin = CONFIG_PMU_IRQ;
uint32_t cycleInterval;
bool pmu_irq = false;

void setup()
{
    Serial.begin(115200);
    while (!Serial);


    bool result =  PMU.init(Wire, i2c_sda, i2c_scl, SY6970_SLAVE_ADDRESS);

    if (result == false) {
        while (1) {
            Serial.println("PMU is not online...");
            delay(50);
        }
    }

    // Set the minimum operating voltage. Below this voltage, the PMU will protect
    PMU.setSysPowerDownVoltage(3300);

    // Set input current limit, default is 500mA
    PMU.setInputCurrentLimit(3250);

    Serial.printf("getInputCurrentLimit: %d mA\n",PMU.getInputCurrentLimit());

    // Disable current limit pin
    PMU.disableCurrentLimitPin();

    // Set the charging target voltage, Range:3840 ~ 4608mV ,step:16 mV
    PMU.setChargeTargetVoltage(4208);

    // Set the precharge current , Range: 64mA ~ 1024mA ,step:64mA
    PMU.setPrechargeCurr(64);

    // The premise is that Limit Pin is disabled, or it will only follow the maximum charging current set by Limi tPin.
    // Set the charging current , Range:0~5056mA ,step:64mA
    PMU.setChargerConstantCurr(832);

    // Get the set charging current
    PMU.getChargerConstantCurr();
    Serial.printf("getChargerConstantCurr: %d mA\n",PMU.getChargerConstantCurr());


    // To obtain voltage data, the ADC must be enabled first
    PMU.enableADCMeasure();
    
    // Turn on charging function
    // If there is no battery connected, do not turn on the charging function
    PMU.enableCharge();

    // Turn off charging function
    // If USB is used as the only power input, it is best to turn off the charging function, 
    // otherwise the VSYS power supply will have a sawtooth wave, affecting the discharge output capability.
    // PMU.disableCharge();


    // The OTG function needs to enable OTG, and set the OTG control pin to HIGH
    // After OTG is enabled, if an external power supply is plugged in, OTG will be turned off

    // PMU.enableOTG();
    // PMU.disableOTG();
    // pinMode(OTG_ENABLE_PIN, OUTPUT);
    // digitalWrite(OTG_ENABLE_PIN, HIGH);


    attachInterrupt(pmu_irq_pin, []() {
        pmu_irq = true;
    }, FALLING);

    delay(2000);
}


void loop()
{

    if (pmu_irq) {
        pmu_irq = false;
        Serial.print("-> [");
        Serial.print(millis() / 1000);
        Serial.print("] ");

        // Get PMU interrupt status
        PMU.getIrqStatus();


        if (PMU.isWatchdogFault()) {
            Serial.println("Watchdog Fault");
        }
        if (PMU.isBoostFault()) {
            Serial.println("Boost Fault");
        }
        if (PMU.isChargeFault()) {
            Serial.println("Charge Fault");
        }
        if (PMU.isBatteryFault()) {
            Serial.println("Batter Fault");
        }
        if (PMU.isNTCFault()) {
            Serial.print("NTC Fault:");
            Serial.print(PMU.getNTCStatusString());

            Serial.print(" Percentage:");
            Serial.print(PMU.getNTCPercentage()); Serial.println("%");
        }   
        // The battery may be disconnected or damaged.
        if (PMU.isVsysLowVoltageWarning()) {
            Serial.println("In VSYSMIN regulation (BAT<VSYSMIN)");
        }

    }

    // SY6970 When VBUS is input, the battery voltage detection will not take effect
    if (millis() > cycleInterval) {

        Serial.println("Sats        VBUS    VBAT   SYS    VbusStatus      String   ChargeStatus     String      TargetVoltage       ChargeCurrent       Precharge       NTCStatus           String");
        Serial.println("            (mV)    (mV)   (mV)   (HEX)                         (HEX)                    (mV)                 (mA)                   (mA)           (HEX)           ");
        Serial.println("--------------------------------------------------------------------------------------------------------------------------------");
        Serial.print(PMU.isVbusIn() ? "Connected" : "Disconnect"); Serial.print("\t");
        Serial.print(PMU.getVbusVoltage()); Serial.print("\t");
        Serial.print(PMU.getBattVoltage()); Serial.print("\t");
        Serial.print(PMU.getSystemVoltage()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(PMU.getBusStatus(), HEX); Serial.print("\t");
        Serial.print(PMU.getBusStatusString()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(PMU.chargeStatus(), HEX); Serial.print("\t");
        Serial.print(PMU.getChargeStatusString()); Serial.print("\t");

        Serial.print(PMU.getChargeTargetVoltage()); Serial.print("\t");
        Serial.print(PMU.getChargeCurrent()); Serial.print("\t");
        Serial.print(PMU.getPrechargeCurr()); Serial.print("\t");
        Serial.print(PMU.getNTCStatus()); Serial.print("\t");
        Serial.print(PMU.getNTCStatusString()); Serial.print("\t");


        Serial.println();
        Serial.println();
        cycleInterval = millis() + 1000;
    }

}





