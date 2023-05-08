# Obslužný firmware napájecí jednotky

[![Pipeline status](https://gitlab.com/sbc-pdu/pdu-unit/firmware/badges/master/pipeline.svg)](https://gitlab.com/sbc-pdu/pdu-unit/firmware/-/commits/master)
[![Apache License](https://img.shields.io/badge/license-Apache2-blue.svg)](LICENSE)

Tento repozitář obsahuje obslužný firmware napájecí jednotky.

## Prerekvizity

- Node.js 18.x
- NPM
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#installation)

## Sestavení

1. Naklonujte si tento repozitář pomocí příkazu:
	```bash
	git clone https://gitlab.com/sbc-pdu/pdu-unit/firmware.git
	```
2. Přesuňte se do složky `webapp` pomocí příkazu:
	```bash
	cd webapp
	```
3. Nainstaluje závislosti frontendu pomocí příkazu:
	```bash
	npm install
	```
4. Sestavte frontend pomocí příkazu:
	```bash
	npm run build
	```
5. Vraťte se do kořenové složky repozitáře pomocí příkazu:
	```bash
	cd ../
	```
6. Upravte výchozí konfiguraci napájecí jednotky ve funkci `initNvs` v souboru `main/main.cpp`:
```cpp
void initNvs() {
	ESP_ERROR_CHECK(NvsManager::init());
	// WiFi NVS
	NvsManager wifiNvs("wifi");
	/* Zabezpečení WiFi z výčtu [wifi_auth_mode_t](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv416wifi_auth_mode_t) */
	wifiNvs.setDefault("authmode", static_cast<uint8_t>(WIFI_AUTH_WPA2_PSK));
	/* SSID WiFi sítě */
	wifiNvs.setStringDefault("ssid", "SSID");
	/* Heslo WiFi sítě */
	wifiNvs.setStringDefault("psk", "password");
	wifiNvs.commit();
	// MQTT NVS
	NvsManager mqttNvs("mqtt");
	/* Adresa MQTT brokeru */
	mqttNvs.setStringDefault("uri", "mqtts://mqtt.romanondracek.cz:8883");
	/* Uživatelské jméno k MQTT brokeru */
	mqttNvs.setStringDefault("username", "sbc_pdu");
	/* Heslo k MQTT brokeru */
	mqttNvs.setStringDefault("password", "password");
	mqttNvs.commit();
	// SNTP NVS
	NvsManager ntpNvs("ntp");
	/* Počet NTP serverů */
	uint8_t ntpServers = 2;
	ntpNvs.setDefault("servers", ntpServers);
	/* Adresa prvního NTP serveru */
	ntpNvs.setStringDefault("server0", "tik.cesnet.cz");
	/* Adresa druhého NTP serveru */
	ntpNvs.setStringDefault("server1", "tak.cesnet.cz");
	/* Časová zóna */
	ntpNvs.setStringDefault("timezone", "CET/CEST");
	ntpNvs.commit();
	// HTTP credentials NVS
	NvsManager httpCredentialsNvs("httpCredentials");
	/* Uživatelské jméno do webového rozhraní */
	httpCredentialsNvs.setStringDefault("username", "admin");
	/* Heslo do webového rozhraní */
	httpCredentialsNvs.setStringDefault("password", "sbc-pdu");
	httpCredentialsNvs.commit();
}
```
7. Sestavte firmware pomocí příkazu:
	```bash
	idf.py build
	```

## Nahrání firmwaru do zařízení

Firmware do zařízení (typicky na portu `/dev/ttyUSB0`) nahrajeme pomocí příkazu:
```bash
idf.py --port /dev/ttyUSB0 flash
```

Výstup ze sériové konzole si můžete zobrazit pomocí příkazu:
```bash
idf.py --port /dev/ttyUSB0 monitor
```
Ze sériové konzole se odchází pomocí `Ctrl+]`.
