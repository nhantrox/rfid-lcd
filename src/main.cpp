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
#include "webpage.h"

// --- PHAN 1: CAU HINH PHAN CUNG ---
#define RST_PIN   22
#define SS_PIN    5
MFRC522 mfrc522(SS_PIN, RST_PIN);

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
  } else {
    lastCardKnown = false;
    lastCardUser  = "";
    Serial.println("!!! THE MOI: " + uid);
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
}

// ============================================
// PHAN 8: LOOP - VONG LAP CHINH
// ============================================

void loop() {
  server.handleClient();  // Xu ly yeu cau tu web
  handleRFID();           // Kiem tra the RFID
}
