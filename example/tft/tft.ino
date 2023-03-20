#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <Button2.h>


// #define LED_PIN     1
// #define BUTTON_PIN  2

#define SWITCH      1
#define BUTTON      10
Button2 btn = Button2(SWITCH);
Button2 btn2 = Button2(BUTTON);

#define DT          2
#define CLK         3

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 170

#define FONT_SIZE 3

#if false
#define debug(message) Serial.println(message);
#else
#define debug(message)
#endif
TFT_eSPI tft;
RotaryEncoder encoder(DT, CLK, RotaryEncoder::LatchMode::TWO03);

int buttonState;            // current state of the button
int lastButtonState = LOW;  // previous state of the button
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200); // 初始化Serial
  tft.init();
  scanWiFi();

  pinMode(SWITCH, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  for(int i=0 ; i<100 ; i++) {
    // tft.fillScreen(TFT_WHITE);
    // delay(100);
    // tft.fillScreen(TFT_BLACK);
    // delay(100);
    switchOption();
    delay(10);
  }
  btn.setPressedHandler(sendRequest);
  btn2.setPressedHandler(pressedHandler);



  // connectToWiFi();
}

bool status = true;
int count = 0;
int fontSize = 0;

void showOptions(String options[]) {
  debug(tft.fontHeight());
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(0, 0);
  debug("FontSize: " + String(fontSize) + "\tFontHeight: " + tft.fontHeight());

  for (int i = 0; i < options->length(); i++) {
    tft.println(options[i]);
    int y = i == 0 ? 3 : 3 + i * 36;
    debug("y: " + String(y));
    tft.drawString(options[i], 0, y);
  }
}

String options[] = {
  "aaaaaa",
  "bbbbbb",
  "cccccc",
  "dddddd",
};

int yCollect[] = {
  3,
  39,
  75,
  111,
};

int target = 0;

void showFixedOptions() {
  debug(tft.fontHeight());
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(4);
  tft.setCursor(0, 0);
  debug("FontSize: " + String(fontSize) + "\tFontHeight: " + tft.fontHeight());

  tft.setTextColor(
    target == 0 ? TFT_BLACK : TFT_WHITE,
    target == 0 ? TFT_WHITE : TFT_BLACK
  );
  tft.drawString(String(options[0]) + String(yCollect[0]), 0, yCollect[0]);

  tft.setTextColor(
    target == 1 ? TFT_BLACK : TFT_WHITE,
    target == 1 ? TFT_WHITE : TFT_BLACK
  );
  tft.drawString(String(options[1]) + String(yCollect[1]), 0, yCollect[1]);

  tft.setTextColor(
    target == 2 ? TFT_BLACK : TFT_WHITE,
    target == 2 ? TFT_WHITE : TFT_BLACK
  );
  tft.drawString(String(options[2]) + String(yCollect[2]), 0, yCollect[2]);

  tft.setTextColor(
    target == 3 ? TFT_BLACK : TFT_WHITE,
    target == 3 ? TFT_WHITE : TFT_BLACK
  );
  tft.drawString(String(options[3]) + String(yCollect[3]), 0, yCollect[3]);
}

void showMessage(String message) {
  tft.fillScreen(TFT_BLACK); // 填滿黑色背景
  tft.setTextColor(TFT_WHITE); // 文字白色
  tft.setTextSize(1); // 文字大小
  tft.setCursor(20, 50); // 設定文字起始位置
  tft.drawString(message, 0, 0, 2);
}

String wifiStatus(wl_status_t status) {
  String statusStr = "None";
  switch(status) {
    case(255):
      statusStr = "WL_NO_SHIELD";
      break;
    case(0):
      statusStr = "WL_IDLE_STATUS";
      break;
    case(1):
      statusStr = "WL_NO_SSID_AVAIL";
      break;
    case(2):
      statusStr = "WL_SCAN_COMPLETED";
      break;
    case(3):
      statusStr = "WL_CONNECTED";
      break;
    case(4):
      statusStr = "WL_CONNECT_FAILED";
      break;
    case(5):
      statusStr = "WL_CONNECTION_LOST";
      break;
    case(6):
      statusStr = "WL_DISCONNECTED";
      break;
  }

  return statusStr;
}

