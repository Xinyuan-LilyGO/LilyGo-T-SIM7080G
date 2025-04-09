## 1. News

* The firmware supports any one in the list below. The firmware used in the list below is the same and there is no need to distinguish it.

## 2. How to Flash ?

### Use ESP Download Tool

- Download [Flash_download_tool](https://www.espressif.com.cn/sites/default/files/tools/flash_download_tool_3.9.7_0.zip)

![web_flasher](./images/esp_downloader.gif)

* Note that after writing is completed, you need to press RST to reset.

### Use Web Flasher

- [ESP Web Flasher Online](https://espressif.github.io/esptool-js/)

![web_flasher](./images/web_flasher.gif)

* Note that after writing is completed, you need to press RST to reset.

### Use command line

If system asks about install Developer Tools, do it.

```bash
python3 -m pip install --upgrade pip
python3 -m pip install esptool
```

In order to launch esptool.py, exec directly with this:

```bash
python3 -m esptool
```

```bash
esptool --chip esp32s3 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m 0x0 firmware_file.bin
```

## 3. FAQ

Can't upload any sketch, please enter the upload mode manually.

1. Connect the board via the USB cable
2. Press and hold the **BOOT** button, While still pressing the **BOOT** button, **Not SBOOT**
3. Press **RST** button
4. Release the **RST** button
5. Release the **BOOT** button
6. Upload sketch
7. Press the **RST** button to exit download mode
