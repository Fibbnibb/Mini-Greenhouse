# ESP32 Greenhouse

An ESP32 reads temperature, pressure, humidity, and soil moisture, shows them on a
small OLED, switches a fan and a water pump when thresholds are crossed, and serves
the readings as a web page on the local network.

## Hardware

- ESP32 dev board
- BME280 temperature / pressure / humidity sensor — I2C, address `0x76`
- SSD1306 OLED, 128×32 — I2C, address `0x3C`
- Analog soil moisture sensor — AOUT to **GPIO 33**
- Fan (via relay or MOSFET) — **GPIO 16**
- Water pump (via relay or MOSFET) — **GPIO 14**

I2C uses the ESP32 defaults (SDA 21, SCL 22). Drive the fan and pump through a relay
or transistor, never straight off a GPIO.

## Libraries

`Adafruit BME280`, `Adafruit SSD1306`, `Adafruit GFX`, `Adafruit Unified Sensor`.
`Wire`, `SPI`, and `WiFi` ship with the ESP32 core.

## Configuration

```cpp
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

The sketch prints its IP address to serial at **9600 baud** once connected. Open that
address in a browser to see the readings table.

## Behaviour

Each loop pass (~1 s, longer when an actuator fires):

1. Read BME280 and the soil moisture ADC.
2. Redraw the OLED.
3. Fan on for 20 s if temperature ≥ 30 °C.
4. Pump on for 5 s if the soil reading is above 2470.
5. Print readings to serial.
6. Serve one HTTP client if any is waiting.

## Build

Install the ESP32 board package and the libraries above, set your Wi-Fi credentials,
upload, then open Serial Monitor at 9600 baud to find the IP address.
