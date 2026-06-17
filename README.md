# 🏠 IoT Smart Home

This project presents an IoT-based smart home system developed using ESP32, Flask, HTML/CSS/JavaScript, and PlatformIO. The system provides real-time monitoring and remote control through a web dashboard, combining automation, security, and environmental sensing in a single platform.

The smart home continuously measures temperature and humidity using a DHT11 sensor and displays the values on a live dashboard together with a real-time temperature graph. Lighting can be controlled manually from the website or automatically according to ambient brightness detected by an LDR sensor. A PIR sensor enables motion detection capabilities.

Security functions are implemented using an RFID authentication system, an ultrasonic sensor for intrusion detection, and a buzzer alarm. The system can be armed or disarmed remotely through the web interface or locally using an authorized RFID card. An MQ-2 gas sensor provides smoke and gas leakage detection, while the gas alarm can also be controlled remotely.

Communication between the ESP32 and the Flask server is performed through Wi-Fi and REST APIs, allowing sensor data and commands to be exchanged in real time. The project demonstrates the integration of embedded systems, web technologies, and wireless communication to create an affordable and practical smart home solution.

Project was made by two person: Anano Avazashvili, Andro Tepnadze.
Drive link for videos: https://drive.google.com/drive/u/1/folders/1EJCMkJJfbG0gE4r6hSJose6e3JajwSBf
