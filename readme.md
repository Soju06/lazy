# Lazy
더 직관적인 시계

#### 더 이상 해야 할 일을 내일로 미루지 마세요.

하루의 활동 시간을 지정하고, 남은 활동 시간을 퍼센트로 보여줍니다.

## Requirements

- ESP32 또는 ESP8266 기반의 보드 (NodeMCU 권장)
- SSD1306 I2C OLED 디스플레이 (128x64 권장)
- Arduino 및 VisualMicro가 설치된 Visual Studio 2022
- [한국환경공단_에어코리아_대기오염정보](https://www.data.go.kr/tcs/dss/selectApiDataDetailView.do?publicDataPk=15073861)에서 발급한 API 키

## Configuration

레포지토리를 클론하고, `src/config.sample.hpp` 파일을 `src/config.hpp`로 복사합니다.

그 후, `config.hpp` 파일을 열어 자신의 환경에 맞게 설정합니다.

```cpp
constexpr uint32_t          LAZY_WAKEUP_TIME                = (07 * 60 + 00) * 60 * 1000;           // 07:00 AM (활동 시작 시간)
constexpr uint32_t          LAZY_SLEEP_TIME                 = (23 * 60 + 40) * 60 * 1000;           // 11:40 PM (활동 종료 시간)

constexpr uint32_t          LAZY_WEATHER_UPDATE_INTERVAL    = 600000;                               // 10 minutes (날씨 정보 업데이트 주기)
constexpr const char*       LAZY_WEATHER_ZONE               = "1159066000";                         // www.weather.go.kr RSS Zone Code (https://www.weather.go.kr/w/pop/rss-guide.do 에서 자신의 지역 코드 확인)
constexpr const char*       LAZY_WEATHER_HOST               = "www.weather.go.kr";
constexpr uint16_t          LAZY_WEATHER_PORT               = 80;
constexpr const char*       LAZY_WEATHER_PATH               = "/w/rss/dfs/hr1-forecast.do?zone={zone}";

constexpr uint32_t          LAZY_AIR_UPDATE_INTERVAL        = 1200000;                              // 20 minutes (대기 오염 정보 업데이트 주기)
constexpr const char*       LAZY_AIR_ZONE                   = "%EB%8F%99%EC%9E%91%EA%B5%AC";        // Encoded Dongjak-gu (https://www.airkorea.or.kr/web/stationInfo?pMENU_NO=93 에서 측정소 명 확인 가능)
constexpr uint16_t          LAZY_AIR_STATION_COUNT          = 20;
constexpr const char*       LAZY_AIR_API_KEY                = "Zo74qDEhPe0yaB%2B%2BfTJVWQOtJ9D67%2BTCrDgC..."; // Encoded API Key (https://www.data.go.kr/tcs/dss/selectApiDataDetailView.do?publicDataPk=15073861)
constexpr const char*       LAZY_AIR_HOST                   = "apis.data.go.kr";
constexpr uint16_t          LAZY_AIR_PORT                   = 80;
constexpr const char*       LAZY_AIR_PATH                   = "/B552584/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?serviceKey={apiKey}&returnType=xml&numOfRows={stationCount}&pageNo=1&stationName={zoneName}&dataTerm=DAILY&ver=1.5";

constexpr uint16_t          LAZY_FLICKERING_INTERVAL        = 500;                                  // 0.5 seconds
constexpr uint16_t          LAZY_FLICKERING_DURATION        = 3000;                                 // 3 seconds

constexpr uint16_t          LAZY_DISPLAY_ECHO_PORT          = 1024;                                 // if value is 0, then no echo server is used


constexpr uint32_t          SERIAL_BAUD_RATE                = 115200;

constexpr int8_t            I2C_MAX_DEVICES                 = 16;                                   // 1 ~ 128 (7-bit address)
constexpr int8_t            I2C_SDA_PIN                     = 4;                                    // D2 - GPIO4
constexpr int8_t            I2C_SCL_PIN                     = 5;                                    // D1 - GPIO5
constexpr uint32_t          I2C_CLOCK_SPEED                 = 400000;                               // 100kHz, 400kHz, 1MHz, 3.4MHz

constexpr uint8_t           PRIMARY_DISPLAY_WIDTH           = 128;
constexpr uint8_t           PRIMARY_DISPLAY_HEIGHT          = 64;
constexpr int8_t            PRIMARY_DISPLAY_RESET_PIN       = -1;                                   // D0 - GPIO16. if value is -1, then no reset pin is used
constexpr uint8_t           PRIMARY_DISPLAY_I2C_ADDRESS     = 0x3C;                                 // 0x3C or 0x3D
constexpr uint8_t           PRIMARY_DISPLAY_POWER_SOURCE    = 0;                                    // 0 = VCC, 1 = External

constexpr const char*       WIFI_SSID                       = "soju";
constexpr const char*       WIFI_PASSWORD                   = "iLoveSoju";
constexpr const char*       WIFI_HOSTNAME                   = "Lazy";
```

## Day mode

<img src="https://github.com/Soju06/lazy/assets/34199905/ef048e0d-baa4-4308-96dd-11a9bf878886" alt="lazy-3" width="300"/>

## Night mode

<img src="https://github.com/Soju06/lazy/assets/34199905/cd993e9a-f0a0-4bf3-be52-93d4e1947b40" alt="lazy-2" width="300"/>

## Echo Client

![lazy-1](https://github.com/Soju06/lazy/assets/34199905/a912116a-1d32-4f2c-8c58-7dd79e2b51bf)
