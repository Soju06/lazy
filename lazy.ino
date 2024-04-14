/*
 Project Name:		Lazy - A more intuitive watch
 Author:			soju06
 Created:			2/11/2024 9:46:37 PM
 Developed using NodeMCU v1.0 (ESP8266) 80MHz and SSD1306 I2C 128x64 OLED.

 Pin Configuration:
 	- SSD1306 I2C 128x64 OLED
		- SDA: D2
		- SCL: D1
		- RES: RST
		- VCC: 3.3V
		- GND: GND
*/

#include <time.h>
#include <WString.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include "src/config.hpp"

Adafruit_SSD1306 display(PRIMARY_DISPLAY_WIDTH, PRIMARY_DISPLAY_HEIGHT, &Wire, PRIMARY_DISPLAY_RESET_PIN);

WiFiUDP ntpUdp;
NTPClient ntpTime(ntpUdp, "kr.pool.ntp.org", 32400, 60000);
// In 2038, this 32bit NTP client will not work :(

WiFiServer displayEchoServer(LAZY_DISPLAY_ECHO_PORT);
WiFiClient displayEchoClient;

void setup() {
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.println("Hello, World!");

	if (!display.begin(PRIMARY_DISPLAY_POWER_SOURCE == 0 ? SSD1306_SWITCHCAPVCC : SSD1306_EXTERNALVCC, PRIMARY_DISPLAY_I2C_ADDRESS)) {
		Serial.println(F("SSD1306 allocation failed"));
		delay(30000);
		ESP.restart();
		return;
	}

	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
	display.setTextWrap(true);
	display.setCursor(0, 0);
	display.clearDisplay();
	display.write("_");
	display.display();

	delay(1000);

	display.clearDisplay();
	display.setCursor(0, 0);
	display.printf("Connecting to %s\n", WIFI_SSID);
	display.display();

	WiFi.mode(WIFI_STA);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	WiFi.setSleepMode(WIFI_NONE_SLEEP);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	WiFi.setHostname(WIFI_HOSTNAME);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		display.print(".");
		display.display();

		if (WiFi.status() == WL_WRONG_PASSWORD) {
			display.println("\nWrong Password");
			display.display();
			delay(30000);
			ESP.restart();
			return;
		}
	}

	display.printf("OK\n\nIP: %s\n\n", WiFi.localIP().toString());
	display.display();

	if (LAZY_DISPLAY_ECHO_PORT) {
		displayEchoServer.begin(LAZY_DISPLAY_ECHO_PORT, 1);
	}

	ntpTime.begin();
	ntpTime.update();

	{
		time_t	now = ntpTime.getEpochTime();
		tm		ts	= *localtime(&now);

		display.printf("%d/%d/%d\n%d:%02d:%02d %s\n",
			ts.tm_mon + 1, ts.tm_mday, 1900 + ts.tm_year,
			ts.tm_hour > 12 ? ts.tm_hour - 12 : ts.tm_hour, ts.tm_min, ts.tm_sec,
			ts.tm_hour > 12 ? "PM" : "AM");
		display.display();
	}

	delay(1000);
}

constexpr uint8_t AL = 0b00000001;
constexpr uint8_t AR = 0b00000010;
constexpr uint8_t AH = AL | AR;
constexpr uint8_t AT = 0b00010000;
constexpr uint8_t AB = 0b00100000;
constexpr uint8_t AV = AT | AB;
constexpr uint8_t AC = AH | AV;

static void alignText(uint8_t align, int16_t x, int16_t y, const char* text, ...) {
	va_list args{};
	va_start(args, text);
	char buffer[128]{};
	vsnprintf(buffer, sizeof(buffer), text, args);
	va_end(args);

	int16_t x1 = 0, y1 = 0;
	uint16_t w, h;
	display.getTextBounds((const char*)buffer, 0, 0, &x1, &y1, &w, &h);

	if ((align & AH) == AH) {
		x += (display.width() - w) / 2;
	} else if ((align & AR) == AR) {
		x = display.width() - w - x;
	}

	if ((align & AV) == AV) {
		y += (display.height() - h) / 2;
	} else if ((align & AB) == AB) {
		y = display.height() - h - y;
	}

	int16_t ox = display.getCursorX();
	int16_t oy = display.getCursorY();

	display.setCursor(x, y);
	display.print(buffer);
	display.setCursor(ox, oy);
}

time_t		lastTime				= 0;
uint32_t	lastTimeMillis			= 0;