bool connectToWiFi() {
  WiFi.disconnect(true); //關閉網絡
  WiFi.begin("45Y077988", "0925027069");

  while(WiFi.status() != WL_CONNECTED) {
    String status = wifiStatus(WiFi.status());
    delay(1000);
    Serial.println("Connecting..." + status);
    showMessage("Connecting..." + status);
  }
  Serial.println("Connected to WiFi:" + wifiStatus(WiFi.status()));
  String ip = WiFi.localIP().toString().c_str();
  Serial.println("IP: " + ip);
  showMessage("IP: " + ip);

  return true;
}

void handleRequest() {
  AsyncClient *client = new AsyncClient;
  client->onError([](void *arg, AsyncClient *client, int error) {
    Serial.println("Error connecting to server");
    client->close();
  }, NULL);

  client->onConnect([](void *arg, AsyncClient *client) {
    Serial.println("Connected to server");
    DynamicJsonDocument doc(1024);
    doc["action"] = "from esp32";

    String jsonString;
    serializeJson(doc, jsonString);
    String postRequest =
      String("POST /api/postmarks HTTP/1.1\r\n") +
      String("Host: 192.168.1.110:8199\r\n") +
      String("Content-Type: application/json\r\n") +
      String("Content-Length: ") + String(jsonString.length()) + String("\r\n") +
      String("Connection: close\r\n\r\n") + jsonString;

    client->write(postRequest.c_str(), postRequest.length());

  }, NULL);

  client->onData([](void *arg, AsyncClient *client, void *data, size_t len) {
    String message = String((char*)data);
    Serial.println(message);
    showMessage("Success at ");
  }, NULL);

  client->onDisconnect([](void *arg, AsyncClient *client) {
    Serial.println("Disconnected from server");
    delete client;
  }, NULL);

  client->connect("192.168.1.110", 8199);
}

void scanWiFi() {
  WiFi.mode(WIFI_STA);

  Serial.println("Scanning available WiFi networks...");
  int networkCount = WiFi.scanNetworks();
  Serial.println("Network count: " + String(networkCount));

  if (networkCount) {
    Serial.printf("%d available WiFi networks found:\n", networkCount);
    for (int i = 0; i < networkCount; i++) {
      Serial.printf(
        "MAC:%s\t%d: %s (%ddBm)\n",
        WiFi.macAddress(),
        i+1,
        WiFi.SSID(i).c_str(),
        WiFi.RSSI(i)
      );
    }
  }
}

void whenPressTheButton(void (*callback)()) {
  int reading = digitalRead(SWITCH);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        int newPos = (int)(encoder.getDirection());
        Serial.println(
          "Button pressed!" +
          String(newPos) +
          ";"
        );
        callback();
      }
    }
  }
  lastButtonState = reading;
}

void whenSpinTheRotaryEncoder() {
  static int pos = 0;

  encoder.tick();
  int newPos = encoder.getPosition();
  if (pos != newPos) {
    // showMessage(String(newPos));
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    int dir = (int)(encoder.getDirection());
    Serial.println(dir);

    yCollect[target] = yCollect[target] + dir;
    showFixedOptions();

    pos = newPos;
  }
}

void sendRequest(Button2& btn) {

  fontSize++;
  String options[] = {"aaaa", "bbbbb", "ccccc", "ddddd"};
  showOptions(options);
  // if (WiFi.status() == WL_CONNECTED) {
  //   handleRequest();
  // } else {
  //   Serial.println("No connection!");
  // }
}


void pressedHandler(Button2& btn) {
  // fontSize--;
  // String options[] = {"aaaa", "bbbbb", "ccccc", "ddddd"};
  // showOptions(options);
  switchOption();
}

void switchOption() {
  target = target == 3 ? 0 : target + 1;
  showFixedOptions();
}

void loop() {
  btn.loop();
  btn2.loop();
  whenSpinTheRotaryEncoder();

}
