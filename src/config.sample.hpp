#pragma once

#include <stdint.h>


constexpr uint32_t		LAZY_WAKEUP_TIME				= (07 * 60 + 00) * 60 * 1000;			// 07:00 AM
constexpr uint32_t		LAZY_SLEEP_TIME					= (23 * 60 + 40) * 60 * 1000;			// 11:40 PM

constexpr uint32_t		LAZY_WEATHER_UPDATE_INTERVAL	= 600000;								// 10 minutes
constexpr const char*	LAZY_WEATHER_ZONE				= "1159066000";							// www.weather.go.kr RSS Zone Code
constexpr const char*	LAZY_WEATHER_HOST				= "www.weather.go.kr";
constexpr uint16_t		LAZY_WEATHER_PORT				= 80;
constexpr const char*	LAZY_WEATHER_PATH				= "/w/rss/dfs/hr1-forecast.do?zone={zone}";

constexpr uint32_t		LAZY_AIR_UPDATE_INTERVAL		= 1200000;								// 20 minutes
constexpr const char*	LAZY_AIR_ZONE					= "%EB%8F%99%EC%9E%91%EA%B5%AC";		// Encoded Dongjak-gu
constexpr uint16_t		LAZY_AIR_STATION_COUNT			= 20;
constexpr const char*	LAZY_AIR_API_KEY				= "Zo74qDEhPe0yaB%2B%2BfTJVWQOtJ9D67%2BTCrDgC..."; // Encoded API Key (https://www.data.go.kr/tcs/dss/selectApiDataDetailView.do?publicDataPk=15073861)
constexpr const char*	LAZY_AIR_HOST					= "apis.data.go.kr";
constexpr uint16_t		LAZY_AIR_PORT					= 80;
constexpr const char*	LAZY_AIR_PATH = "/B552584/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?serviceKey={apiKey}&returnType=xml&numOfRows={stationCount}&pageNo=1&stationName={zoneName}&dataTerm=DAILY&ver=1.5";

constexpr uint16_t		LAZY_FLICKERING_INTERVAL		= 500;									// 0.5 seconds
constexpr uint16_t		LAZY_FLICKERING_DURATION		= 3000;									// 3 seconds

constexpr uint16_t		LAZY_DISPLAY_ECHO_PORT			= 1024;									// if value is 0, then no echo server is used


constexpr uint32_t		SERIAL_BAUD_RATE				= 115200;

constexpr int8_t		I2C_MAX_DEVICES					= 16;									// 1 ~ 128 (7-bit address)
constexpr int8_t		I2C_SDA_PIN						= 4;									// D2 - GPIO4
constexpr int8_t		I2C_SCL_PIN						= 5;									// D1 - GPIO5
constexpr uint32_t		I2C_CLOCK_SPEED					= 400000;								// 100kHz, 400kHz, 1MHz, 3.4MHz

constexpr uint8_t		PRIMARY_DISPLAY_WIDTH			= 128;
constexpr uint8_t		PRIMARY_DISPLAY_HEIGHT			= 64;
constexpr int8_t		PRIMARY_DISPLAY_RESET_PIN		= -1;									// D0 - GPIO16. if value is -1, then no reset pin is used
constexpr uint8_t		PRIMARY_DISPLAY_I2C_ADDRESS		= 0x3C;									// 0x3C or 0x3D
constexpr uint8_t		PRIMARY_DISPLAY_POWER_SOURCE	= 0;									// 0 = VCC, 1 = External

constexpr const char*	WIFI_SSID						= "soju";
constexpr const char*	WIFI_PASSWORD					= "iLoveSoju";
constexpr const char*	WIFI_HOSTNAME					= "Lazy";