static uint64_t getTime() {
	return lastTime * 1000 + (static_cast<long long>(millis()) - lastTimeMillis);
}

static uint32_t getRemaining(uint32_t millis, uint32_t targetMillis) {
	return millis < targetMillis ? targetMillis - millis : 86400000 - millis + targetMillis;
}

static float calculateDayRatio(uint64_t time, uint32_t wakeTime, uint32_t sleepTime) {
	auto millis = time % 86400000;
	auto wake = getRemaining(millis, wakeTime);
	auto sleep = getRemaining(millis, sleepTime);

	return sleep < wake ?
		1 - (float)sleep / getRemaining(wakeTime, sleepTime) :
		2 - (float)wake / getRemaining(sleepTime, wakeTime);
}

WiFiClient weatherClient;
uint64_t lastWeatherUpdate = 0;
uint8_t lastWeatherStatus = 0;
String weatherData = "N/A";

void updateWeather() {
	if (!lastWeatherStatus && lastWeatherUpdate && getTime() - lastWeatherUpdate < LAZY_WEATHER_UPDATE_INTERVAL) {
		return;
	}

	if (lastWeatherStatus == 0) {
		if (!weatherClient.connect(LAZY_WEATHER_HOST, LAZY_WEATHER_PORT)) {
			weatherData = "C_ERR";
		} else {
			lastWeatherStatus++;
		}
		return;
	}

	if (!weatherClient.connected()) {
		weatherData = "D_ERR";
		lastWeatherStatus = 0;
		weatherClient.stop();
		return;
	}

	switch (lastWeatherStatus) {
	case 1:
		{
			auto path = String(LAZY_WEATHER_PATH);
			path.replace("{zone}", LAZY_WEATHER_ZONE);
			weatherClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
				"Host: " + LAZY_WEATHER_HOST + "\r\n" +
				"Connection: close\r\n\r\n");
			lastWeatherStatus++;
			return;
		}

	case 2:
		if (weatherClient.available()) {
			auto line = weatherClient.readStringUntil('\n');
			auto startTempIndex = line.indexOf("<temp>");
			auto endTempIndex = line.indexOf("</temp>");

			if (startTempIndex != -1 && endTempIndex != -1) {
				weatherData = line.substring(startTempIndex + 6, endTempIndex) + (char)167 + "C";
				lastWeatherStatus++;
			}
		}
		return;

	case 3:
		lastWeatherUpdate = getTime();
		lastWeatherStatus = 0;
		weatherClient.stop();
		return;
	}
}


WiFiClient airQualityClient;
uint64_t lastAirQualityUpdate = 0;
uint8_t lastAirQualityStatus = 0;

uint16_t caiValues[LAZY_AIR_STATION_COUNT];
uint16_t caiCount = 0;
String airQualityData = "N/A";

void updateAirQuality() {
	if (!lastAirQualityStatus && lastAirQualityUpdate && getTime() - lastAirQualityUpdate < LAZY_AIR_UPDATE_INTERVAL) {
		return;
	}

	if (lastAirQualityStatus == 0) {
		if (!airQualityClient.connect(LAZY_AIR_HOST, LAZY_AIR_PORT)) {
			airQualityData = "C_ERR";
		} else {
			lastAirQualityStatus++;
		}
		return;
	}

	if (!airQualityClient.connected()) {
		if (lastAirQualityStatus < 2 || !caiCount) {
			airQualityData = "ERR";
			lastAirQualityStatus = 0;
			airQualityClient.stop();
			return;
		}

		lastAirQualityStatus = 3;
	}

	switch (lastAirQualityStatus) {
	case 1:
	{
		auto path = String(LAZY_AIR_PATH);
		path.replace("{zoneName}", LAZY_AIR_ZONE);
		path.replace("{apiKey}", LAZY_AIR_API_KEY);
		path.replace("{stationCount}", String(LAZY_AIR_STATION_COUNT));

		airQualityClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
			"Host: " + LAZY_AIR_HOST + "\r\n" +
			"Connection: close\r\n\r\n");
		lastAirQualityStatus++;
		caiCount = 0;
		return;
	}

	case 2:
		if (airQualityClient.available()) {
			auto line = airQualityClient.readStringUntil('\n');
			auto startCAIIndex = line.indexOf("<khaiValue>");
			auto endCAIIndex = line.indexOf("</khaiValue>");

			if (startCAIIndex != -1 && endCAIIndex != -1) {
				auto valueString = line.substring(startCAIIndex + 11, endCAIIndex);
				auto value = valueString.toInt();

				if (!value && valueString != "0") {
					return;
				}

				caiValues[caiCount++] = value;

				if (!lastAirQualityUpdate) {
					auto ratio = (float)caiCount / LAZY_AIR_STATION_COUNT;
					airQualityData = String((int)(ratio * 100)) + "%%";
				}
			}
		}
		return;

	case 3:
		{
			auto cai = 0;

			for (auto i = 0; i < caiCount; i++) {
				cai += caiValues[i];
			}

			cai /= caiCount;

			if (cai <= 50) {
				airQualityData = "Good";
			} else if (cai <= 100) {
				airQualityData = "Moderate";
			} else if (cai <= 250) {
				airQualityData = "Unhealthy";
			} else if (cai < 500) {
				airQualityData = "Very Unhealthy";
			} else {
				airQualityData = "Hazardous";
			}
		}

		lastAirQualityUpdate = getTime();
		lastAirQualityStatus = 0;
		airQualityClient.stop();
		return;
	}
}

