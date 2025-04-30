# Smart IoT-Based Parking Management System Using ESP32-CAM and Custom ANPR  Model

## Project Overview
The **Smart Parking Management** project is an IoT-based solution designed to automate vehicle parking processes using the ESP32-CAM module and Automatic Number Plate Recognition (ANPR). The system captures vehicle images, sends them to a local server for number plate recognition, and controls access using IR sensors and a servo motor. A web interface provides real-time status updates, including time, system status, and recognized number plates. The project integrates hardware (ESP32-CAM, IR sensors, servo motor, flashlight) and software (ANPR model, local server) to create an efficient parking management system suitable for small-scale parking lots.

This repository contains all the code, documentation, and resources needed to replicate the project, including ESP32 firmware, server-side code, ANPR model, testing scripts, and circuit diagrams.

## Key Components
- **ESP32-CAM**: A microcontroller with an OV2640 camera module used for capturing vehicle images and hosting a web server for status monitoring.
- **IR Sensors**: Two infrared sensors detect vehicle entry (GPIO 16) and exit (GPIO 15), triggering image capture and servo actions.
- **Servo Motor**: Controls a gate mechanism, toggling between 0° and 90° based on entry IR sensor triggers (GPIO 14).
- **Flashlight**: An LED (GPIO 4) illuminates the scene during image capture for better visibility.
- **Local Server**: A Flask-based server (`192.168.29.56:5000/upload`) processes images, performs ANPR, and returns JSON responses with number plate and image URL data.
- **ANPR Model**: A Jupyter Notebook (`anpr.ipynb`) implements the Automatic Number Plate Recognition model using computer vision techniques.
- **Web Interface**: A simple HTML interface served by the ESP32-CAM displays real-time system status, time, last event, and captured image links.

## Repository Structure
The repository is organized into three main folders: `ANPR`, `ESP32-Cam`, and `image`. Below is a detailed breakdown:

```
Smart-Parking-Management/
├── ANPR/
│   ├── anpr.ipynb
│   └── server/
│       ├── server_code/
│       │   └── [server code files]
│       └── server_test/
│           ├── [test code files]
│           ├── [real car images]
│           └── [toy car images]
├── ESP32-Cam/
│   ├── final_parking/
│   │   ├── parking.ino
│   │   └── Motor_check_code.ino
│   └── extra/
│       ├── IR_ultrasonic_motor_test.ino
│       └── manual-click.ino
├── image/
│   ├── circuit.png
│   ├── ESP32-CAM-Pinout.jpg
│   ├── esp_32_cam_connection.jpg
│   └── flowchart.png
└── README.md
```

