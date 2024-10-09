// Contribution from Bfreak : https://github.com/Bfreak
// https://github.com/Xinyuan-LilyGO/LilyGo-T-SIM7080G/pull/71

#include <Arduino.h>
#include "utilities.h"  // Include the utilities header
#include <vector>
#include <SPI.h>
#include <FS.h>
#include <SD_MMC.h>
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS
#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
XPowersPMU  PMU;

bool firstChunk = true;

#include <algorithm> // Include the <algorithm> header for the std::min function

const size_t bufferSize = 10240; // 10KB buffer size
uint8_t buffer[bufferSize];

void sendFileToModem(File file, const char* filename) {
    modem.sendAT("+CFSTERM"); // Close FS in case it's still initialized
    if (modem.waitResponse() != 1) {
        Serial.println("Failed to terminate file system");
        return;
    }
    
    modem.sendAT("+CFSINIT"); // Initialize FS
    if (modem.waitResponse() != 1) {
        Serial.println("Failed to initialize file system");
        return;
    }

    // Get the total size of the file
    size_t totalSize = file.size();
    size_t alreadySent = 0;
    bool firstChunk = true;

    // Loop for sending chunks
    while (totalSize > 0) {
        // Determine the size of the next chunk to send
        size_t chunkSize = std::min(totalSize, static_cast<size_t>(10000)); // Limit chunk size to 10,000 bytes

        // Prepare the file upload command
        String command = "+CFSWFILE=0,\"" + String(filename) + "\"," + String(firstChunk ? 0 : 1) + "," + String(chunkSize) + ",10000";
        Serial.println(command); // For reference
        modem.sendAT(command.c_str()); // Send file upload command
        // if (modem.waitResponse(30000UL, "ERROR") == 1) { // Wait for modem confirmation
        //     Serial.println("Modem did not respond with DOWNLOAD");
        //     return;
        // }

        // Write the chunk of data to the modem
        size_t bytesRead = file.read(buffer, std::min(chunkSize, bufferSize)); // Read chunkSize bytes from the file
        if (bytesRead > 0) {
            size_t bytesWritten = modem.stream.write(buffer, bytesRead); // Write the read data to the modem's stream
            if (bytesWritten != bytesRead) {
                Serial.println("Failed to write chunk to modem");
                return;
            }
            alreadySent += bytesWritten;
            totalSize -= bytesWritten;

            Serial.printf("Sent %d bytes, %d bytes remaining\n", bytesWritten, totalSize);
        } else {
            Serial.println("Failed to read chunk from file");
            return;
        }

        firstChunk = false; // Update the flag after the first chunk
    }

    Serial.println("File upload completed");

    // Terminate file system after sending the file
    modem.sendAT("+CFSTERM");
    if (modem.waitResponse() != 1) {
        Serial.println("Failed to terminate file system after sending the file");
        return;
    }
}




void setup() {
    Serial.begin(115200);

    // Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

    Serial.println();

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    if (!PMU.begin(Wire1, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }
    
    // Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    // SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    // Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      // The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disabled, otherwise it cannot be charged
    PMU.disableTSPinMeasure();

    /*********************************
     * step 2 : start modemAT
    ***********************************/

    Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);

    int retry = 0;
    while (!modem.testAT(1000)) {
        Serial.print(".");
        if (retry++ > 6) {
            // Pull down PWRKEY for more than 1 second according to manual requirements
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            delay(100);
            digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
            delay(1000);
            digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
            retry = 0;
            Serial.println("Retry start modem .");
        }
    }

    // Initialize SD card with specific pins
    PMU.setALDO3Voltage(3300);   // SD Card VDD 3300
    PMU.enableALDO3();

    // TS Pin detection must be disabled, otherwise it cannot be charged
    PMU.disableTSPinMeasure();

    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);   //set sdcard pin use 1-bit mode

    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("Card Mount Failed");
        while (1) {
            delay(1000);
        }

    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
        while (1) {
            delay(1000);
        }
    }

  File imageFile = SD_MMC.open("/camimage.jpg");
  if (imageFile) {
    Serial.println("Image opened, sending!");
    sendFileToModem(imageFile, "camimage.jpg");
    imageFile.close();
  } else {
    Serial.println("Failed to open image file!");
  }
}

void loop() {
}