static void updateDisplayEcho() {
	if constexpr (!LAZY_DISPLAY_ECHO_PORT) {
		return;
	}

	if (!displayEchoClient.connected()) {
		displayEchoClient = displayEchoServer.accept();
		return;
	}

	if (!displayEchoClient.available()) {
		return;
	}

	displayEchoClient.read();

	auto bufferSize = display.width() * display.height() / 8;

	if (displayEchoClient.write(display.getBuffer(), bufferSize) != bufferSize) {
		displayEchoClient.stop();
		return;
	}
}

uint64_t	lastFlickeringMillis	= 0;
float		lastFlickeringRatio		= 0;

void loop() {
	ntpTime.update();

	{
		time_t now = ntpTime.getEpochTime();

		if (lastTime != now) {
			lastTime = now;
			lastTimeMillis = millis();
		}
	}

	updateWeather();
	updateAirQuality();

	auto  	now				= getTime();
	time_t	epoch			= now / 1000;
	auto	millis			= now % 1000;
	auto	time			= *localtime(&epoch);
	auto	dayMinutes		= time.tm_hour * 60 + time.tm_min;
	auto	dayRatio		= calculateDayRatio(now, LAZY_WAKEUP_TIME, LAZY_SLEEP_TIME);

	if (!lastFlickeringRatio) {
		lastFlickeringRatio = (int)((1 - dayRatio) * 100);
	}

	display.clearDisplay();
	display.setCursor(0, 0);

	// Draw time percentage
	if (dayRatio < 1) {
		bool isVisible = true;
		auto percentage = (1 - dayRatio) * 100;

		if ((int)percentage != lastFlickeringRatio) { // flickering effect
			lastFlickeringRatio = (int)percentage;
			lastFlickeringMillis = now;
		}

		if (now - lastFlickeringMillis < LAZY_FLICKERING_DURATION) {
			isVisible = (now - lastFlickeringMillis) % (static_cast<unsigned long long>(LAZY_FLICKERING_INTERVAL) * 2) < LAZY_FLICKERING_INTERVAL;
			percentage = floorf(percentage) + 1;
		}

		if (isVisible) {
			display.setTextSize(2);
			alignText(AC, 0, 0, "%.04f%%", percentage);
		}
	} else {
		auto percentage = (1 - (dayRatio - 1)) * 100;
		display.setTextSize(2);
		alignText(AC, 0, -7, "%d:%02d", time.tm_hour, time.tm_min);
		display.setTextSize(1);
		alignText(AC, 0, 5, "%.02f%%", percentage);
	}

	display.setTextSize(1);

	// Draw other time information
	alignText(AR | AB, 0, 0, "%d/%d/%d", time.tm_mon + 1, time.tm_mday, 1900 + time.tm_year);
	alignText(AL | AB, 0, 0, "%d:%02d:%02d %s", time.tm_hour > 12 ? time.tm_hour - 12 : time.tm_hour,
		time.tm_min, time.tm_sec,time.tm_hour > 12 ? "PM" : "AM");

	display.cp437(true);

	// Draw weather information
	alignText(AR | AT, 0, 0, weatherData.c_str());
	// Draw air quality information
	alignText(AL | AT, 0, 0, airQualityData.c_str());

	display.cp437(false);

	display.display();

	updateDisplayEcho();
}
