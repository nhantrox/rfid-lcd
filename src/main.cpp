// ============================================
// HE THONG KIEM SOAT RFID VOI GIAO DIEN WEB
// ESP32 + RC522 + WiFi Web Server
// ============================================
// File nay xu ly: WiFi, RFID, Web Server, LittleFS
// Giao dien web nam trong file: webpage.h
// ============================================

#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include "webpage.h"

// --- PHAN 1: CAU HINH PHAN CUNG ---
#define RST_PIN   4     // Da doi tu 22 sang 4 (vi GPIO 22 = SCL cho LCD I2C)
#define SS_PIN    5
MFRC522 mfrc522(SS_PIN, RST_PIN);

// --- LCD I2C ---
// Dia chi 0x27, LCD 16 cot x 2 hang
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- PHAN 2: CAU HINH WIFI ---
// *** HAY SUA THANH WIFI NHA BAN ***
const char* ssid     = "CauXanh2";
const char* password = "22222222";

// --- PHAN 3: BIEN TOAN CUC ---
WebServer server(80);
const char* userFile = "/users.json";

// Luu tru the vua quet gan nhat
String lastScannedUID = "";
bool   lastCardKnown  = false;
String lastCardUser   = "";

// --- QUAN LY TRANG THAI LCD ---
#define LCD_SHOW_DURATION 3000  // Hien thi ket qua 3 giay
bool   lcdShowingResult = false;
unsigned long lcdShowTime = 0;

// Hieu ung dau cham dong man hinh cho
#define DOT_INTERVAL 500  // Them 1 cham moi 500ms
unsigned long lastDotTime = 0;
byte dotCount = 0;

// Hieu ung scroll ten dai
#define SCROLL_INTERVAL 400  // Cuon 1 ky tu moi 400ms
String scrollText = "";
int    scrollPos  = 0;
unsigned long lastScrollTime = 0;
bool   isScrolling = false;

void lcdShowIdle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  RFID System   ");
  lcd.setCursor(0, 1);
  lcd.print("Quet the ");
  dotCount = 0;
  lastDotTime = millis();
}

void lcdUpdateDots() {
  if (lcdShowingResult) return;  // Dang hien ket qua, khong update
  if (millis() - lastDotTime < DOT_INTERVAL) return;
  lastDotTime = millis();

  dotCount = (dotCount % 4) + 1;  // 1 -> 2 -> 3 -> 4 -> 1 ...
  lcd.setCursor(9, 1);             // Vi tri sau "Quet the "
  for (byte i = 0; i < 4; i++) {
    lcd.print(i < dotCount ? "." : " ");
  }
}

void lcdShowResult(bool known, const String& name, const String& uid) {
  lcd.clear();
  if (known) {
    lcd.setCursor(0, 0);
    lcd.print("Xin chao!");
    if (name.length() <= 16) {
      // Ten ngan: hien thi binh thuong
      lcd.setCursor(0, 1);
      lcd.print(name);
      isScrolling = false;
    } else {
      // Ten dai: bat dau scroll
      scrollText = name + "   ";  // Them khoang cach de lap lai dep
      scrollPos  = 0;
      isScrolling = true;
      lastScrollTime = millis();
      lcd.setCursor(0, 1);
      lcd.print(scrollText.substring(0, 16));
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Chua dang ky!");
    lcd.setCursor(0, 1);
    lcd.print(uid.substring(0, 11));
    isScrolling = false;
  }
  lcdShowingResult = true;
  lcdShowTime = millis();
}

void lcdUpdateScroll() {
  if (!lcdShowingResult || !isScrolling) return;
  if (millis() - lastScrollTime < SCROLL_INTERVAL) return;
  lastScrollTime = millis();

  scrollPos++;
  if (scrollPos > (int)scrollText.length()) scrollPos = 0;

  // Lay 16 ky tu tu vi tri scrollPos (vong tron)
  String display = "";
  String looped  = scrollText + scrollText;  // Lap vong
  display = looped.substring(scrollPos, scrollPos + 16);

  lcd.setCursor(0, 1);
  lcd.print(display);
}

// ============================================
// PHAN 4: HAM XU LY DU LIEU NGUOI DUNG
// (Doc / Ghi file JSON tren Flash)
// ============================================

// Doc danh sach nguoi dung tu Flash
String readUsersFromFlash() {
  File file = LittleFS.open(userFile, "r");
  if (!file) return "[]";
  String content = file.readString();
  file.close();
  if (content.length() == 0) return "[]";
  return content;
}

// Ghi danh sach nguoi dung vao Flash
void writeUsersToFlash(const String& jsonData) {
  File file = LittleFS.open(userFile, "w");
  if (file) {
    file.print(jsonData);
    file.close();
  }
}

// Kiem tra UID co trong danh sach khong
// Tra ve ten nguoi dung neu tim thay, tra ve "" neu khong
String findUserByUID(const String& uid) {
  DynamicJsonDocument doc(4096);
  deserializeJson(doc, readUsersFromFlash());
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject user : arr) {
    if (user["uid"].as<String>() == uid) {
      return user["name"].as<String>();
    }
  }
  return "";
}

// ============================================
// PHAN 5: HAM XU LY RFID
// (Chuyen UID thanh chuoi, kiem tra danh sach)
// ============================================

void handleRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial())   return;

  // Chuyen 4 byte UID thanh chuoi dang "52 D6 4C 06"
  String uid = "";
  for (byte i = 0; i < 4; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
    if (i < 3) uid += " ";
  }
  uid.toUpperCase();

  // Luu lai UID vua quet
  lastScannedUID = uid;

  // Kiem tra trong danh sach
  String userName = findUserByUID(uid);
  if (userName.length() > 0) {
    lastCardKnown = true;
    lastCardUser  = userName;
    Serial.println(">>> KHOP MA! Xin chao: " + userName);
    lcdShowResult(true, userName, uid);
  } else {
    lastCardKnown = false;
    lastCardUser  = "";
    Serial.println("!!! THE MOI: " + uid);
    lcdShowResult(false, "", uid);
  }

  Serial.println("UID: " + uid);
  Serial.println("---------------------------");

  delay(500);
  mfrc522.PICC_HaltA();
}

// ============================================
// PHAN 6: CAC ROUTE CUA WEB SERVER
// (Trang chinh, API doc/them/xoa)
// ============================================

// Trang chinh - tra ve giao dien HTML
void handleRoot() {
  server.send_P(200, "text/html", WEBPAGE_HTML);
}

// API: Lay danh sach nguoi dung
void handleList() {
  server.send(200, "application/json", readUsersFromFlash());
}

// API: Lay thong tin the vua quet
void handleScan() {
  String json = "{";
  json += "\"uid\":\"" + lastScannedUID + "\",";
  json += "\"known\":" + String(lastCardKnown ? "true" : "false") + ",";
  json += "\"userName\":\"" + lastCardUser + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// API: Them nguoi dung moi
void handleAdd() {
  String name = server.arg("name");
  String uid  = server.arg("uid");

  if (name.length() == 0 || uid.length() == 0) {
    server.send(400, "text/plain", "Thieu ten hoac UID");
    return;
  }

  DynamicJsonDocument doc(4096);
  deserializeJson(doc, readUsersFromFlash());
  JsonObject newUser = doc.createNestedObject();
  newUser["name"] = name;
  newUser["uid"]  = uid;

  String output;
  serializeJson(doc, output);
  writeUsersToFlash(output);

  // Cap nhat trang thai the vua quet
  lastCardKnown = true;
  lastCardUser  = name;

  Serial.println("+ DA THEM: " + name + " [" + uid + "]");
  server.send(200, "text/plain", "OK");
}

// API: Xoa nguoi dung theo index
void handleDelete() {
  int id = server.arg("id").toInt();

  DynamicJsonDocument doc(4096);
  deserializeJson(doc, readUsersFromFlash());

  if (id >= 0 && id < (int)doc.size()) {
    String deletedName = doc[id]["name"].as<String>();
    doc.remove(id);
    String output;
    serializeJson(doc, output);
    writeUsersToFlash(output);
    Serial.println("- DA XOA: " + deletedName);
  }

  server.send(200, "text/plain", "OK");
}

// ============================================
// PHAN 7: SETUP - KHOI TAO HE THONG
// ============================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== KHOI DONG HE THONG RFID ===");

  // 7.1: Khoi tao RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // 7.1b: Khoi tao LCD I2C
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("RFID System");
  lcd.setCursor(0, 1);
  lcd.print("Dang khoi dong..");
  Serial.println("[OK] Module RFID RC522");

  // 7.2: Khoi tao bo nho Flash (LittleFS)
  if (!LittleFS.begin(true)) {
    Serial.println("[LOI] Khong khoi tao duoc LittleFS!");
    return;
  }
  Serial.println("[OK] Bo nho Flash (LittleFS)");

  // 7.3: Ket noi WiFi
  WiFi.begin(ssid, password);
  Serial.print("[...] Dang ket noi WiFi");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 40) {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi da ket noi!");
    Serial.println("     IP cua ban: http://" + WiFi.localIP().toString());
  } else {
    Serial.println("\n[LOI] Khong ket noi duoc WiFi!");
    Serial.println("      Hay kiem tra lai SSID va mat khau.");
    return;
  }

  // 7.4: Cai dat cac route cho Web Server
  server.on("/",       handleRoot);
  server.on("/list",   handleList);
  server.on("/scan",   handleScan);
  server.on("/add",    handleAdd);
  server.on("/delete", handleDelete);
  server.begin();
  Serial.println("[OK] Web Server da khoi dong!");

  Serial.println("=== HE THONG SAN SANG ===\n");

  lcdShowIdle();
}

// ============================================
// PHAN 8: LOOP - VONG LAP CHINH
// ============================================

void loop() {
  server.handleClient();  // Xu ly yeu cau tu web
  handleRFID();           // Kiem tra the RFID

  // Tu dong quay lai man hinh cho sau LCD_SHOW_DURATION ms
  if (lcdShowingResult && (millis() - lcdShowTime >= LCD_SHOW_DURATION)) {
    lcdShowingResult = false;
    lcdShowIdle();
  }

  // Cap nhat hieu ung dau cham
  lcdUpdateDots();

  // Cap nhat hieu ung scroll ten
  lcdUpdateScroll();
}
