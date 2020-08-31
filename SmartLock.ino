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

BLECharacteristic *pCharacteristicFingerprint;
BLECharacteristic *pCharacteristicRFID;
BLECharacteristic *pCharacteristicCamera;

bool deviceConnected = false;

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
        }
    }
};

void BLEinit()
{
    BLEDevice::init("ESP32"); // Give it a name
    BLEDevice::setMTU(1024);
    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristicFingerprint = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX_FINGERPRINT,
        BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_READ);
    pCharacteristicFingerprint->addDescriptor(new BLE2902());

    pCharacteristicRFID = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX_RFID,
        BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_READ);
    pCharacteristicRFID->addDescriptor(new BLE2902());

    pCharacteristicCamera = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX_CAMERA,
        BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_READ);
    pCharacteristicCamera->addDescriptor(new BLE2902());

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
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

    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial2.printf("Camera init failed with error 0x%x", err);
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

void flash(int n = 2)
{
    for (int i = 0; i < n; i++)
    {
        digitalWrite(FLASH, 1);
        delay(100);
        digitalWrite(FLASH, 0);
        delay(100);
    }
}

void fingerInit()
{
    finger.begin(57600);
    delay(5);

    if (finger.verifyPassword())
    {
        Serial2.println("Found fingerprint sensor!");
    }
    else
    {
        Serial2.println("Did not find fingerprint sensor :(");
    }

    Serial2.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial2.print(F("Status: 0x"));
    Serial2.println(finger.status_reg, HEX);
    Serial2.print(F("Sys ID: 0x"));
    Serial2.println(finger.system_id, HEX);
    Serial2.print(F("Capacity: "));
    Serial2.println(finger.capacity);
    Serial2.print(F("Security level: "));
    Serial2.println(finger.security_level);
    Serial2.print(F("Device address: "));
    Serial2.println(finger.device_addr, HEX);
    Serial2.print(F("Packet len: "));
    Serial2.println(finger.packet_len);
    Serial2.print(F("Baud rate: "));
    Serial2.println(finger.baud_rate);

    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        Serial2.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else
    {
        Serial2.println("Waiting for valid finger...");
        Serial2.print("Sensor contains ");
        Serial2.print(finger.templateCount);
        Serial2.println(" templates");
    }
}

void setup()
{
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

    BLEinit();
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(FLASH, OUTPUT);
    Serial2.begin(115200, SERIAL_8N1, -1, 33);
    Serial2.println();
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

    if (rfid != "")
    {
        rfid = String("{RFID: \"" + rfid + "\"}");
        Serial2.print("RFID: ");
        Serial2.println(rfid);
        pCharacteristicRFID->setValue(rfid.c_str());
        pCharacteristicRFID->notify();
    }

    if (fingerId != -1)
    {
        String fingerIdStr = String(fingerId);
        fingerIdStr = String("{FINGER: \"" + fingerIdStr + "\"}");
        Serial2.print("FINGER: ");
        Serial2.println(fingerIdStr);
        pCharacteristicFingerprint->setValue(fingerIdStr.c_str());
        pCharacteristicFingerprint->notify();
    }

    if (digitalRead(BUTTON) == 0)
    {
        String imgBase64 = capture();
        int i = 0;
        int id = random(1000, 10000);
        // imgBase64 = String("{CAMERA: \"" + imgBase64 + "\"}");
        Serial2.println(imgBase64);
        
        while (imgBase64.length() > 0)
        {
            String img32 = imgBase64.substring(0, 400);
            imgBase64.remove(0, 400);
            
            img32 = String("{CAMERA: \"" + img32 + "\", SEQ: " + i++ + ", ID: " + id + "}");
            Serial2.println(img32);
            pCharacteristicCamera->setValue(img32.c_str());
            pCharacteristicCamera->notify();
        }
    }
}

String capture()
{
    flash();
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial2.println("Camera capture failed");
        return "";
    }
    else
    {
        String imgDataB64 = base64::encode(fb->buf, fb->len);
        return imgDataB64;
    }
}

String getRFID()
{
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return "";
    }

    if (!mfrc522.PICC_ReadCardSerial())
    {
        return "";
    }

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
        Serial2.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
        Serial2.println("No finger detected");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial2.println("Communication error");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial2.println("Imaging error");
        return p;
    default:
        Serial2.println("Unknown error");
        return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial2.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial2.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial2.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial2.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial2.println("Could not find fingerprint features");
        return p;
    default:
        Serial2.println("Unknown error");
        return p;
    }

    // OK converted!
    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK)
    {
        Serial2.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial2.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial2.println("Did not find a match");
        return p;
    }
    else
    {
        Serial2.println("Unknown error");
        return p;
    }

    // found a match!
    Serial2.print("Found ID #");
    Serial2.print(finger.fingerID);
    Serial2.print(" with confidence of ");
    Serial2.println(finger.confidence);

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
    Serial2.print("Found ID #");
    Serial2.print(finger.fingerID);
    Serial2.print(" with confidence of ");
    Serial2.println(finger.confidence);
    return finger.fingerID;
}

uint8_t getFingerprintEnroll(uint8_t id)
{

    int p = -1;
    Serial2.print("Waiting for valid finger to enroll as #");
    Serial2.println(id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial2.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial2.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial2.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial2.println("Imaging error");
            break;
        default:
            Serial2.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial2.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial2.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial2.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial2.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial2.println("Could not find fingerprint features");
        return p;
    default:
        Serial2.println("Unknown error");
        return p;
    }

    Serial2.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial2.print("ID ");
    Serial2.println(id);
    p = -1;
    Serial2.println("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial2.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial2.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial2.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial2.println("Imaging error");
            break;
        default:
            Serial2.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial2.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial2.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial2.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial2.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial2.println("Could not find fingerprint features");
        return p;
    default:
        Serial2.println("Unknown error");
        return p;
    }

    // OK converted!
    Serial2.print("Creating model for #");
    Serial2.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial2.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial2.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial2.println("Fingerprints did not match");
        return p;
    }
    else
    {
        Serial2.println("Unknown error");
        return p;
    }

    Serial2.print("ID ");
    Serial2.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        Serial2.println("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial2.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial2.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial2.println("Error writing to flash");
        return p;
    }
    else
    {
        Serial2.println("Unknown error");
        return p;
    }

    return true;
}
