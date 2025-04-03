// MQTT Topics
// Topic to which the status is published
#define TOPIC_STATUS "<your state topic path>"

// Control topics
// Don't forget the trailing slash
#define TOPIC_PREFIX "<your control topic root path>"

#define TOPIC_INTENSITY TOPIC_PREFIX "intensity"
#define TOPIC_CLEAR TOPIC_PREFIX "clear"
#define TOPIC_STATE TOPIC_PREFIX "state"
#define TOPIC_MODE TOPIC_PREFIX "mode"
#define TOPIC_REBOOT TOPIC_PREFIX "reboot"

// WiFi/MQTT Network settings
#define SSID_NAME "<ssid name>"
#define SSID_PASSWORD "<ssid password>"
#define MQTT_BROKER "<MQTT broker name or IP address>"
#define MQTT_CLIENT_ID "<MQTT client ID>"
#define MQTT_PORT <MQTT broker port> // integer (not a string)

// If MQTT Authorization is required, uncomment the following lines and set the values
//#define MQTT_USERNAME "mqtt"
//#define MQTT_PASSWORD "mqtt"

// Time stuff
#define TIMEZONE "<Your timezone>" // e.g. "America/Denver" for UTC-7
#define TIMEDEBUG_LEVEL NONE // NONE,ERROR,INFO,DEBUG
#define APP_NTP_SERVER "<NTP server>" // e.g. "us.pool.ntp.org"
#define NTP_UPDATE_INTERVAL 3600 // NTP update interval in seconds (default is 3600 seconds)

// Debugging stuff
#define OPT_DEBUG_PRINT

#if defined(OPT_DEBUG_PRINT)
#define DEBUG_BEGIN(x) Serial.begin(x)
#define DEBUG_PRINT(x) (Serial.print(x))
#define DEBUG_PRINTLN(x) (Serial.println(x))
#define DEBUG_PRINTLNHEX(x) (Serial.println(x, HEX))
#else
#define DEBUG_BEGIN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLNHEX(x)
#endif

