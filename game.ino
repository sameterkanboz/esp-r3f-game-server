#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Access Point SSID ve Şifresi
const char *ssid = "ESP32-Access-Point";
const char *password = "12345678";

// Web sunucusu portu
WebServer server(80);

// I2C adresi (tipik olarak 0x27 veya 0x3F olabilir)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Oyun karakteri için özel karakter tanımı
byte customChar[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b11111,
  0b00100,
  0b00100,
  0b01010,
  0b10001
};

// Karakterin başlangıç pozisyonu
int charPos = 7;

// Düşmanların pozisyonları
int enemyPosLeft = 0;
int enemyPosRight = 15;
int enemyRowLeft = 0;
int enemyRowRight = 1;

// Zamanlayıcı
unsigned long previousMillis = 0;
const long interval = 2000; // 2 saniye

bool gameOver = false;

void setup() {
  Serial.begin(115200);

  // LCD başlatma
  lcd.init();
  lcd.backlight();

  // Oyun karakteri için özel karakteri yükleme
  lcd.createChar(0, customChar);

  // WiFi Access Point başlatma
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Başlatıldı");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.softAPIP());

  // Web sunucusu rotası
  server.on("/", handleRoot);
  server.on("/move", HTTP_POST, handleMove);

  // Web sunucusunu başlatma
  server.begin();
  Serial.println("Web Sunucusu Başlatıldı");

  // Başlangıçta karakteri gösterme
  updateDisplay();
}

void loop() {
  server.handleClient();
  if (!gameOver) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      moveEnemies();
      checkCollision();
    }
  }
}

void handleRoot() {
  server.send(200, "text/plain", "Merhaba! Karakteri hareket ettirmek için /move endpointine POST isteği yapın ('r', 'l', veya 'w' parametreleri ile).");
}

void handleMove() {
  if (server.hasArg("direction") == false) {
    server.send(400, "text/plain", "Yanlış İstek");
    return;
  }

  String direction = server.arg("direction");

  if (direction == "r" && !gameOver) {
    moveRight();
  } else if (direction == "l" && !gameOver) {
    moveLeft();
  } else if (direction == "w" && !gameOver) {
    moveUpAndDown();
  } else {
    server.send(400, "text/plain", "Geçersiz Yön");
    return;
  }

  server.send(200, "text/plain", "Karakter hareket etti: " + direction);
}

void updateDisplay() {
  lcd.clear();

  // Karakteri yazdır
  lcd.setCursor(charPos, 1);
  lcd.write(byte(0));

  // Düşmanları yazdır
  lcd.setCursor(enemyPosLeft, enemyRowLeft);
  lcd.write('*');
  lcd.setCursor(enemyPosRight, enemyRowRight);
  lcd.write('*');
}

void moveRight() {
  if (charPos < 15) { // 16x2 LCD için sınır
    charPos++;
    updateDisplay();
  }
}

void moveLeft() {
  if (charPos > 0) {
    charPos--;
    updateDisplay();
  }
}

void moveUpAndDown() {
  // İkinci satırı temizle
  lcd.setCursor(charPos, 1);
  lcd.write(' ');

  // Karakteri birinci satıra yaz
  lcd.setCursor(charPos, 0);
  lcd.write(byte(0));
  delay(500);

  // Birinci satırı temizle
  lcd.setCursor(charPos, 0);
  lcd.write(' ');

  // Karakteri tekrar ikinci satıra yaz
  lcd.setCursor(charPos, 1);
  lcd.write(byte(0));
  delay(500);
}

void moveEnemies() {
  // Düşmanları sola ve sağa doğru hareket ettir
  if (enemyPosLeft < 15) {
    enemyPosLeft++;
  } else {
    enemyPosLeft = 0;
    enemyRowLeft = !enemyRowLeft; // Satırı değiştir
  }

  if (enemyPosRight > 0) {
    enemyPosRight--;
  } else {
    enemyPosRight = 15;
    enemyRowRight = !enemyRowRight; // Satırı değiştir
  }

  updateDisplay();
}

void checkCollision() {
  // Karakter düşmana değdi mi kontrol et
  if ((charPos == enemyPosLeft && 1 == enemyRowLeft) || (charPos == enemyPosRight && 1 == enemyRowRight)) {
    gameOver = true;
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Game Over");
  }
}
