# ESP32 BLE Music Control library

This library allows you to make the ESP32 act as a Bluetooth Music Controller. This devie allows to control and monitors the music playback of a blueooth capable music source. 
This code in based in the repository:
- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard)
You might also be interested in:
- [ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Mouse)
- [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad)


## Features

 - [x] Media keys are supported
 - [x] Set battery level (basically works, but doesn't show up in Android's status bar)
 - [?] Compatible with Android
 - [?] Compatible with Windows
 - [?] Compatible with Linux
 - [ ] Compatible with MacOs
 - [ ] Read play/stop state
 - [x] Compatible with iOS (nYou may need to use a third party software to find the device the first time)

The supported media keys are following:
- KEY_MEDIA_NEXT_TRACK
- KEY_MEDIA_PREVIOUS_TRACK
- KEY_MEDIA_STOP
- KEY_MEDIA_PLAY_PAUSE
- KEY_MEDIA_MUTE
- KEY_MEDIA_VOLUME_UP
- KEY_MEDIA_VOLUME_DOWN
- KEY_MEDIA_PLAY_SKIP

There is also Bluetooth specific information that you can set (optional):
Instead of `BleKeyboard bleKeyboard;` you can do `BleKeyboard bleKeyboard("Bluetooth Device Name", "Bluetooth Device Manufacturer", 100);`.
The third parameter is the initial battery level of your device. To adjust the battery level later on you can simply call e.g.  `bleKeyboard.setBatteryLevel(50)` (set battery level to 50%).
By default the battery level will be set to 100%, the device name will be `ESP32 Bluetooth Keyboard` and the manufacturer will be `Espressif`.


