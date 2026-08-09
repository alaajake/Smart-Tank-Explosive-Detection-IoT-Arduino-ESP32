#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "TOT_tank";
const char* password = "12345678";

// Camera pins (for AI-Thinker ESP32-CAM)
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

// Metal detection variables
volatile bool metalDetected = false;

WebServer server(80);

// Function prototypes
void handleRoot();
void handleStream();
void handleControl();
void handleMetalStatus();

void setup() {
  Serial.begin(115200);
  
  // Initialize camera
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

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Flip configuration
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);  // Vertical flip
  s->set_hmirror(s, 0); // Horizontal mirror

  // Start WiFi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Start web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/stream", HTTP_GET, handleStream);
  server.on("/control", HTTP_GET, handleControl);
  server.on("/metal-status", HTTP_GET, handleMetalStatus);
  
  server.begin();
}

void loop() {
  server.handleClient();
  
  // Check for serial input
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    metalDetected = (input == "metal detected");
  }
}

void handleMetalStatus() {
  server.send(200, "text/plain", metalDetected ? "detected" : "clear");
}

// Web page handler
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>TOT-TANK-CAM Live</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { 
        background: #1a1a1a; 
        color: white; 
        font-family: Arial;
        margin: 0;
        padding: 20px;
      }
      .container {
        max-width: 800px;
        margin: 0 auto;
      }
      .video-frame {
        background: #333;
        border-radius: 10px;
        padding: 10px;
        margin-bottom: 20px;
      }
      #stream {
        width: 100%;
        border-radius: 5px;
      }
      .controls {
        display: flex;
        gap: 20px;
        background: #333;
        padding: 20px;
        border-radius: 10px;
      }
      .joystick {
        background: #444;
        width: 200px;
        height: 200px;
        border-radius: 50%;
        position: relative;
      }
      .joystick-inner {
        width: 50px;
        height: 50px;
        background: #666;
        border-radius: 50%;
        position: absolute;
        top: 75px;
        left: 75px;
      }
      .buttons {
        display: flex;
        flex-direction: column;
        gap: 10px;
      }
      button {
        padding: 15px 30px;
        border: none;
        border-radius: 5px;
        background: #4CAF50;
        color: white;
        cursor: pointer;
      }
      .status {
        margin-top: 20px;
        display: flex;
        gap: 10px;
        align-items: center;
        font-size: 1.2em;
      }
      .indicator {
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: #00ff00;
        transition: background-color 0.3s ease;
      }
      #metalText {
        margin-left: 10px;
        color: #00ff00;
        transition: color 0.3s ease;
      }
      .metal-detected {
        background-color: #ff0000 !important;
        color: #ff0000 !important;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>TOT-TANK-CAM Live</h1>
      
      <div class="video-frame">
        <img id="stream" src="/stream">
      </div>

      <div class="controls">
        <div class="joystick" id="joystick">
          <div class="joystick-inner" id="joystickInner"></div>
        </div>
        
        <div class="buttons">
          <button id="modeBtn">Manual</button>
          <button id="autoBtn">Auto</button>
          <button id="turn360">360 Turn</button>
        </div>
      </div>

      <div class="status">
        Metal Status: 
        <span id="metalText">No Metal Detected</span>
        <div class="indicator" id="metalIndicator"></div>
      </div>
    </div>

    <script>
      // Video stream handling
      const streamImg = document.getElementById('stream');
      setInterval(() => {
        streamImg.src = '/stream?' + Date.now();
      }, 100);

      // Joystick control
      const joystick = document.getElementById('joystick');
      const joystickInner = document.getElementById('joystickInner');
      let isDragging = false;

      joystick.addEventListener('mousedown', startDrag);
      joystick.addEventListener('touchstart', startDrag);
      document.addEventListener('mouseup', stopDrag);
      document.addEventListener('touchend', stopDrag);

      function startDrag(e) {
        isDragging = true;
        updatePosition(e);
      }

      function stopDrag() {
        isDragging = false;
        joystickInner.style.top = '75px';
        joystickInner.style.left = '75px';
      }

      function updatePosition(e) {
        if (!isDragging) return;
        
        const rect = joystick.getBoundingClientRect();
        let x = (e.touches ? e.touches[0].clientX : e.clientX) - rect.left;
        let y = (e.touches ? e.touches[0].clientY : e.clientY) - rect.top;

        x = Math.max(0, Math.min(x, 200));
        y = Math.max(0, Math.min(y, 200));
        
        joystickInner.style.left = (x - 25) + 'px';
        joystickInner.style.top = (y - 25) + 'px';

        const dirX = x - 100;
        const dirY = y - 100;
        fetch(`/control?x=${dirX}&y=${dirY}`);
      }

      // Button controls
      document.getElementById('modeBtn').addEventListener('click', () => {
        fetch('/control?mode=manual');
      });

      document.getElementById('autoBtn').addEventListener('click', () => {
        fetch('/control?mode=auto');
      });

      document.getElementById('turn360').addEventListener('click', () => {
        fetch('/control?turn=360');
      });

      // Metal detection update
      function updateMetalStatus() {
        fetch('/metal-status')
          .then(response => response.text())
          .then(status => {
            const indicator = document.getElementById('metalIndicator');
            const textElement = document.getElementById('metalText');
            
            if (status === 'detected') {
              indicator.classList.add('metal-detected');
              textElement.classList.add('metal-detected');
              textElement.textContent = 'Metal Detected!';
            } else {
              indicator.classList.remove('metal-detected');
              textElement.classList.remove('metal-detected');
              textElement.textContent = 'No Metal Detected';
            }
          });
      }

      // Update every second
      setInterval(updateMetalStatus, 1000);
      updateMetalStatus(); // Initial check
    </script>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Video stream handler
void handleStream() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  server.sendContent("HTTP/1.1 200 OK\r\n");
  server.sendContent("Content-Type: image/jpeg\r\n");
  server.sendContent("Connection: close\r\n\r\n");
  server.sendContent_P((const char *)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

// Control command handler
void handleControl() {
  String message = "Control Received: ";
  bool hasInput = false;

  if (server.hasArg("x") && server.hasArg("y")) {
    message += "Joystick X: " + server.arg("x") + " Y: " + server.arg("y");
    hasInput = true;
  }

  if (server.hasArg("mode")) {
    message += (hasInput ? " | " : "") + String("Mode: ") + server.arg("mode");
    hasInput = true;
  }

  if (server.hasArg("turn")) {
    message += (hasInput ? " | " : "") + String("Turn: ") + server.arg("turn");
    hasInput = true;
  }

  if(hasInput) {
    // Send formatted commands to Arduino
    if(server.hasArg("mode")) {
      String modeCmd = "MODE:" + server.arg("mode");
      Serial.println(modeCmd);
    }
    if(server.hasArg("turn") && server.arg("turn") == "360") {
      Serial.println("TURN360");
    }
    if(server.hasArg("x") && server.hasArg("y")) {
      String joyCmd = "JOYSTICK:" + server.arg("x") + "," + server.arg("y");
      Serial.println(joyCmd);
    }
  }
}