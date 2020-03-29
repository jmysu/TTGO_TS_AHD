/*
 *
 *  TTGO TS MPU9250 AHD
 * 
 */
#include <Arduino.h>

///////////////////////
#define TFT_BL      27

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13

#define I2C_SDA     21
#define I2C_SCL     22

#define SPEAKER_PWD -1
#define SPEAKER_OUT 25
#define ADC_IN      35

#define BUTTON_1    36
#define BUTTON_2    37
#define BUTTON_3    39

#define BUTTONS_MAP {BUTTON_1,BUTTON_2,BUTTON_3}

#define CHANNEL_0   0
#define BOARD_VRESION   "<T10 V2.0>"
/////////////////////////////////////
// The first time you need to define the board model and version

// #define T4_V12
// #define T4_V13
// #define T10_V14
// #define T10_V18
#define T10_V20


#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include <Ticker.h>
#include <Button2.h>
#include <SD.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

SPIClass sdSPI(VSPI);
#define IP5306_ADDR         0X75
#define IP5306_REG_SYS_CTL0 0x00

uint8_t state = 0;
Button2 *pBtns = nullptr;
uint8_t g_btns[] =  BUTTONS_MAP;
char buff[512];
Ticker btnscanT;

bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}

void button_handle(uint8_t gpio)
{
    switch (gpio) {
#ifdef BUTTON_1
    case BUTTON_1: {
        state = 1;
    }
    break;
#endif

#ifdef BUTTON_2
    case BUTTON_2: {
        state = 2;
    }
    break;
#endif

#ifdef BUTTON_3
    case BUTTON_3: {
        state = 3;
    }
    break;
#endif

#ifdef BUTTON_4
    case BUTTON_4: {
        state = 4;
    }
    break;
#endif
    default:
        break;
    }
}

void button_callback(Button2 &b)
{
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        if (pBtns[i] == b) {
            Serial.printf("btn: %u press\n", pBtns[i].getAttachPin());
            button_handle(pBtns[i].getAttachPin());
        }
    }
}

void button_init()
{
    uint8_t args = sizeof(g_btns) / sizeof(g_btns[0]);
    pBtns = new Button2 [args];
    for (int i = 0; i < args; ++i) {
        pBtns[i] = Button2(g_btns[i]);
        pBtns[i].setPressedHandler(button_callback);
    }
#if defined(T10_V18) || defined(T4_V13) ||defined(T10_V20) || defined(T10_V14)
#if defined(T10_V18) || defined(T4_V13)|| defined(T10_V14)
    pBtns[0].setLongClickHandler([](Button2 & b) {
#elif defined(T10_V20)
    pBtns[1].setLongClickHandler([](Button2 & b) {
#endif

        int x = tft.width() / 2 ;
        int y = tft.height() / 2 - 30;
        int r = digitalRead(TFT_BL);
        tft.setTextSize(1);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.fillScreen(TFT_BLACK);
#if defined(T10_V14)
        tft.drawString(r ? "Backlight OFF" : "Backlight ON", x, y);
        tft.drawString("IP5306 KeepOn ", x - 20, y + 30);

        bool isOk = setPowerBoostKeepOn(1);
        tft.setTextColor(isOk ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.drawString( isOk ? "PASS" : "FAIL", x + 50, y + 30);
        y += 30;
#endif
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Press again to wake up", x - 20, y + 30);
//#ifdef ENABLE_MPU9250
//        IMU.setSleepEnabled(true);
//#endif

#ifndef ST7735_SLPIN
#define ST7735_SLPIN   0x10
#define ST7735_DISPOFF 0x28
#endif

        delay(3000);
        tft.writecommand(ST7735_SLPIN);
        tft.writecommand(ST7735_DISPOFF);
        digitalWrite(TFT_BL, !r);
        delay(1000);
        // esp_sleep_enable_ext0_wakeup((gpio_num_t )BUTTON_1, LOW);
        esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
        esp_deep_sleep_start();
    });
#endif
}

void button_loop() {
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        pBtns[i].loop();
    }
}

void spisd_test() {
    tft.fillScreen(TFT_BLACK);
    if (SD_CS >  0) {
        tft.setTextDatum(MC_DATUM);
        sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (!SD.begin(SD_CS, sdSPI)) {
            tft.setTextFont(2);
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString("SDCard MOUNT FAIL", tft.width() / 2, tft.height() / 2);
        } else {
            uint32_t cardSize = SD.cardSize() / (1024 * 1024);
            String str = "SDCard Size: " + String(cardSize) + "MB";
            tft.setTextFont(2);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.drawString(str, tft.width() / 2, tft.height() / 2);
        }
        delay(500);
    }
}


void playSound(void) {
    if (SPEAKER_OUT > 0) {
        if (SPEAKER_PWD > 0) {
            digitalWrite(SPEAKER_PWD, HIGH);
            delay(200);
        }
        ledcWriteTone(CHANNEL_0, 1000);
        delay(200);
        ledcWriteTone(CHANNEL_0, 0);
        if (SPEAKER_PWD > 0) {
            delay(200);
            digitalWrite(SPEAKER_PWD, LOW);
        }
    }
}

void buzzer_test() {
    if (SPEAKER_OUT > 0) {
        if (SPEAKER_PWD > 0) {
            pinMode(SPEAKER_PWD, OUTPUT);
        }
        ledcSetup(CHANNEL_0, 1000, 8);
        ledcAttachPin(SPEAKER_OUT, CHANNEL_0);
    }
}



void wifi_scan() {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    tft.drawString("Scan Network", tft.width() / 2, tft.height() / 2);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int16_t n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
    if (n == 0) {
        tft.drawString("no networks found", tft.width() / 2, tft.height() / 2);
    } else {
        tft.setTextDatum(TL_DATUM);
        tft.setCursor(0, 0);
        Serial.printf("Fount %d net\n", n);
        for (int i = 0; i < n; ++i) {
            sprintf(buff,
                    "[%d]:%s(%d)",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i));
            tft.println(buff);
        }
    }
}

void listDir(fs::FS & fs, const char *dirname, uint8_t levels) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(0, 0);

    tft.println("Listing directory:" + String(dirname));

    File root = fs.open(dirname);
    if (!root) {
        tft.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        tft.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            tft.print("  DIR : ");
            tft.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            tft.print("  FILE: ");
            tft.print(file.name());
            tft.print("  SIZE: ");
            tft.println(file.size());
        }
        file = root.openNextFile();
    }
}

