
## Quick Step

* [SIM7080G-1951B17SIM7080](https://drive.google.com/file/d/1-m0eF53mw9n0vw4vfNZ88cRR3GM0T-a1/view?usp=sharing)

1. Update the built-in firmware of ESP32S3 using [MinimalModemUpgrade](../examples/MinimalModemUpgrade/MinimalModemUpgrade.ino) to ensure that the program is only responsible for enabling SIM7080G
2. Connect the two USB ports of the board, USB-C and Micro-USB, and operate according to the following image

3. Open [SIM7080 Driver](https://drive.google.com/file/d/1YGMAH57G7QRPPg9Vrx_PJX9Z7VdwxhIF/view?usp=sharing)
   
4. There are two ways to add drivers:

    (1)Open the computer device manager and follow the steps below to add the driver. 
    ![](../image/update_simxxxx_2.png)
    ![](../image/update_simxxxx_3.png)
    ![](../image/update_simxxxx_4.png)
    ![](../image/update_sim7080_5.png)
    ![](../image/update_simxxxx_6.png)

    (2) Open [SIMCOM_Windows_USB_Driver](https://drive.google.com/file/d/1YGMAH57G7QRPPg9Vrx_PJX9Z7VdwxhIF/view?usp=sharing)

    ![](../image/update_simxxxx_1_1.png)
    ![](../image/update_simxxxx_1_2.png)
    ![](../image/update_simxxxx_1_3.png)


    Follow the above steps to install the driver for the remaining ports that are not installed.
    ![](../image/update_simxxxx_7.png)

5. Download [upgrade_tool](https://drive.google.com/file/d/1nOqJuzBgE8KrMkpQUAWy3nJwqEBVABVn/view?usp=sharing) , The diagram is for version 1.57. The tool has been updated to 1.67. Please use the latest upgrade tool.
6. Open `sim7080_sim7500_sim7600_sim7900_sim8200 qdl v1.58 only for update.exe`
7. Open the upgrade tool and follow the diagram below 

    ![](../image/update_simxxxx_8.png)
    ![](../image/update_simxxxx_9.png)
    ![](../image/update_simxxxx_10.png)
    ![](../image/update_simxxxx_11.png)
    ![](../image/update_simxxxx_12.png)
    ![](../image/update_simxxxx_13.png)
    ![](../image/update_simxxxx_15.png)



8. Open the serial terminal tool, or the built-in serial tool of `Arduino IDE`, select `AT Port` for the port, and enter `AT+CGMR` to view the firmware version 
    ![](../image/update_simxxxx_14.png)



