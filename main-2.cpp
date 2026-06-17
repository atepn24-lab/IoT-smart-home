
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// ================= WIFI =================

const char* ssid = "iPhone";
const char* password = "123456789";

// ================= SERVER =================

String serverName =
"http://172.20.10.8:2000/data";

String commandServer =
"http://172.20.10.8:2000/light/state";

// ================= PINS =================

#define GAS_BUZZER_PIN 25
#define RELAY_PIN 2
#define PIR_PIN 5
#define PIR_LED_PIN 27
#define LDR_PIN 34
#define DHT_PIN 15
#define BUZZER_PIN 32
#define REED_PIN 21
#define TRIG_PIN 26
#define ECHO_PIN 14
#define GAS_PIN 33

// ================= RFID =================

#define SS_PIN     4
#define RST_PIN    13

#define RFID_SCK   18
#define RFID_MISO  19
#define RFID_MOSI  23

// ================= DHT =================

#define DHTTYPE DHT11

DHT dht(
    DHT_PIN,
    DHTTYPE
);

// ================= RFID =================

MFRC522 rfid(
    SS_PIN,
    RST_PIN
);

// ================= STATES =================

bool lightState = false;

bool manualMode = false;

bool gasDetected = false;

bool websiteLightState = false;

bool motionState = false;

bool securityArmed = true;

bool alarmTriggered = false;

float temperature = 0;

float humidity = 0;

float distance = 0;

int gasValue = 0;

// ================= SENSOR TASK =================

void sensorTask(
    void *pvParameters
) {

    while (true) {

        // ================= DHT =================

        float newTemp =
            dht.readTemperature();

        float newHum =
            dht.readHumidity();

        if (
            !isnan(newTemp) &&
            !isnan(newHum)
        ) {

            temperature = newTemp;

            humidity = newHum;
        }

        // ================= PIR =================

        motionState =
            digitalRead(PIR_PIN);

        digitalWrite(
            PIR_LED_PIN,
            motionState
        );

        // ================= SMART LIGHT =================

        if (manualMode) {

            digitalWrite(
                RELAY_PIN,
                websiteLightState
            );

            lightState =
                websiteLightState;
        }

        else {

            int ldrValue =
                analogRead(LDR_PIN);

            bool dark =
                ldrValue < 2000;

            digitalWrite(
                RELAY_PIN,
                dark
            );

            lightState =
                dark;
        }

        // ================= ULTRASONIC =================

        digitalWrite(
            TRIG_PIN,
            LOW
        );

        delayMicroseconds(2);

        digitalWrite(
            TRIG_PIN,
            HIGH
        );

        delayMicroseconds(10);

        digitalWrite(
            TRIG_PIN,
            LOW
        );

        long duration =
            pulseIn(
                ECHO_PIN,
                HIGH,
                30000
            );

        distance =
            duration * 0.034 / 2;

        // ================= GAS SENSOR =================

        gasValue =
            analogRead(GAS_PIN);

        if (gasValue > 1000) {

            gasDetected = true;

            digitalWrite(
                GAS_BUZZER_PIN,
                HIGH
            );
        }

        else {

            gasDetected = false;

            digitalWrite(
                GAS_BUZZER_PIN,
                LOW
            );
        }

        // ================= SECURITY =================

        bool intruderDetected =
            (
                distance > 0 &&
                distance < 20
            );

        bool doorOpen =
            digitalRead(REED_PIN)
            == LOW;

        if (
            securityArmed &&
            (
                intruderDetected ||
                doorOpen
            )
        ) {

            alarmTriggered = true;
        }

        // ================= RFID =================

        if (
            rfid.PICC_IsNewCardPresent() &&
            rfid.PICC_ReadCardSerial()
        ) {

            String cardUID = "";

            for (
                byte i = 0;
                i < rfid.uid.size;
                i++
            ) {

                if (
                    rfid.uid.uidByte[i] < 0x10
                ) {

                    cardUID += "0";
                }

                cardUID += String(
                    rfid.uid.uidByte[i],
                    HEX
                );

                if (
                    i !=
                    rfid.uid.size - 1
                ) {

                    cardUID += " ";
                }
            }

            cardUID.toLowerCase();

            Serial.print(
                "CARD UID: "
            );

            Serial.println(
                cardUID
            );

            // ================= CORRECT CARD =================

            if (
                cardUID ==
                "a2 b0 27 07"
            ) {

                Serial.println(
                    "ACCESS GRANTED"
                );

                alarmTriggered = false;

                securityArmed = false;

                digitalWrite(
                    BUZZER_PIN,
                    LOW
                );
            }

            // ================= WRONG CARD =================

            else {

                Serial.println(
                    "ACCESS DENIED"
                );

                alarmTriggered = true;
            }

            rfid.PICC_HaltA();

            rfid.PCD_StopCrypto1();

            delay(1000);
        }

        // ================= SECURITY BUZZER =================

        digitalWrite(
            BUZZER_PIN,
            alarmTriggered
        );

        // ================= SERIAL MONITOR =================

        Serial.println(
            "===================="
        );

        Serial.print(
            "Temperature: "
        );

        Serial.println(
            temperature
        );

        Serial.print(
            "Humidity: "
        );

        Serial.println(
            humidity
        );

        Serial.print(
            "Motion: "
        );

        Serial.println(
            motionState
        );

        Serial.print(
            "Distance: "
        );

        Serial.println(
            distance
        );

        Serial.print(
            "Gas Value: "
        );

        Serial.println(
            gasValue
        );

        Serial.print(
            "Alarm: "
        );

        Serial.println(
            alarmTriggered
        );

        Serial.println(
            "===================="
        );

        vTaskDelay(
            1000 /
            portTICK_PERIOD_MS
        );
    }
}

