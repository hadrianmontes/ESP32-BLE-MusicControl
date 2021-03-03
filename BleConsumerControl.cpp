#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"
#include "Arduino.h"

#include "BleConnectionStatus.h"
#include "BleConsumerControl.h"

#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif


// Report IDs:
#define MEDIA_KEYS_ID 0x01

static const uint8_t _hidReportDescriptor[] = {
    USAGE_PAGE(1),       0x01, // USAGE_PAGE (Generic Desktop)
    USAGE(1),           0x07,          // USAGE (Keypad)
    COLLECTION(1),       0x01, // COLLECTION (Application)
        // USAGE(1),           0x07,          // USAGE (Keypad)
        // COLLECTION(1),      0x00,
        USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
		USAGE(1),           0x01,          // USAGE (Consumer Control)
		COLLECTION(1),      0x00,
            LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
            LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
            REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
            REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (16)
            USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
            USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
            USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
            USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
            USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
            USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
            USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
            USAGE(1),           0xCE,          //   USAGE(play/skip)
            HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        END_COLLECTION(0),
    END_COLLECTION(0)                  // END_COLLECTION
};



BleConsumerControl::BleConsumerControl(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) : hid(0)
{
  this->deviceName = deviceName;
  this->deviceManufacturer = deviceManufacturer;
  this->batteryLevel = batteryLevel;
  this->connectionStatus = new BleConnectionStatus();
}

void BleConsumerControl::begin(void)
{
  Serial.println("Beggining");
  xTaskCreate(this->taskServer, "server", 20000, (void *)this, 5, NULL);
}

void BleConsumerControl::end(void)
{
}

bool BleConsumerControl::isConnected(void) {
  Serial.println("is connected");
  return this->connectionStatus->connected;
}

void BleConsumerControl::setBatteryLevel(uint8_t level) {
  Serial.println("Battery level");

  this->batteryLevel = level;
  if (hid != 0)
    this->hid->setBatteryLevel(this->batteryLevel);
}

