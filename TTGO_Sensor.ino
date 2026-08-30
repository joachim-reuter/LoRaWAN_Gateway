/*  Sender TTGO  FINAL 28.04.2026
Temp: Innen und Distance*/


#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

// -------- BMP280 --------
Adafruit_BMP280 bmp;

// -------- LoRa Keys --------
static const u1_t PROGMEM APPEUI[8] = {0};
static const u1_t PROGMEM DEVEUI[8] = { 
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
};
static const u1_t PROGMEM APPKEY[16] = {
  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
};

void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

// -------- Pins --------
const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 23,
  .dio = {26, 33, 32},
};

#define TRIG_PIN 4
#define ECHO_PIN 2

static osjob_t sendjob;
const unsigned TX_INTERVAL = 300;

// -------- Distanz --------
float measureDistance() {
  long duration;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;

  return (duration * 0.034 / 2.0) / 100.0;
}

// -------- Events --------
void onEvent (ev_t ev) {
  switch(ev) {
    case EV_JOINED:
      Serial.println("EV_JOINED");
      do_send(&sendjob);
      break;

    case EV_TXCOMPLETE:
      Serial.println("TX complete");
      break;
  }
}

// -------- Senden --------
void do_send(osjob_t* j){

  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println("LoRa busy...");
  } else {

    float distance = measureDistance();
    float temp = bmp.readTemperature();

    Serial.print("Distanz: ");
    Serial.println(distance);

    Serial.print("Temp: ");
    Serial.println(temp);

    if (distance > 0) {

      uint16_t dist_cm = distance * 100;
      int16_t temp_c = temp * 100;

      uint8_t payload[4];

      payload[0] = highByte(dist_cm);
      payload[1] = lowByte(dist_cm);

      payload[2] = highByte(temp_c);
      payload[3] = lowByte(temp_c);

      LMIC_setTxData2(1, payload, sizeof(payload), 0);

      Serial.println("Sende Distanz + Temp");
    }
  }

  os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
}

// -------- Setup --------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Wire.begin(21, 22);

  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 nicht gefunden!");
  }

  SPI.begin(5, 19, 27, 18);

  os_init();
  LMIC_reset();

  LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
  LMIC_setLinkCheckMode(0);
  LMIC_setDrTxpow(DR_SF7, 14);

  LMIC.dn2Dr = DR_SF9;
  LMIC.rxDelay = 1;

  LMIC_startJoining();
}

void loop() {
  os_runloop_once();
}