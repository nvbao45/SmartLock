#include "esp_camera.h"
// #include "soc/soc.h"          // Disable brownour problems
// #include "soc/rtc_cntl_reg.h" // Disable brownour problems
// #include "driver/rtc_io.h"
// #include "esp_system.h"
#include "define.h"
#include <base64.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

int pictureNumber = 0;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
MFRC522 mfrc522(HSPI_SS, RST_PIN);

BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristicRFID;
BLECharacteristic *pCharacteristicCamera;

bool deviceConnected = false;
const int LED = 4;
const int readPin = 33;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0)
        {
            //Serial.println("*********");
            //Serial.print("Received Value: ");

            for (int i = 0; i < rxValue.length(); i++)
            {
                //Serial.print(rxValue[i]);
            }

            //Serial.println();

            // Do stuff based on the command received from the app
            if (rxValue.find("A") != -1)
            {
                //Serial.print("Turning ON!");
                digitalWrite(LED, HIGH);
            }
            else if (rxValue.find("B") != -1)
            {
                //Serial.print("Turning OFF!");
                digitalWrite(LED, LOW);
            }

            //Serial.println();
            //Serial.println("*********");
        }
    }
};

void BLEinit()
{
    BLEDevice::init("ESP32 UART Test"); // Give it a name

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX_FINGERPRINT,
        BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristic->addDescriptor(new BLE2902());

    pCharacteristicRFID = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX_RFID,
        BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicRFID->addDescriptor(new BLE2902());

    pCharacteristicCamera = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristicCamera->addDescriptor(new BLE2902());

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX_CAMERA,
        BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
}

void cameraInit()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        //Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    cameraSetting();
}

void cameraSetting()
{
    sensor_t *s = esp_camera_sensor_get();

    s->set_brightness(s, 0);                 // -2 to 2
    s->set_contrast(s, 0);                   // -2 to 2
    s->set_saturation(s, 0);                 // -2 to 2
    s->set_special_effect(s, 0);             // 0 to 6 (0 – No Effect, 1 – Negative, 2 – Grayscale, 3 – Red Tint, 4 – Green Tint, 5 – Blue Tint, 6 – Sepia)
    s->set_whitebal(s, 1);                   // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);                   // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);                    // 0 to 4 – if awb_gain enabled (0 – Auto, 1 – Sunny, 2 – Cloudy, 3 – Office, 4 – Home)
    s->set_exposure_ctrl(s, 1);              // 0 = disable , 1 = enable
    s->set_aec2(s, 0);                       // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);                   // -2 to 2
    s->set_aec_value(s, 300);                // 0 to 1200
    s->set_gain_ctrl(s, 1);                  // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);                   // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
    s->set_bpc(s, 0);                        // 0 = disable , 1 = enable
    s->set_wpc(s, 1);                        // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);                    // 0 = disable , 1 = enable
    s->set_lenc(s, 1);                       // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);                    // 0 = disable , 1 = enable
    s->set_vflip(s, 0);                      // 0 = disable , 1 = enable
    s->set_dcw(s, 1);                        // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);                   // 0 = disable , 1 = enable
}

void flash()
{
    digitalWrite(FLASH, 1);
    delay(100);
    digitalWrite(FLASH, 0);
    delay(100);
    digitalWrite(FLASH, 1);
    delay(100);
    digitalWrite(FLASH, 0);
    delay(100);
}

void fingerInit()
{
    finger.begin(57600);
    delay(5);
    if (finger.verifyPassword())
    {
        //Serial.println("Found fingerprint sensor!");
    }
    else
    {
        //Serial.println("Did not find fingerprint sensor :(");
    }

    //Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    //Serial.print(F("Status: 0x"));
    //Serial.println(finger.status_reg, HEX);
    //Serial.print(F("Sys ID: 0x"));
    //Serial.println(finger.system_id, HEX);
    //Serial.print(F("Capacity: "));
    //Serial.println(finger.capacity);
    //Serial.print(F("Security level: "));
    //Serial.println(finger.security_level);
    //Serial.print(F("Device address: "));
    //Serial.println(finger.device_addr, HEX);
    //Serial.print(F("Packet len: "));
    //Serial.println(finger.packet_len);
    //Serial.print(F("Baud rate: "));
    //Serial.println(finger.baud_rate);

    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        //Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else
    {
        //Serial.println("Waiting for valid finger...");
        //Serial.print("Sensor contains ");
        //Serial.print(finger.templateCount);
        //Serial.println(" templates");
    }
}

void setup()
{
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    
    BLEinit();
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(FLASH, OUTPUT);
    //Serial.begin(115200);
    //Serial.println();
    cameraInit();
    fingerInit();
    SPI.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI);
    mfrc522.PCD_Init();
    delay(4);
}

void loop()
{
    String rfid = getRFID();
    int fingerId = getFingerprintIDez();

    pCharacteristicRFID->setValue("TEST");
    pCharacteristicRFID->notify();

    if (rfid != "")
    {
        //Serial.print("RFID: ");
        //Serial.println(rfid);
        pCharacteristicRFID->setValue(rfid.c_str());
        pCharacteristicRFID->notify();
    }

    if (fingerId != -1){
        pCharacteristic->setValue(fingerId);
        pCharacteristic->notify();
    }

    if (digitalRead(BUTTON) == 0)
    {
        flash();
        camera_fb_t *fb = NULL;
        fb = esp_camera_fb_get();
        if (!fb)
        {
            //Serial.println("Camera capture failed");
        }
        else
        {
            String imgDataB64 = base64::encode(fb->buf, fb->len);
            //Serial.println(imgDataB64);
            pCharacteristicCamera->setValue(imgDataB64.c_str());
            pCharacteristicCamera->notify();
        }
    }

    
    delay(50);
}

void sendBluetooth(String str){
    if (deviceConnected)
    {
        char txString[8];                 // make sure this is big enuffz
        pCharacteristic->setValue(str.c_str());
        pCharacteristic->notify(); // Send the value to the app!
    }
}

String getRFID()
{
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return "";
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return "";
    }

    // Dump debug info about the card; PICC_HaltA() is automatically called
    String uid = "";
    uid += String(mfrc522.uid.uidByte[0]);
    uid += ":";
    uid += String(mfrc522.uid.uidByte[1]);
    uid += ":";
    uid += String(mfrc522.uid.uidByte[2]);
    uid += ":";
    uid += String(mfrc522.uid.uidByte[3]);
    return uid;
}

uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        //Serial.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
        //Serial.println("No finger detected");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        //Serial.println("Imaging error");
        return p;
    default:
        //Serial.println("Unknown error");
        return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        //Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        //Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        //Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        //Serial.println("Could not find fingerprint features");
        return p;
    default:
        //Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK)
    {
        //Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        //Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        //Serial.println("Did not find a match");
        return p;
    }
    else
    {
        //Serial.println("Unknown error");
        return p;
    }

    // found a match!
    //Serial.print("Found ID #");
    //Serial.print(finger.fingerID);
    //Serial.print(" with confidence of ");
    //Serial.println(finger.confidence);

    return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)
        return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)
        return -1;

    // found a match!
    //Serial.print("Found ID #");
    //Serial.print(finger.fingerID);
    //Serial.print(" with confidence of ");
    //Serial.println(finger.confidence);
    return finger.fingerID;
}
