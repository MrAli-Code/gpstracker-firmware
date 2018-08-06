#include "Debug.h"
#include "Flash.h"
#include "Positions.h"
#include "Core.h"
#include "Alerts.h"

#define LOGGER_NAME "Debug"

#define MENU_ENTRY(name, text) const char MENU_##name[] PROGMEM = text

const char FAKE_GPS_ENTRY[] PROGMEM = "1,1,20170924184842.000,49.454862,1.144537,71.900,2.70,172.6,1,,1.3,2.2,1.8,,11,7,,,37,,";

MENU_ENTRY(HEADER,					"========================\n-- Menu --");
MENU_ENTRY(SEPARATOR,				"----");

MENU_ENTRY(RUN,						"[R] Run");
MENU_ENTRY(RUN_ONCE,				"[r] Run once");
MENU_ENTRY(RAM,						"[f] Free RAM");
MENU_ENTRY(READ_BATTERY,			"[b] Read battery");
MENU_ENTRY(GPS_ON,					"[G] GPS On");
MENU_ENTRY(GPS_OFF,					"[g] GPS Off");
MENU_ENTRY(GPS_GET,					"[L] Get GPS position");
MENU_ENTRY(GPS_SET,					"[l] Set last GPS position");
MENU_ENTRY(RTC_SET,					"[T] Get RTC time");
MENU_ENTRY(RTC_GET,					"[t] Set RTC time");
MENU_ENTRY(EEPROM_GET_CONFIG,		"[C] Get EEPROM config");
MENU_ENTRY(EEPROM_RESET_CONFIG,		"[c] Reset EEPROM config");
MENU_ENTRY(EEPROM_GET_CONTENT,		"[E] Get EEPROM content");
MENU_ENTRY(EEPROM_GET_ENTRIES,		"[P] Get EEPROM entries");
MENU_ENTRY(EEPROM_GET_LAST_ENTRY,	"[p] Get EEPROM last entry");
MENU_ENTRY(EEPROM_ADD_ENTRY,		"[a] Add last entry to EEPROM");
MENU_ENTRY(EEPROM_BACKUP_ENTRIES,	"[B] Backup EEPROM entries");
MENU_ENTRY(NOTIFY_FAILURES,			"[F] Notify failures");
MENU_ENTRY(CLEAR_ALERTS,			"[A] Clear alerts");
MENU_ENTRY(SLEEP_DEEP,				"[s] Deep sleep for 10s");
MENU_ENTRY(QUESTION,				"?");

const PROGMEM uint8_t commandIdMapping[] = {
	'R', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::RUN),
	'r', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::ONCE),
	'f', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::RAM),
	'b', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::BATTERY),
	'G', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::GPS_ON),
	'g', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::GPS_OFF),
	'L', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::GPS_GET),
	'l', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::GPS_SET),
	'T', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::RTC_GET),
	't', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::RTC_SET),
	'C', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_GET_CONFIG),
	'c', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_RESET_CONFIG),
	'E', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_GET_CONTENT),
	'P', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_GET_ENTRIES),
	'p', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_GET_LAST_ENTRY),
	'a', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_ADD_ENTRY),
	'B', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::EEPROM_BACKUP_ENTRIES),
	'F', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::NOTIFY_FAILURES),
	'A', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::CLEAR_ALERTS),
	's', static_cast<uint8_t>(debug::GPSTRACKER_DEBUG_COMMAND::SLEEP_DEEP),
};

const char * const MENU_ENTRIES[] PROGMEM = {
	MENU_HEADER,
	MENU_RUN,
	MENU_RUN_ONCE,

	MENU_SEPARATOR,

	MENU_RAM,
	MENU_READ_BATTERY,

	MENU_SEPARATOR,

	MENU_GPS_ON,
	MENU_GPS_OFF,
	MENU_GPS_GET,
	MENU_GPS_SET,

	MENU_SEPARATOR,

	MENU_RTC_SET,
	MENU_RTC_GET,

	MENU_SEPARATOR,

	MENU_EEPROM_GET_CONFIG,
	MENU_EEPROM_RESET_CONFIG,
	MENU_EEPROM_GET_CONTENT,
	MENU_EEPROM_GET_ENTRIES,
	MENU_EEPROM_GET_LAST_ENTRY,
	MENU_EEPROM_ADD_ENTRY,
	MENU_EEPROM_BACKUP_ENTRIES,
	MENU_SEPARATOR,

	MENU_NOTIFY_FAILURES,
	MENU_CLEAR_ALERTS,
	MENU_SEPARATOR,

	MENU_SLEEP_DEEP,

	MENU_QUESTION
};