void BleConsumerControl::taskServer(void* pvParameter) {
  BleConsumerControl* bleKeyboardInstance = (BleConsumerControl *) pvParameter; //static_cast<BleKeyboard *>(pvParameter);
  BLEDevice::init(bleKeyboardInstance->deviceName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(bleKeyboardInstance->connectionStatus);

  bleKeyboardInstance->hid = new BLEHIDDevice(pServer);
  bleKeyboardInstance->inputMediaKeys = bleKeyboardInstance->hid->inputReport(0);
  bleKeyboardInstance->connectionStatus->inputMediaKeys = bleKeyboardInstance->inputMediaKeys;

  bleKeyboardInstance->hid->manufacturer()->setValue(bleKeyboardInstance->deviceManufacturer);

  bleKeyboardInstance->hid->pnp(0x01,0x02e5,0xabbb,0x0110);
  bleKeyboardInstance->hid->hidInfo(0x00,0x01);

  BLESecurity *pSecurity = new BLESecurity();

  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  bleKeyboardInstance->hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  bleKeyboardInstance->hid->startServices();

  bleKeyboardInstance->onStarted(pServer);

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(bleKeyboardInstance->hid->hidService()->getUUID());
  pAdvertising->setScanResponse(false);
  pAdvertising->start();
  bleKeyboardInstance->hid->setBatteryLevel(bleKeyboardInstance->batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  vTaskDelay(portMAX_DELAY); //delay(portMAX_DELAY);
}


void BleConsumerControl::sendReport(MediaKeyReport* keys)
{
  Serial.println("Sending Report");
  if (this->isConnected())
  {
    this->inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
    this->inputMediaKeys->notify();
  }
}

extern
const uint8_t _asciimap[128] PROGMEM;

#define SHIFT 0x80
const uint8_t _asciimap[128] =
{
	0x00,             // NUL
	0x00,             // SOH
	0x00,             // STX
	0x00,             // ETX
	0x00,             // EOT
	0x00,             // ENQ
	0x00,             // ACK
	0x00,             // BEL
	0x2a,			// BS	Backspace
	0x2b,			// TAB	Tab
	0x28,			// LF	Enter
	0x00,             // VT
	0x00,             // FF
	0x00,             // CR
	0x00,             // SO
	0x00,             // SI
	0x00,             // DEL
	0x00,             // DC1
	0x00,             // DC2
	0x00,             // DC3
	0x00,             // DC4
	0x00,             // NAK
	0x00,             // SYN
	0x00,             // ETB
	0x00,             // CAN
	0x00,             // EM
	0x00,             // SUB
	0x00,             // ESC
	0x00,             // FS
	0x00,             // GS
	0x00,             // RS
	0x00,             // US

	0x2c,		   //  ' '
	0x1e|SHIFT,	   // !
	0x34|SHIFT,	   // "
	0x20|SHIFT,    // #
	0x21|SHIFT,    // $
	0x22|SHIFT,    // %
	0x24|SHIFT,    // &
	0x34,          // '
	0x26|SHIFT,    // (
	0x27|SHIFT,    // )
	0x25|SHIFT,    // *
	0x2e|SHIFT,    // +
	0x36,          // ,
	0x2d,          // -
	0x37,          // .
	0x38,          // /
	0x27,          // 0
	0x1e,          // 1
	0x1f,          // 2
	0x20,          // 3
	0x21,          // 4
	0x22,          // 5
	0x23,          // 6
	0x24,          // 7
	0x25,          // 8
	0x26,          // 9
	0x33|SHIFT,      // :
	0x33,          // ;
	0x36|SHIFT,      // <
	0x2e,          // =
	0x37|SHIFT,      // >
	0x38|SHIFT,      // ?
	0x1f|SHIFT,      // @
	0x04|SHIFT,      // A
	0x05|SHIFT,      // B
	0x06|SHIFT,      // C
	0x07|SHIFT,      // D
	0x08|SHIFT,      // E
	0x09|SHIFT,      // F
	0x0a|SHIFT,      // G
	0x0b|SHIFT,      // H
	0x0c|SHIFT,      // I
	0x0d|SHIFT,      // J
	0x0e|SHIFT,      // K
	0x0f|SHIFT,      // L
	0x10|SHIFT,      // M
	0x11|SHIFT,      // N
	0x12|SHIFT,      // O
	0x13|SHIFT,      // P
	0x14|SHIFT,      // Q
	0x15|SHIFT,      // R
	0x16|SHIFT,      // S
	0x17|SHIFT,      // T
	0x18|SHIFT,      // U
	0x19|SHIFT,      // V
	0x1a|SHIFT,      // W
	0x1b|SHIFT,      // X
	0x1c|SHIFT,      // Y
	0x1d|SHIFT,      // Z
	0x2f,          // [
	0x31,          // bslash
	0x30,          // ]
	0x23|SHIFT,    // ^
	0x2d|SHIFT,    // _
	0x35,          // `
	0x04,          // a
	0x05,          // b
	0x06,          // c
	0x07,          // d
	0x08,          // e
	0x09,          // f
	0x0a,          // g
	0x0b,          // h
	0x0c,          // i
	0x0d,          // j
	0x0e,          // k
	0x0f,          // l
	0x10,          // m
	0x11,          // n
	0x12,          // o
	0x13,          // p
	0x14,          // q
	0x15,          // r
	0x16,          // s
	0x17,          // t
	0x18,          // u
	0x19,          // v
	0x1a,          // w
	0x1b,          // x
	0x1c,          // y
	0x1d,          // z
	0x2f|SHIFT,    // {
	0x31|SHIFT,    // |
	0x30|SHIFT,    // }
	0x35|SHIFT,    // ~
	0				// DEL
};


uint8_t USBPutChar(uint8_t c);

// press() adds the specified key (printing, non-printing, or modifier)
// to the persistent key report and sends the report.  Because of the way
// USB HID works, the host acts like the key remains pressed until we
// call release(), releaseAll(), or otherwise clear the report and resend.

size_t BleConsumerControl::press(const MediaKeyReport k)
{

  Serial.println("Pressing");
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);

    mediaKeyReport_16 |= k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

	sendReport(&_mediaKeyReport);
	return 1;
}

// release() takes the specified key out of the persistent key report and
// sends the report.  This tells the OS the key is no longer pressed and that
// it shouldn't be repeated any more.

size_t BleConsumerControl::release(const MediaKeyReport k)
{
  Serial.println("releasing");
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
    mediaKeyReport_16 &= ~k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

	sendReport(&_mediaKeyReport);
	return 1;
}

void BleConsumerControl::releaseAll(void)
{
  Serial.println("releasing all");
    _mediaKeyReport[0] = 0;
    _mediaKeyReport[1] = 0;
}


size_t BleConsumerControl::write(const MediaKeyReport c)
{
  Serial.println("writing");
	uint16_t p = press(c);  // Keydown
	release(c);            // Keyup
	return p;              // just return the result of press() since release() almost always returns 1
}
