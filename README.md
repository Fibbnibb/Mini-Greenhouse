# ESP32 Greenhouse Monitor

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

## Known issues

1. **BME280 initialised twice** — the second `bme.begin(0x76)` block is redundant, and both use `while (1);` which hangs the board with no watchdog reset if the sensor is missing.
2. **Blocking actuator delays** — the 20 s fan delay and 5 s pump delay stop everything, including the web server. A page request during a fan cycle waits up to 20 s or times out.
3. **Web server is starved** — it's only polled once per loop, after a 1 s delay, so the page is always slow and occasionally unreachable. Move the client handling to the top of the loop and replace the delays with `millis()` timing.
4. **Wrong pin comment** — the `AOUT_PIN` comment says GPIO 36 / ADC0 but the value is 33. (33 is the correct choice, since ADC2 pins don't work while Wi-Fi is active.)
5. **Comment contradicts the threshold** — the pump condition reads "if soil moisture is below 500" but the test is `> 2470`. On most of these sensors a higher ADC value means *drier*, so the code is right and the comment is wrong.
6. **No hysteresis** — at 29.9–30.0 °C the fan cycles repeatedly. Add a deadband, e.g. on at 30 °C, off at 27 °C.
7. **Malformed HTML** — the `</table>` tag is missing, and soil moisture is labelled with the unit `m`, which isn't a unit of anything here. It's a raw 0–4095 ADC count; consider converting to a percentage.
8. **`header` is collected but never used** — no request routing, so every URL returns the same page.
9. **Return value of `display.begin()` is ignored**, so a missing OLED fails silently.
10. **Unused code** — `SEALEVELPRESSURE_HPA` and `delayTime` are declared but never read, and `SPI.h` isn't needed for an I2C-only build.
11. **OLED overflow** — four lines at text size 1 exactly fill 32 px, and "Soil moisture: " plus a four-digit value is wider than 128 px, so it wraps and pushes content off screen. Shorten the label to "Soil:".

## Build

Install the ESP32 board package and the libraries above, set your Wi-Fi credentials,
upload, then open Serial Monitor at 9600 baud to find the IP address.
