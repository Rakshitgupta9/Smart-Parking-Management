#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi credentials and server information
const char* ssid = "Virus";           // WiFi SSID
const char* password = "bkg@renu";    // WiFi Password
String serverName = "192.168.29.56";  // Server IP
String serverPath = "/upload";        // Server endpoint
const int serverPort = 5000;          // HTTP port

WiFiClient client;  // Client for HTTP communication

// Camera GPIO pins
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// NTP setup
const char* ntpServer = "pool.ntp.org";
const long utcOffsetInSeconds = 19800; // IST offset (UTC + 5:30)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);
String currentTime = "";

// Web server on port 80
WebServer server(80);

// Variables for status and data
String recognizedPlate = "";   // Store recognized plate number
String imageLink = "";         // Store image link
String currentStatus = "Idle"; // System status

// Function to extract JSON string value by key
String extractJsonStringValue(const String& jsonString, const String& key) {
  int keyIndex = jsonString.indexOf(key);
  if (keyIndex == -1) return "";

  int startIndex = jsonString.indexOf(':', keyIndex) + 2;
  int endIndex = jsonString.indexOf('"', startIndex);

  if (startIndex == -1 || endIndex == -1) return "";
  return jsonString.substring(startIndex, endIndex);
}

// Function to handle root web page
void handleRoot() {
  String html = "<!DOCTYPE html><html lang='en'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32-CAM Image Capture</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; color: #333; }";
  html += ".container { max-width: 800px; margin: 0 auto; padding: 20px; box-sizing: border-box; }";
  html += "header { text-align: center; padding: 15px; background-color: #0e3d79; color: white; }";
  html += "h1, h2 { text-align: center; margin-bottom: 20px; }";
  html += "p { margin: 10px 0; }";
  html += "form { text-align: center; margin: 20px 0; }";
  html += "input[type='submit'] { background-color: #007bff; color: white; border: none; padding: 10px 20px; font-size: 16px; cursor: pointer; border-radius: 5px; }";
  html += "input[type='submit']:hover { background-color: #0056b3; }";
  html += "a { color: #007bff; text-decoration: none; }";
  html += "a:hover { text-decoration: underline; }";
  html += "img { max-width: 100%; height: auto; margin: 20px auto; display: block; }";
  html += "</style>";
  html += "</head><body>";
  html += "<header><h1>ESP32-CAM</h1></header>";
  html += "<div class='container'>";
  html += "<h1>Image Capture System</h1>";
  html += "<p><strong>Time:</strong> " + currentTime + "</p>";
  html += "<p><strong>Status:</strong> " + currentStatus + "</p>";
  html += "<p><strong>Last Recognized Plate:</strong> " + recognizedPlate + "</p>";
  html += "<p><strong>Last Captured Image:</strong> <a href=\"" + imageLink + "\" target=\"_blank\">View Image</a></p>";
  if (imageLink != "") {
    html += "<img src=\"" + imageLink + "\" alt=\"Captured Image\">";
  }
  html += "<form action=\"/trigger\" method=\"POST\">";
  html += "<input type=\"submit\" value=\"Capture Image\">";
  html += "</form>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// Function to handle image capture trigger
void handleTrigger() {
  currentStatus = "Capturing Image";
  server.handleClient();

  int status = sendPhoto();

  if (status == -1) currentStatus = "Image Capture Failed";
  else if (status == -2) currentStatus = "Server Connection Failed";
  else currentStatus = "Idle";

  server.sendHeader("Location", "/");
  server.send(303);
}

// Function to capture and send photo
int sendPhoto() {
  camera_fb_t* fb = NULL;

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    currentStatus = "Image Capture Failed";
    server.handleClient();
    return -1;
  }

  Serial.println("Connecting to server: " + serverName);
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");

    String filename = "capture.jpeg";
    String head = "--ESP32CAM\r\nContent-Disposition: form-data; name=\"image_file\"; filename=\"" + filename + "\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--ESP32CAM--\r\n";
    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=ESP32CAM");
    client.println();
    client.print(head);

    currentStatus = "Uploading Image";
    server.handleClient();

    uint8_t* fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n += 1024) {
      if (n + 1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      } else {
        size_t remainder = fbLen % 1024;
        client.write(fbBuf, remainder);
      }
    }

    client.print(tail);
    esp_camera_fb_return(fb);
    Serial.println("Image sent successfully");

    currentStatus = "Waiting for Server Response";
    server.handleClient();

    String response = "";
    long startTime = millis();
    while (client.connected() && millis() - startTime < 10000) {
      if (client.available()) {
        char c = client.read();
        response += c;
      }
    }

    recognizedPlate = extractJsonStringValue(response, "\"number_plate\"");
    imageLink = extractJsonStringValue(response, "\"view_image\"");

    currentStatus = "Response Received Successfully";
    server.handleClient();

    Serial.print("Response: ");
    Serial.println(response);
    client.stop();
    return 0;
  } else {
    Serial.println("Connection to server failed");
    esp_camera_fb_return(fb);
    return -2;
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.update();

  server.on("/", handleRoot);
  server.on("/trigger", HTTP_POST, handleTrigger);
  server.begin();
  Serial.println("Web server started");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 5;
    config.fb_count = 2;
    Serial.println("PSRAM found");
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Configure camera sensor for vflip and hmirror
  sensor_t * sensor = esp_camera_sensor_get();
  sensor->set_vflip(sensor, 1);    // Vertical flip
  sensor->set_hmirror(sensor, 1);  // Horizontal mirror
}

void loop() {
  timeClient.update();
  currentTime = timeClient.getFormattedTime();
  server.handleClient();
}
