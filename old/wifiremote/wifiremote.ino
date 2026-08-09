#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_camera.h"
#include <HardwareSerial.h>

// WiFi credentials
const char* ssid = "TOT_tank";
const char* password = "12345678";

// Camera pins (AI-Thinker ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WebServer server(80);
HardwareSerial mySerial(1);

// Camera status and metal detection
bool cameraActive = true;
bool metalDetected = false;

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    ESP.restart();
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 0);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  setupCamera();

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>TOT-TANK-CAM</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { background: #1a1a1a; color: white; font-family: Arial; margin: 0; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        .video-frame { background: #333; border-radius: 10px; padding: 10px; margin-bottom: 20px; }
        #stream { width: 100%; border-radius: 5px; }
        .controls { display: flex; gap: 20px; background: #333; padding: 20px; border-radius: 10px; }
        button { padding: 15px 30px; border: none; border-radius: 5px; cursor: pointer; }
        .camera-btn { background: #4CAF50; }
        .camera-btn.active { background: #f44336; }
        .status { margin-top: 20px; display: flex; gap: 10px; align-items: center; }
        .indicator { width: 20px; height: 20px; border-radius: 50%; background: #666; }
        .active { background: #ff0000; }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>TOT-TANK-CAM</h1>
        
        <div class="video-frame">
          <img id="stream" src="">
        </div>

        <div class="controls">
          <button id="cameraBtn" class="camera-btn" onclick="toggleCamera()">Start Stream</button>
          <button id="modeBtn">Manual Mode</button>
        </div>

        <div class="status">
          Metal Detection: 
          <div class="indicator" id="metalIndicator"></div>
        </div>
      </div>

      <script>
        let streamActive = false;
        let updateInterval;
        const cameraBtn = document.getElementById('cameraBtn');
        
        function toggleCamera() {
          fetch('/toggle-camera')
            .then(response => response.text())
            .then(status => {
              streamActive = status === 'Camera ON';
              cameraBtn.textContent = streamActive ? 'Stop Stream' : 'Start Stream';
              cameraBtn.classList.toggle('active', streamActive);
              
              if(streamActive) {
                startStream();
              } else {
                stopStream();
              }
            });
        }

        function startStream() {
          document.getElementById('stream').src = '/stream?' + Date.now();
          updateInterval = setInterval(() => {
            if(streamActive) {
              document.getElementById('stream').src = '/stream?' + Date.now();
            }
          }, 100);
        }

        function stopStream() {
          clearInterval(updateInterval);
          document.getElementById('stream').src = '';
        }

        // Update metal detection status
        setInterval(() => {
          fetch('/status')
            .then(response => response.json())
            .then(data => {
              document.getElementById('metalIndicator').className = 
                data.metal ? 'indicator active' : 'indicator';
            });
        }, 1000);
      </script>
    </body>
    </html>
    )rawliteral";
    server.send(200, "text/html", html);
  });

  server.on("/stream", HTTP_GET, []() {
    if (!cameraActive) {
      server.send(200, "text/plain", "Camera inactive");
      return;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      server.send(500, "text/plain", "Camera capture failed");
      return;
    }

    server.sendHeader("Content-Type", "image/jpeg");
    server.sendHeader("Content-Length", String(fb->len));
    server.send(200, "image/jpeg", "");
    server.sendContent((const char *)fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
  });

  server.on("/toggle-camera", HTTP_GET, []() {
    cameraActive = !cameraActive;
    server.send(200, "text/plain", cameraActive ? "Camera ON" : "Camera OFF");
  });

  server.on("/status", HTTP_GET, []() {
    String json = String("{\"metal\":") + metalDetected + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient();
  
  if (mySerial.available()) {
    char status = mySerial.read();
    metalDetected = (status == '1');
  }

  // Add periodic system check
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    Serial.printf("Free Heap: %d\n", ESP.getFreeHeap());
  }
}