#include <Arduino.h>
#include <TM1652.h>
#include <TM16xxDisplay.h>
#include <EspMQTTClient.h>
#include <ezTime.h>

#if __has_include("myconfig.h")
# include "myconfig.h"
#else
#include "config.h"
#endif

TM1652 module(D4);
TM16xxDisplay disp(&module, 4);

// MQTT
#if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
EspMQTTClient client(SSID_NAME, SSID_PASSWORD, MQTT_BROKER, MQTT_CLIENT_ID, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); // with MQTT authentification
#else
EspMQTTClient client(SSID_NAME, SSID_PASSWORD, MQTT_BROKER, MQTT_CLIENT_ID, MQTT_PORT);
#endif

// Time
Timezone myTZ;
static uint8_t show_clock_event;
bool timeSynched = false;
static bool showClock = false;

// heartbeat
unsigned long last_clock_update = 0;
static bool heartbeat = false;

// Other
bool stateReceived = false; // true when the sate MQTT message is received
long last_state = 9999;        // last state received from MQTT
bool wifiinfishown = false; // true ig the WiFi IP address has been displayed after connection
byte last_intensity = 4;    // last intensity received from MQTT

void updateTimeOnDisplay()
{
    if (showClock)
    {
        unsigned long h = myTZ.hour();
        unsigned long m = myTZ.minute();
        unsigned long val = h * 100 + m;

        if (heartbeat)
        {
            disp.setDisplayToDecNumber(val, 0b00000100); // show the colon
        }
        else
        {
            disp.setDisplayToDecNumber(val, 0b00000000);
        }

        // Set the event to update the display in one second
        show_clock_event = myTZ.setEvent(updateTimeOnDisplay, myTZ.now() + 1);
    }
}

void onConnectionEstablished()
{
    // Subscribe to "7133egyptian/in/display1" and display received message to Serial
    client.subscribe(TOPIC_MODE, [](const String &payload)
                     {
        DEBUG_PRINTLN(payload);
        if (payload.equalsIgnoreCase("clock"))
        {
            showClock = true;
            updateTimeOnDisplay();
        }
        else if (payload.equalsIgnoreCase("state"))
        {
            deleteEvent(show_clock_event); // Delete the event to stop updating the display
            showClock = false;

            disp.clear();
            disp.setDisplayToDecNumber(last_state, 0, false);
        } });

    client.subscribe(TOPIC_STATE, [](const String &payload)
                     {
        // if waiting for an initial message and received one then clear the screen
        stateReceived = true;
        last_state = payload.toInt();
        if (!showClock)
        {
            disp.setDisplayToDecNumber(last_state, 0, false);
        } });

    client.subscribe(TOPIC_CLEAR, [](const String &payload)
                     {
        DEBUG_PRINTLN(payload);
        disp.clear(); });

    client.subscribe(TOPIC_INTENSITY, [](const String &payload)
                     {
        DEBUG_PRINTLN(payload);
        last_intensity = (byte)(payload.toInt() & 0x7);

        module.setupDisplay(true,last_intensity); });

    client.subscribe(TOPIC_REBOOT, [](const String &payload)
                     {
        DEBUG_PRINTLN(payload);
        client.publish(TOPIC_STATUS, "rebooting"); // You can activate the retain flag by setting the third parameter to true
        client.loop();
        myTZ.clearCache(); // clear the cache to avoid problems with the time after reboot
        module.setDisplayToString("boot"); // show boot while waiting to reboot
        delay(1000); // wait for the message to be sent
        DEBUG_PRINTLN("Rebooting...");
        ESP.restart(); });

    // Publish a message to "mytopic/test"
    client.publish(TOPIC_STATUS, "online"); // You can activate the retain flag by setting the third parameter to true

    module.clearDisplay();
}

void setup()
{
    DEBUG_BEGIN(115200);

    module.setupDisplay(true, 4);
    disp.clear();
    disp.setDisplayToString("IP ?"); // show con1 while waiting to connect to WiFi

    client.enableDebuggingMessages();                                                 // Enable debugging messages sent to serial output
    client.enableHTTPWebUpdater();                                                    // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
    client.enableLastWillMessage(TOPIC_STATUS, "offline", true); // You can activate the retain flag by setting the third parameter to true
    client.setWifiReconnectionAttemptDelay(15000);                                    // Set the delay between reconnection attempts to 1 second
    client.enableDrasticResetOnConnectionFailures();                                  // If the connection fails 10 times in a row, reset the ESP8266/ESP32
    client.enableMQTTPersistence();                                                   // Tell the broker to establish a persistent connection. Disabled by default. Must be called before the first loop() execution

#if defined(TIMEDEBUG_LEVEL)
    setDebug(TIMEDEBUG_LEVEL);
#endif

    setServer(APP_NTP_SERVER); // Set the NTP server to use
    setInterval(NTP_UPDATE_INTERVAL); // Set the NTP update interval to 1 hour (3600 seconds)

    if (!myTZ.setCache(0)) myTZ.setLocation(TIMEZONE);
}

void loop()
{
    client.loop();

    // myTZ event loop
    events(); // Call the events() function to check for any scheduled events

    if (!client.isWifiConnected())
    {
        delay(100);
    }
    else
    {
        disp.setIntensity(last_intensity); // set the display intensity

        if (millis() - last_clock_update > 1000)
        {
            heartbeat = !heartbeat;
            last_clock_update = millis();
        }

        if (!wifiinfishown)
        {
            wifiinfishown = true;
            DEBUG_PRINTLN("Connected to WiFi");
            disp.setDisplayToDecNumber(WiFi.localIP()[0], 0, false);
            delay(500);
            disp.setDisplayToDecNumber(WiFi.localIP()[1], 0, false);
            delay(500);
            disp.setDisplayToDecNumber(WiFi.localIP()[2], 0, false);
            delay(750);
            disp.setDisplayToDecNumber(WiFi.localIP()[3], 0, false);
            delay(750);
            disp.clear();
        }
        else if (!client.isMqttConnected())
        {
            disp.clear();
            disp.setDisplayToString("con2"); // show con2 while waiting to connect to MQTT server
        }
        else if (!timeSynched || (timeStatus() == timeNeedsSync))
        {
            disp.setDisplayToString("time"); // show con3 while waiting to connect to NTP server
            timeSynched = waitForSync(15);
            disp.clear();
        }
        else if (!stateReceived && !showClock)
        {
            if (heartbeat)
            {
                module.setSegments(0b10000000, 0);
                module.setSegments(0b00000000, 2);
            }
            else
            {
                module.setSegments(0b10000000, 2);
                module.setSegments(0b00000000, 0);
            }
        }
    }

    delay(1);
}