int freeRam2() { // dirty hack because putting it in namespace doesn't compile
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

using namespace utils;

namespace debug {

	namespace details {
		inline void displayPosition(PositionEntry entry) {
			Log.notice(F("%d%%, %dmV, %f�C, %ds, %d, %s\n"), entry.metadata.batteryLevel, entry.metadata.batteryVoltage, entry.metadata.temperature, entry.metadata.timeToFix, entry.metadata.status, entry.position);
		}
	}

	int freeRam() {
		return freeRam2();
	}

	void displayFreeRam() {
		Log.notice(F("RAM: %d\n"), freeRam());
	}

	GPSTRACKER_DEBUG_COMMAND parseCommand(char id) {
		size_t mappingArraySize = flash::getArraySize(commandIdMapping);
		char commandId;

		for (uint8_t i = 0; i < mappingArraySize; i += 2) {
			commandId = pgm_read_byte_near(commandIdMapping + i);
			if (commandId == id) return static_cast<GPSTRACKER_DEBUG_COMMAND>(pgm_read_byte_near(commandIdMapping + i + 1));
		}

		return GPSTRACKER_DEBUG_COMMAND::NONE;
	}

	GPSTRACKER_DEBUG_COMMAND menu(uint16_t timeout) {
		GPSTRACKER_DEBUG_COMMAND command;
		size_t menuSize = flash::getArraySize(MENU_ENTRIES);
		uint8_t intermediate_timeout = 50;

		do {
			for (uint8_t i = 0; i < menuSize; i++) {
				Serial.println(reinterpret_cast<const __FlashStringHelper *>(pgm_read_word_near(&MENU_ENTRIES[i])));
			}

			while (!Serial.available()) {
				if (timeout > 0) {
					delay(intermediate_timeout);
					timeout -= intermediate_timeout;
					if (timeout <= 0) {
						NOTICE_MSG("menu", "Timeout expired.");
						return GPSTRACKER_DEBUG_COMMAND::RUN;
					}
				}
			}
			command = parseCommand(Serial.read());
			while (Serial.available()) Serial.read(); //flushing input
		} while (command == GPSTRACKER_DEBUG_COMMAND::NONE);

		return command;
	}

	void getAndDisplayGpsPosition() {
		SIM808_GPS_STATUS gpsStatus = gps::acquireCurrentPosition(GPS_DEFAULT_TOTAL_TIMEOUT_MS);

		NOTICE_FORMAT("getAndDisplayGpsPosition", "%d %s", gpsStatus, gps::lastPosition);
	}

	void setFakeGpsPosition() {
		strlcpy_P(gps::lastPosition, FAKE_GPS_ENTRY, GPS_POSITION_SIZE);

		NOTICE_FORMAT("setFakeGpsPosition", "Last position set to : %s", gps::lastPosition);
		NOTICE_FORMAT("setFakeGpsPosition", "Speed : %d", gps::getVelocity());
		NOTICE_FORMAT("setFakeGpsPosition", "Sleep time : %d", core::mapSleepTime(gps::getVelocity()));
	}

	void getAndDisplayBattery() {
		hardware::sim808::powerOn();
		SIM808ChargingStatus status = hardware::sim808::device.getChargingState();
		hardware::sim808::powerOff();

		NOTICE_FORMAT("getAndDisplayBattery", "%d %d%% %dmV", status.state, status.level, status.voltage);
	}

	void getAndDisplayRtcTime() {
		tmElements_t time;
		rtc::getTime(time);

		NOTICE_FORMAT("getAndDisplayRtcTime", "%d/%d/%d %d:%d:%d %t", tmYearToCalendar(time.Year), time.Month, time.Day, time.Hour, time.Minute, time.Second, rtc::isAccurate());
	}

	void setRtcTime() {
		tmElements_t time;
		gps::getTime(time);
		rtc::setTime(time);
	}

	void getAndDisplaySleepTimes() {
		size_t arraySize = flash::getArraySize(config::defaultSleepTimings);
		sleepTimings_t maxSpeedTiming;
		utils::flash::read(&config::defaultSleepTimings[arraySize - 1], maxSpeedTiming);

		for (int i = 0; i <= maxSpeedTiming.speed; i++) {
			core::mapSleepTime(i);
		}

		NOTICE_MSG("getAndDisplaySleepTimes", "Done");
	}

	void getAndDisplayEepromConfig() {
		config::main::setup(); //forcing read again
	}

	void getAndDisplayEepromContent() {
		char buffer[128];
		hardware::i2c::powerOn();

		for (int i = 0; i < 8; i++) {
			hardware::i2c::eeprom.read(128 * i, buffer, 128);
			for (int i = 0; i < 128; i++) {
				Serial.print(buffer[i], HEX);
			}
		}
		Serial.println();
		hardware::i2c::powerOff();

		NOTICE_MSG("getAndDisplayEepromContent", "Done");
	}

	void getAndDisplayEepromPositions() {
		uint16_t currentEntryIndex = config::main::value.firstEntry;
		PositionEntry currentEntry;

		hardware::i2c::powerOn();
		do {
			if (!positions::get(currentEntryIndex, currentEntry)) break;
			details::displayPosition(currentEntry);
		} while (positions::moveNext(currentEntryIndex));
		hardware::i2c::powerOff();
	}

	void getAndDisplayEepromLastPosition() {
		uint16_t lastEntryIndex = config::main::value.lastEntry;
		PositionEntry lastEntry;

		if(positions::get(lastEntryIndex, lastEntry)) details::displayPosition(lastEntry);
		else Log.notice(F("No position recorded\n"));
	}

	void addLastPositionToEeprom() {
		hardware::sim808::powerOn();
		SIM808ChargingStatus status = hardware::sim808::device.getChargingState();
		hardware::sim808::powerOff();

		PositionEntryMetadata metadata = {
			status.level,
			status.voltage,
			rtc::getTemperature(),
			0,
			SIM808_GPS_STATUS::OFF
		};

		for(int i = 0; i < 10; i++) positions::appendLast(metadata);
	}

	void notifyFailures() {
		PositionEntryMetadata metadata = {
			1, //all battery alerts should goes on with this
			3800, //doesn't matter
			ALERT_SUSPICIOUS_RTC_TEMPERATURE,
			0,
			SIM808_GPS_STATUS::OFF
		};

		uint8_t alerts = core::notifyFailures(metadata);
		NOTICE_FORMAT("notifyFailures", "result : %B", alerts);
		alerts::add(alerts);
	}

	void clearAlerts() {
		PositionEntryMetadata metadata = {
			100, //all battery alerts should goes off with this
			3800, //doesn't matter
			10,
			0,
			SIM808_GPS_STATUS::OFF
		};

		alerts::clear(metadata);
	}
}