extern void setupAHD();
extern void setupIMU();
void setup() {
    Serial.begin(115200);
    delay(1000);

    //Pin out Dump
    Serial.printf("Current select %s version\n", BOARD_VRESION);
    Serial.printf("TFT_MISO:%d\n", TFT_MISO);
    Serial.printf("TFT_MOSI:%d\n", TFT_MOSI);
    Serial.printf("TFT_SCLK:%d\n", TFT_SCLK);
    Serial.printf("TFT_CS:%d\n", TFT_CS);
    Serial.printf("TFT_DC:%d\n", TFT_DC);
    Serial.printf("TFT_RST:%d\n", TFT_RST);
    Serial.printf("TFT_BL:%d\n", TFT_BL);
    Serial.printf("SD_MISO:%d\n", SD_MISO);
    Serial.printf("SD_MOSI:%d\n", SD_MOSI);
    Serial.printf("SD_SCLK:%d\n", SD_SCLK);
    Serial.printf("SD_CS:%d\n", SD_CS);
    Serial.printf("I2C_SDA:%d\n", I2C_SDA);
    Serial.printf("I2C_SCL:%d\n", I2C_SCL);
    Serial.printf("SPEAKER_PWD:%d\n", SPEAKER_PWD);
    Serial.printf("SPEAKER_OUT:%d\n", SPEAKER_OUT);
    Serial.printf("ADC_IN:%d\n", ADC_IN);
    Serial.printf("BUTTON_1:%d\n", BUTTON_1);
    Serial.printf("BUTTON_2:%d\n", BUTTON_2);
    Serial.printf("BUTTON_3:%d\n", BUTTON_3);
#ifdef BUTTON_4
    Serial.printf("BUTTON_4:%d\n", BUTTON_4);
#endif

// // !
// #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

//     Wire.begin(I2C_SDA, I2C_SCL);
//     setupMPU9250();
//     readMPU9250();
//     delay(100);
//     IMU.setSleepEnabled(true);
//     delay(2000);
//     esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * TIME_TO_SLEEP);
//     esp_deep_sleep_start();
// // //!

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 0);

    if (TFT_BL > 0) {
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
    }

    spisd_test();
    buzzer_test();
    button_init();
    tft.setTextFont(1);
    tft.setTextSize(1);

    Wire.begin(I2C_SDA, I2C_SCL);

    setupIMU();
    setupAHD();
    state = 3;

    btnscanT.attach_ms(30, button_loop);
}
/*
float r, x1, ya, z1, x2, y2, z2, x3, y3, z3;               //
int f[8][2], x, y;                                         // Draw box, x, y center
int c[8][3] = {                                            // Cube
 {-20,-20, 20},{20,-20, 20},{20,20, 20},{-20,20, 20},
 {-20,-20,-20},{20,-20,-20},{20,20,-20},{-20,20,-20} };

void drawCube()
{
  x = tft.width() /2;                           // x Center Calculat
  y = tft.height()/2; y = y + 20;

 for (int a = 0; a <= 360; a = a + 2 ) {                   // 0 to 360 degree
   for (int i = 0; i < 8; i++) {                           //
    r  = a * 0.0174532;                                    // 1 degree
    x1 = c[i][2] * sin(r) + c[i][0] * cos(r);              // rotate Y
    ya = c[i][1];                                          //
    z1 = c[i][2] * cos(r) - c[i][0] * sin(r);              //
    x2 = x1;                                               //
    y2 = ya * cos(r) - z1 * sin(r);                        // rotate X
    z2 = ya * sin(r) + z1 * cos(r);                        //
    x3 = x2 * cos(r) - y2 * sin(r);                        // rotate Z
    y3 = x2 * sin(r) + y2 * cos(r);                        //
    z3 = z2; x3 = x3 + x ; y3 = y3 + y ;                   //
    f[i][0] = x3; f[i][1] = y3; f[i][2] = z3;              // store new values
    }
   tft.fillScreen(ST7735_BLACK);
   tft.drawLine(f[0][0],f[0][1],f[1][0],f[1][1],ST7735_WHITE);
   tft.drawLine(f[1][0],f[1][1],f[2][0],f[2][1],ST7735_WHITE);
   tft.drawLine(f[2][0],f[2][1],f[3][0],f[3][1],ST7735_WHITE);
   tft.drawLine(f[3][0],f[3][1],f[0][0],f[0][1],ST7735_WHITE);
   tft.drawLine(f[4][0],f[4][1],f[5][0],f[5][1],ST7735_WHITE);
   tft.drawLine(f[5][0],f[5][1],f[6][0],f[6][1],ST7735_WHITE);
   tft.drawLine(f[6][0],f[6][1],f[7][0],f[7][1],ST7735_WHITE);
   tft.drawLine(f[7][0],f[7][1],f[4][0],f[4][1],ST7735_WHITE);
   tft.drawLine(f[0][0],f[0][1],f[4][0],f[4][1],ST7735_WHITE);
   tft.drawLine(f[1][0],f[1][1],f[5][0],f[5][1],ST7735_WHITE);
   tft.drawLine(f[2][0],f[2][1],f[6][0],f[6][1],ST7735_WHITE);
   tft.drawLine(f[3][0],f[3][1],f[7][0],f[7][1],ST7735_WHITE);
   tft.drawLine(f[1][0],f[1][1],f[3][0],f[3][1],ST7735_WHITE);// cross
   tft.drawLine(f[0][0],f[0][1],f[2][0],f[2][1],ST7735_WHITE);// cross
 }
}
*/
extern float pitch,roll;
extern void loopAHD(int roll, int pitch);
extern void loopIMU();
void loop() {
    switch (state) {
    case 1:
        state = 0;
        wifi_scan();
        break;
    case 2:
        state = 0;
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
#ifdef T4_V12
        tft.drawString("Undefined function", tft.width() / 2, tft.height() / 2);
#else
        tft.drawString("Buzzer Test", tft.width() / 2, tft.height() / 2);
        playSound();
#endif
        break;
    case 3:
        //state = 0;
        //drawCube();

        loopIMU(); //update IMU data
        loopAHD(int(pitch),int(-roll)); //update AHD w/ TTGO TS 

        /*
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        
        readMPU9250();
        snprintf(buff, sizeof(buff), "--   ACC  GYR   MAG");
        tft.drawString(buff, 0, 0);
        snprintf(buff, sizeof(buff), "x% 7.1f% 5.1f% 8.1f", (int)1000 * IMU.ax, IMU.gx, IMU.mx);
        tft.drawString(buff, 0, 16);
        snprintf(buff, sizeof(buff), "y% 7.1f% 5.1f% 8.1f", (int)1000 * IMU.ay, IMU.gy, IMU.my);
        tft.drawString(buff, 0, 32);
        snprintf(buff, sizeof(buff), "z% 7.1f% 5.1f% 8.1f", (int)1000 * IMU.az, IMU.gz, IMU.mz);
        tft.drawString(buff, 0, 48);
        //delay(200);
*/
        //state = 0;
        //listDir(SD, "/", 2);
        break;
    case 4:
        state = 0;
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Undefined function", tft.width() / 2, tft.height() / 2);
        break;
    default:
        break;
    }
}