### Folders and Files
- **ANPR/**
  - `anpr.ipynb`: Jupyter Notebook containing the Automatic Number Plate Recognition model, which processes vehicle images to extract number plates.
  - `server/`
    - `server_code/`: Contains the Flask server code that handles image uploads, runs ANPR, and returns JSON responses with number plate and image URL.
    - `server_test/`: Includes test scripts to verify server functionality and a collection of images (real and toy cars) for testing the ANPR model and server.

- **ESP32-Cam/**
  - `final_parking/`
    - `parking.ino`: The main ESP32-CAM firmware. It captures images, pushes them to the local server for ANPR, controls the servo based on IR sensor input, and serves a web interface.
    - `Motor_check_code.ino`: Code to test the servo motor operation, likely used for debugging motor functionality with IR sensors.
  - `extra/`
    - `IR_ultrasonic_motor_test.ino`: Test code for verifying servo motor operation with IR and ultrasonic sensors, useful for initial hardware setup and debugging.
    - `manual-click.ino`: Code to manually capture images with the ESP32-CAM and send them to the server, used to test server connectivity and ANPR functionality.

- **image/**
  - `circuit.png`: Circuit diagram illustrating the hardware connections for the ESP32-CAM, IR sensors, servo, and flashlight.
  - `ESP32-CAM-Pinout.jpg`: Pinout diagram for the ESP32-CAM module, aiding in hardware setup.
  - `esp_32_cam_connection.jpg`: Image showing the physical connections for the ESP32-CAM and peripherals.
  - `flowchart.png`: Flowchart depicting the system’s operational workflow, from sensor detection to image processing and servo control.

## Key Code Components
The main firmware (`ESP32-Cam/final_parking/parking.ino`) includes several critical functions and structures that drive the smart parking system. Below are their roles:

- **PlateEntry Structure**:
  The `PlateEntry` structure stores information about a vehicle’s valid number plate and its entry time. When a vehicle’s number plate is recognized by the server, this structure captures the plate number and entry timestamp for record-keeping and display on the web interface. It is used to log vehicle entries, though the current implementation simplifies this for exit events.

- **extractJsonStringValue Function**:
  This function parses a JSON string from the server’s response to extract values based on a specified key. It retrieves critical data, such as the recognized license plate number (`"number_plate"`) or the URL of the captured image (`"view_image"`), enabling the system to update the web interface with the latest information.

- **handleRoot Function**:
  The `handleRoot` function generates and serves the HTML content for the root web page, accessible via the ESP32’s IP address. It creates a user-friendly interface displaying the current time, system status (e.g., “Idle,” “Capturing Image”), last event (e.g., recognized plate or exit detection), and a link to the latest captured image. It also includes a button for manual image capture.

- **handleTrigger Function**:
  The `handleTrigger` function initiates image capture when triggered, either manually via the web interface or automatically when the entry IR sensor detects a vehicle. It updates the system status, calls `sendPhoto` to capture and upload the image, and refreshes the web interface to reflect the outcome (e.g., success or failure).

- **openBarrier Function**:
  The `openBarrier` function controls the servo motor to open the parking barrier by setting it to 0°, allowing a vehicle to enter after successful number plate recognition. This function is part of the entry sequence in earlier implementations but may be simplified in the current code to toggle the servo.

- **closeBarrier Function**:
  The `closeBarrier` function moves the servo motor to 90° to close the barrier after a vehicle has entered, securing the parking lot. Like `openBarrier`, it is part of the entry/exit logic in earlier versions but may be adapted for simpler servo toggling.

- **sendPhoto Function**:
  A core function that captures an image using the ESP32-CAM and uploads it to the local server (`192.168.29.56:5000/upload`) via HTTP POST. It activates the flashlight for better image quality, sends the image as multipart/form-data, and processes the server’s JSON response to extract the number plate and image URL. This drives the logic for updating the web interface and logging events.

- **setup Function**:
  The `setup` function runs once at startup to initialize the system. It performs the following tasks:
  - **Brownout Detector Disable**: Prevents ESP32 resets due to voltage fluctuations.
  - **Serial Communication & Pin Setup**: Initializes serial communication (115200 baud) for debugging and configures GPIO pins for the servo (GPIO 14), IR sensors (GPIO 16, 15), and flashlight (GPIO 4).
  - **Wi-Fi Setup**: Connects to the specified Wi-Fi network, enabling server communication and web interface access.
  - **NTPClient Initialization**: Configures the NTP client to fetch current time for timestamping events.
  - **Web Server Setup**: Starts the web server to handle client requests (e.g., displaying the interface or triggering captures).
  - **Camera Configuration**: Sets up the ESP32-CAM with appropriate frame size, quality, and pixel format for image capture.
  - **PWM and Servo Initialization**: Configures the servo motor with PWM settings for precise control.

- **loop Function**:
  The `loop` function runs continuously to manage the system’s operation. It handles:
  - **NTP Time Update**: Periodically updates the current time via the NTP client for accurate event timestamping.
  - **Web Server Handling**: Processes incoming client requests, such as serving the web interface or handling manual triggers.
  - **Vehicle Entry Handling**: When the entry IR sensor (GPIO 16) detects a vehicle (LOW signal), it delays to ensure proper positioning, calls `handleTrigger` to capture and upload an image, and toggles the servo (0°/90°) to simulate gate control. The server’s response updates the web interface.
  - **Vehicle Exit Handling**: When the exit IR sensor (GPIO 15) detects a vehicle, it logs an exit event (“Vehicle Exit Detected”) and updates the web interface. In earlier versions, it also managed barrier control and vehicle counts.

## Project Workflow
1. **Vehicle Detection**:
   - The entry IR sensor (GPIO 16) detects a vehicle, triggering the ESP32-CAM to capture an image.
   - The flashlight (GPIO 4) activates to ensure clear image capture.
2. **Image Processing**:
   - The image is sent to the local server (`192.168.29.56:5000/upload`) via HTTP POST.
   - The server runs the ANPR model to extract the number plate and returns a JSON response with the plate number and image URL.
3. **Servo Control**:
   - On entry IR sensor trigger, the servo motor (GPIO 14) toggles between 0° and 90°, simulating gate opening/closing.
4. **Exit Detection**:
   - The exit IR sensor (GPIO 15) detects a vehicle leaving, updating the web interface with a “Vehicle Exit Detected” event.
5. **Web Interface**:
   - The ESP32-CAM hosts a web server displaying real-time status, time, last event (e.g., number plate or exit event), and a link to the captured image.
   - A manual trigger button allows capturing images for testing.

## Hardware Setup
### Components
- **ESP32-CAM**: Core microcontroller with OV2640 camera.
- **IR Sensors (2)**: Entry (GPIO 16) and exit (GPIO 15) for vehicle detection.
- **Servo Motor**: Connected to GPIO 14, toggles between 0° and 90°.
- **Flashlight**: LED on GPIO 4 for illumination during capture.
- **Power Supply**:
  - ESP32-CAM: 5V 1A–2A (USB or external).
  - Servo: External 5V 1A–2A with common GND.
  - IR Sensors: 3.3V or 5V (check sensor specs).

### Connections
Refer to `image/circuit.png`, `image/ESP32-CAM-Pinout.jpg`, and `image/esp_32_cam_connection.jpg` for wiring. Key connections:
- **Entry IR Sensor**: Signal to GPIO 16, VCC (3.3V/5V), GND.
- **Exit IR Sensor**: Signal to GPIO 15, VCC (3.3V/5V), GND.
- **Servo**: Signal to GPIO 14, VCC (external 5V), GND (common).
- **Flashlight**: LED with 220Ω resistor to GPIO 4, GND.
- Ensure a stable power supply and common GND to avoid brownouts.

## Software Setup
### Prerequisites
- **Arduino IDE**:
  - Install the `esp32` board package by Espressif Systems.
  - Install the `ESP32Servo` library (Arduino Library Manager).
- **Python**:
  - Install Python 3.x for the server and ANPR model.
  - Install dependencies: `flask`, `opencv-python`, `pytesseract`, etc. (see `ANPR/anpr.ipynb` or server requirements).
- **Local Server**:
  - Run the server code in `ANPR/server/server_code/` on a machine accessible at `192.168.29.56:5000`.
  - Ensure the server accepts HTTP POST requests and returns JSON with `"number_plate"` and `"view_image"`.

### Installation
1. **ESP32-CAM**:
   - Open `ESP32-Cam/final_parking/parking.ino` in Arduino IDE.
   - Select “AI Thinker ESP32-CAM” (`Tools > Board > ESP32 Arduino`).
   - Set upload speed to 115200 (`Tools > Upload Speed`).
   - Disconnect peripherals, ground GPIO 0, upload, remove ground.
   - Update WiFi credentials (`ssid`, `password`) in `parking.ino`.
2. **Server**:
   - Navigate to `ANPR/server/server_code/`.
   - Run the Flask server (e.g., `python server.py`).
   - Verify accessibility at `http://192.168.29.56:5000/upload`.
3. **ANPR Model**:
   - Open `ANPR/anpr.ipynb` in Jupyter Notebook.
   - Install required libraries (listed in the notebook).
   - Test with images in `ANPR/server/server_test/`.

## Usage
1. **Power On**:
   - Connect the ESP32-CAM and peripherals to power.
   - Ensure the server is running.
2. **Monitor**:
   - Open the Serial Monitor (115200 baud) to view WiFi connection, sensor triggers, and server responses.
   - Access the ESP32’s IP address (printed in Serial Monitor) in a browser to view the web interface.
3. **Test Sensors**:
   - **Entry IR**: Block the sensor to trigger image capture, servo toggle, and server upload. Check the web interface for updated status and image link.
   - **Exit IR**: Block the sensor to log an exit event (“Vehicle Exit Detected”).
   - **Manual Trigger**: Click “Capture Image” on the web interface to test image capture and upload.
4. **Test Server and ANPR**:
   - Use `ANPR/server/server_test/` scripts and images to verify server and ANPR functionality.
   - Upload test images (real/toy cars) to ensure accurate number plate recognition.

## Troubleshooting
- **Servo Not Rotating**:
  - Verify wiring (GPIO 14, external 5V, common GND).
  - Test with `ESP32-Cam/final_parking/Motor_check_code.ino` or `ESP32-Cam/extra/IR_ultrasonic_motor_test.ino`.
  - Ensure external 5V 1A+ supply; add 100µF capacitor across servo VCC/GND if jitter occurs.
- **IR Sensor Issues**:
  - Confirm sensors output LOW when triggered (test with Serial print or multimeter).
  - Check VCC (3.3V/5V), GND, or replace sensor.
  - Adjust `INPUT_PULLUP` or add 10kΩ pull-up resistor if needed.
- **Image Upload**:
  - If “Server Connection Failed,” verify server IP/port (`192.168.29.56:5000`), network, and server status.
  - Ensure server returns JSON with `"number_plate"` and `"view_image"`.
  - Test connectivity with `ESP32-Cam/extra/manual-click.ino`.
- **Camera Failure**:
  - Check connections, power (5V 1A+), or restart ESP32.
- **Web Server**:
  - Confirm IP address and same-network access.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request with improvements, bug fixes, or additional features.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact
- **Name**: [Rakshit Gupta](https://github.com/Rakshitgupta9/Celebal-Internship)
- **Email**: guptarakshit9858@gmail.com
- **LinkedIn**: [Rakshit Gupta](https://www.linkedin.com/in/rakshit9/)

