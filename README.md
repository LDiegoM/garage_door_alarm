# garage_door_alarm

Garage opened door alarm build with Arduino and IR sensor.

It was developed using PaltformIO with Visual Studio Code.

In [this document](./doc/README.md) you'll find the electronic details to assemble this project.

# Features

Garage door alarm uses and IR sensor to detect and obstacle condition. that is translated as a door closed or opened.

It also connects to an MQTT server to publish this door status to the topic `topic-door-status`.

The program logs door status changes in the SPIFFS memory using LittleFS, in the file `/logs/door_data.txt` (defined in settings file).

# Getting started

When garage_door_alarm is running with an empty configuration, it'll not be able to connect to MQTT and date time services. To allow user to configure the program, it creates a WiFi access point called `garage_door_alarm`. You can connect to this WiFi network and access the next [link](http://192.168.4.1) to configure garage_door_alarm.

After this, when garage_door_alarm is running connected to a WiFi network and MQTT services, then you can configure it accessing to `http://meteo_station_ip_address/`.

# MQTT service

I choose [Hime MQ](https://www.hivemq.com/) MQTT service for testing porpuses. [Here](./doc/mqtt_ca_root.crt) you'll find the root ca certificate sor a secure connection to this service.

# MQTT monitoring

For monitoring porpuses I configured a complet IoT dashboard using [Iot MQTT Panel](https://play.google.com/store/apps/details?id=snr.lab.iotmqttpanel.prod) application for Android.

<!--[Here](./doc/IoTMQTTPanel.json) is a JSON file to import in IoT MQTT Panel application to built the default dashboards.-->

<!--![Dashboard](./doc/IoTMQTTPanel_dashboard.jpg) ![Settings](./doc/IoTMQTTPanel_settings.jpg)-->

# MQTT management queues

Garage door alarm listens from the topic `topic-door-cmd` to receive different commands:

- `RESEND`: when this command is received, the aplication will send current door status to MQTT topic.
- `GET_IP`: the application will publish current local IP to topic `topic-door-res-ip`.
- `GET_LOG`: the application will publish last door event timestamp to topic `topic-door-res-log`, in format `yyyy-mm-dd hh:mm:ss`.
- `GET_LOG_SIZE`: the application will publish current door events file size to topic `topic-door-res-logsize`.
- `GET_STO_FREE`: the application will publish current storage free size to topic `topic-door-res-freesto`.

# Configuration and management API Endpoints

Garage door status implements API endpoints to allow configuration and management, listening in port 80.

<!--[Here](https://app.swaggerhub.com/apis-docs/LDiegoM/meteo_station/1.0.0) is the Open API Documentation.-->

# License

Garage door status is an open source project by Diego M. Lopez that is licensed under [MIT](https://opensource.org/licenses/MIT). Diego M. Lopez reserves the right to change the license of future releases.

# Author

- Main idea, development and functional prototype by Diego M. Lopez (ldiegom@gmail.com)

# Changelog

## Unreleased

- Refactor for multiple device types.
- Increase wifi connection timeout.
- Add doorbell button.
- Add wifi connection indicator.

## 0.0.1

- First projects POC.
