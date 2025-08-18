#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Lirik";
const char* password = "12345678";
ESP8266WebServer server(80);

String lirikList[100];
int delayList[100];
int textSizeList[100];
int lirikCount = 0;

const char hiddenText[] = {
  0x42, 0x79, 0x20, 0x40, 0x6B, 0x61, 0x69, 0x73,
  0x61, 0x72, 0x5F, 0x70, 0x65, 0x64, 0x6F, 0x00
};

void tampilkanTengah(String teks, int size) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  int charWidth = 6 * size;
  int textWidth = teks.length() * charWidth;
  int x = (SCREEN_WIDTH - textWidth) / 2;
  int y = (SCREEN_HEIGHT - (8 * size)) / 2;
  display.setCursor(x, y);
  display.print(teks);
  display.display();
}

String htmlForm() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'/>";
  html += "<title>Edit Lirik</title></head><body><h2>Input Lirik</h2>";
  html += "<form action='/submit' method='POST'>";
  html += "Lirik (pisahkan dengan baris baru):<br><textarea name='lirik' rows='10' cols='30'></textarea><br>";
  html += "Delay per baris (ms):<br><input name='delay' type='number' value='1000'><br>";
  html += "Ukuran Teks (1 atau 2):<br><input name='textsize' type='number' value='1'><br>";
  html += "<input type='submit' value='Tambah'>";
  html += "</form><hr><h3>Edit Lirik:</h3><form action='/update' method='POST'>";
  for (int i = 0; i < lirikCount; i++) {
    html += "<b>" + String(i + 1) + ".</b> ";
    html += "<input name='lirik" + String(i) + "' value='" + lirikList[i] + "'> ";
    html += "Delay:<input name='delay" + String(i) + "' type='number' value='" + delayList[i] + "'> ";
    html += "Size:<input name='size" + String(i) + "' type='number' value='" + textSizeList[i] + "'> ";
    html += "<a href='/delete?i=" + String(i) + "'>[hapus]</a><br>";
  }
  html += "<input type='submit' value='Update Semua'>";
  html += "</form><hr><form action='/clear' method='POST'><input type='submit' value='Hapus Semua Lirik'></form>";

  html += "<script>";
  html += "let hex = ['62','79','20','40','6B','61','69','73','61','72','5F','70','65','64','6F'];";
  html += "let teks = '';";
  html += "for(let i=0;i<hex.length;i++){ teks += String.fromCharCode(parseInt(hex[i],16)); }";
  html += "document.write('<p style=\"font-size:20px;color:#aaa;\">'+teks+'</p>');";
  html += "</script>";

  html += "</body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  server.on("/", []() {
    server.send(200, "text/html", htmlForm());
  });

  server.on("/submit", HTTP_POST, []() {
    String input = server.arg("lirik");
    int delayVal = server.arg("delay").toInt();
    int sizeVal = constrain(server.arg("textsize").toInt(), 1, 2);
    input.replace("\r", "");
    while (input.length() > 0 && lirikCount < 100) {
      int newline = input.indexOf('\n');
      String line = (newline != -1) ? input.substring(0, newline) : input;
      lirikList[lirikCount] = line;
      delayList[lirikCount] = delayVal;
      textSizeList[lirikCount] = sizeVal;
      lirikCount++;
      if (newline == -1) break;
      input = input.substring(newline + 1);
    }
    server.send(200, "text/html", "<meta http-equiv='refresh' content='0; url=/' />");
  });

  server.on("/update", HTTP_POST, []() {
    for (int i = 0; i < lirikCount; i++) {
      lirikList[i] = server.arg("lirik" + String(i));
      delayList[i] = server.arg("delay" + String(i)).toInt();
      textSizeList[i] = constrain(server.arg("size" + String(i)).toInt(), 1, 2);
    }
    server.send(200, "text/html", "<meta http-equiv='refresh' content='0; url=/' />");
  });

  server.on("/clear", HTTP_POST, []() {
    lirikCount = 0;
    server.send(200, "text/html", "<meta http-equiv='refresh' content='0; url=/' />");
  });

  server.on("/delete", []() {
    int i = server.arg("i").toInt();
    if (i >= 0 && i < lirikCount) {
      for (int j = i; j < lirikCount - 1; j++) {
        lirikList[j] = lirikList[j + 1];
        delayList[j] = delayList[j + 1];
        textSizeList[j] = textSizeList[j + 1];
      }
      lirikCount--;
    }
    server.send(200, "text/html", "<meta http-equiv='refresh' content='0; url=/' />");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (lirikCount == 0) {
    tampilkanTengah(String(hiddenText), 1);
    delay(1000);
    return;
  }

  for (int i = 0; i < lirikCount; i++) {
    tampilkanTengah(lirikList[i], textSizeList[i]);
    delay(delayList[i]);
  }
}