// ================= NETWORK TASK =================

void networkTask(
    void *pvParameters
) {

    while (true) {

        if (
            WiFi.status() ==
            WL_CONNECTED
        ) {

            // ================= SEND DATA =================

            HTTPClient http;

            http.begin(
                serverName
            );

            http.addHeader(
                "Content-Type",
                "application/json"
            );

            String jsonData = "{";

            jsonData +=
                "\"temperature\":" +
                String(temperature) + ",";

            jsonData +=
                "\"humidity\":" +
                String(humidity) + ",";

            jsonData +=
                "\"light\":" +
                String(
                    lightState
                    ? "true"
                    : "false"
                ) + ",";

            jsonData +=
                "\"motion\":" +
                String(
                    motionState
                    ? "true"
                    : "false"
                ) + ",";

            jsonData +=
                "\"gasDetected\":" +
                String(
                    gasDetected
                    ? "true"
                    : "false"
                ) + ",";

            jsonData +=
                "\"gasValue\":" +
                String(gasValue) + ",";

            jsonData +=
                "\"securityArmed\":" +
                String(
                    securityArmed
                    ? "true"
                    : "false"
                ) + ",";

            jsonData +=
                "\"alarm\":" +
                String(
                    alarmTriggered
                    ? "true"
                    : "false"
                ) + ",";

            jsonData +=
                "\"distance\":" +
                String(distance);

            jsonData += "}";

            int httpResponseCode =
                http.POST(jsonData);

            Serial.print(
                "POST RESPONSE: "
            );

            Serial.println(
                httpResponseCode
            );

            http.end();

            // ================= RECEIVE COMMANDS =================

            HTTPClient commandHttp;

            commandHttp.begin(
                commandServer
            );

            int responseCode =
                commandHttp.GET();

            if (responseCode > 0) {

                String payload =
                    commandHttp.getString();

                JsonDocument doc;

                deserializeJson(
                    doc,
                    payload
                );

                manualMode =
                    doc["manualMode"];

                websiteLightState =
                    doc["light"];

                securityArmed =
                    doc["securityArmed"];
            }

            commandHttp.end();
        }

        vTaskDelay(
            1000 /
            portTICK_PERIOD_MS
        );
    }
}

// ================= SETUP =================

void setup() {

    Serial.begin(115200);

    // ================= PIN MODES =================

    pinMode(
        GAS_BUZZER_PIN,
        OUTPUT
    );

    pinMode(
        GAS_PIN,
        INPUT
    );

    pinMode(
        RELAY_PIN,
        OUTPUT
    );

    pinMode(
        PIR_PIN,
        INPUT
    );

    pinMode(
        PIR_LED_PIN,
        OUTPUT
    );

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    pinMode(
        REED_PIN,
        INPUT_PULLUP
    );

    pinMode(
        TRIG_PIN,
        OUTPUT
    );

    pinMode(
        ECHO_PIN,
        INPUT
    );

    // ================= DEFAULT STATES =================

    digitalWrite(
        RELAY_PIN,
        LOW
    );

    digitalWrite(
        BUZZER_PIN,
        LOW
    );

    digitalWrite(
        GAS_BUZZER_PIN,
        LOW
    );

    // ================= DHT =================

    dht.begin();

    // ================= RFID =================

    SPI.begin(
        RFID_SCK,
        RFID_MISO,
        RFID_MOSI,
        SS_PIN
    );

    rfid.PCD_Init();

    Serial.println(
        "RFID READY"
    );

    // ================= WIFI =================

    WiFi.begin(
        ssid,
        password
    );

    Serial.print(
        "Connecting to WiFi"
    );

    while (
        WiFi.status() !=
        WL_CONNECTED
    ) {

        delay(500);

        Serial.print(".");
    }

    Serial.println();

    Serial.println(
        "WiFi Connected"
    );

    Serial.print(
        "ESP32 IP: "
    );

    Serial.println(
        WiFi.localIP()
    );

    // ================= RTOS TASKS =================

    xTaskCreatePinnedToCore(
        sensorTask,
        "SensorTask",
        4096,
        NULL,
        1,
        NULL,
        0
    );

    xTaskCreatePinnedToCore(
        networkTask,
        "NetworkTask",
        8192,
        NULL,
        1,
        NULL,
        1
    );
}

// ================= LOOP =================

void loop() {

}