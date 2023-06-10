/*

Copyright 2023 Bugra Ozden <bugra.ozden@gmail.com>
- https://github.com/bug7a

Licensed under the Apache License, Version 2.0

*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <IRremote.h>
#include <Adafruit_NeoPixel.h>

const int co2_sensor_pin = A0; // Hava kalitesini ölçmek için
const int isik_sensor_pin = A1; // Işık seviyesini ölçmek için

const int remote_alici_pin = 4; // Uzaktan kumanda alıcısı
const int dht_pin = 7; // Nem ve ısı sensörü
const int dht_type = DHT11;
const int nem_role_pin = 8; // Nemlendirme cihazı aç/kapat
const int led_pin = 9; // Aydınlatma
const int fan1_pin = 10; // CO2 çıkışı
const int fan2_pin = 11; // Oksijen girişi

int fan_status_setting = 2; // 0: kapalı, 1: açık, 2: otomatik
int nem_status_setting = 2;
int led_status_setting = 2;
int led_renk_setting = 3;

int last_color_1 = 0;
int last_color_2 = 0;
int last_color_3 = 0;

int fan_status = 0;
unsigned long fan_status_time = 0;
int nem_status = 0;
unsigned long nem_status_time = 0;
int nem_time_to_run = 0;
int led_status = 0;
unsigned long led_status_time = 0;
unsigned long button_clicked_time = 0;


int nem = 0; // Kutu içindeki nem
int sicaklik = 0; // Sıcaklık
int co2 = 0; // Hava kalitesi
int isik = 0; // Işık seviyesi

// Sabitler KULLANILMIYOR:
const int nem_limit_min = 70; // %60 dan az olursa; mantar gelişimi yavaşlar.
const int nem_limit_max = 90; // >95 iyi değil; hastalık riski.
const int sicaklik_limit_min = 19; // 20-24°C en ideal sıcaklık
const int sicaklik_limit_max = 25; // 16-28 arası son sınırlar dayanabilir.
const int co2_limit_max = 800; // 1200ppm e kadar dayanabilir.

int page = 1;
int page_max = 5;

// LCD ekran
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 0x3F
// Isı ve sıcaklık sensörü
DHT dht(dht_pin, dht_type);
// IR alıcı
IRrecv IR(remote_alici_pin);
// Aydınlarma
Adafruit_NeoPixel pixels(8, 9, NEO_GRB + NEO_KHZ800);

void setup() {

  // I2C iletişimini başlat
  Wire.begin();
  Serial.begin(9600);

  pinMode(nem_role_pin, OUTPUT);
  pinMode(fan1_pin, OUTPUT);
  pinMode(fan2_pin, OUTPUT);

  // Elektrik geldiğinde, röleyi kapalı duruma getir.
  // Röle için
  // digitalWrite(nem_role_pin, HIGH);
  // Kablo için
  digitalWrite(nem_role_pin, LOW);

  // LCD ekranı başlat
  lcd.init(); 
  lcd.backlight();
  button_clicked_time = millis();

  dht.begin();
  IR.enableIRIn();

  pixels.begin();  // Neopixel şeridi başlat
  pixels.show();   // LED'leri sıfırla (kapat)
  //pixels.clear(); // Set all pixel colors to 'off'

  Serial.println("-Start-");
  
}

String getStatusSettingName(int statusSetting) {

  if (statusSetting == 0) {
    return "KAPALI";
  } else if (statusSetting == 1) {
    return "ACIK";
  } else if (statusSetting == 2) {
    return "OTOMATIK";
  } else {
    return "TANIMSIZ";
  }

}

String getLedRenkName(int nameId) {

  if (nameId == 0) {
    return "KIRMIZI";
  } else if (nameId == 1) {
    return "MAVI";
  } else if (nameId == 2) {
    return "YESIL";
  } else if (nameId == 3) {
    return "AZ ISIK";
  } else if (nameId == 4) {
    return "SARI";
  } else if (nameId == 5) {
    return "PEMBE";
  } else if (nameId == 6) {
    return "CAM GOBEGI";
  } else if (nameId == 7) {
    return "BEYAZ";
  } else {
    return "TANIMSIZ";
  }

}

int setStatusSettingName(int statusSetting, int value) {
  int statusSettingResult = statusSetting + value;

  if (statusSettingResult < 0) {
    statusSettingResult = 2;
  }

  if (statusSettingResult > 2) {
    statusSettingResult = 0;
  }

  return statusSettingResult;
}

int setLedRenkName(int statusSetting, int value) {
  int statusSettingResult = statusSetting + value;

  if (statusSettingResult < 0) {
    statusSettingResult = 7;
  }

  if (statusSettingResult > 7) {
    statusSettingResult = 0;
  }

  return statusSettingResult;
}

String createLCDValue(String value, int maxLength) {
  String resultValue = value;

  if (resultValue.length() > maxLength) {
    resultValue = resultValue.substring(0, maxLength);
  } else if (resultValue.length() < maxLength) {
    int spaceCount = maxLength - resultValue.length();
    for (int i = 0; i < spaceCount; i++) {
      resultValue += " ";
    }
  }

  return resultValue;
}

void print4ToLCD(String val1 = "", String val2 = "", String val3 = "", String val4 = "") {
    lcd.setCursor(0, 0);
    lcd.print(createLCDValue(val1, 8));
    lcd.setCursor(8, 0);
    lcd.print(createLCDValue(val2, 8));
    lcd.setCursor(0, 1);
    lcd.print(createLCDValue(val3, 8));
    lcd.setCursor(8, 1);
    lcd.print(createLCDValue(val4, 8));
}

void print2ToLCD(String val1 = "", String val2 = "") {
    lcd.setCursor(0, 0);
    lcd.print(createLCDValue(val1, 16));
    lcd.setCursor(0, 1);
    lcd.print(createLCDValue(val2, 16));
}

void printCharToLCD(String char1 = " ") {
    lcd.setCursor(15, 0);
    lcd.print(char1);
}

String getTimeString(unsigned long int baslangicZamani) {
  unsigned long int simdikiZamani = millis();
  unsigned long int gecenSure = 0;

  if (simdikiZamani >= baslangicZamani) {
    gecenSure = (simdikiZamani - baslangicZamani) / 1000; // Geçen süreyi saniyeye çevir
  } else {
    // millis() değeri overflow olduğunda (49.7 gün sonra) düzgün hesaplama yapmak için
    // 2^32 (4,294,967,296) değerini ekleyerek hesaplama yapılır.
    gecenSure = (2UL * 4294967UL + simdikiZamani - baslangicZamani) / 1000; // Geçen süreyi saniyeye çevir
  }

  unsigned int saat = gecenSure / 3600;
  unsigned int dakika = (gecenSure % 3600) / 60;
  unsigned int saniye = (gecenSure % 3600) % 60;

  String sonuc = "";
  if (saat > 0) {
    sonuc = String(saat) + "sa " + String(dakika) + "dk " + String(saniye) + "sn";
  } else if (dakika > 0) {
    sonuc = String(dakika) + "dk " + String(saniye) + "sn";
  } else {
    sonuc = String(saniye) + "sn";
  }
  return sonuc;
}

unsigned long getTimeAsMinutes(unsigned long baslangicZamani) {
  unsigned long simdikiZaman = millis();
  unsigned long zamanFarkiMillis;
  
  if (simdikiZaman >= baslangicZamani) {
    zamanFarkiMillis = simdikiZaman - baslangicZamani;
  } else {
    // millis() değeri overflow olduğunda (49.7 gün sonra) düzgün hesaplama yapmak için
    // 2^32 (4,294,967,296) değerini ekleyerek hesaplama yapılır.
    zamanFarkiMillis = (2UL * 4294967296UL - baslangicZamani + simdikiZaman);
  }

  unsigned long zamanFarkiDakika = zamanFarkiMillis / 60000; // 1 dakika = 60000 milisaniye

  return zamanFarkiDakika;
}

unsigned long getTimeAsSeconds(unsigned long baslangicZamani) {
  unsigned long simdikiZaman = millis();
  unsigned long zamanFarkiMillis;
  
  if (simdikiZaman >= baslangicZamani) {
    zamanFarkiMillis = simdikiZaman - baslangicZamani;
  } else {
    // millis() değeri overflow olduğunda (49.7 gün sonra) düzgün hesaplama yapmak için
    // 2^32 (4,294,967,296) değerini ekleyerek hesaplama yapılır.
    zamanFarkiMillis = (2UL * 4294967296UL - baslangicZamani + simdikiZaman);
  }

  unsigned long zamanFarkiDakika = zamanFarkiMillis / 1000; // 1 dakika = 60000 milisaniye

  return zamanFarkiDakika;
}

void openLed(int color1, int color2, int color3) {

  if (last_color_1 == color1 && last_color_2 == color2 && last_color_3 == color3) {
    // led zaten açık ve bu renkte; hiçbir şey yapma.
  } else {
    pixels.clear(); // Set all pixel colors to 'off'

    last_color_1 = color1;
    last_color_2 = color2;
    last_color_3 = color3;
  
    for(int i=0; i<8; i++) { // For each pixel...
      pixels.setPixelColor(i, pixels.Color(color1, color2, color3));
      pixels.show();
    }
  }
}

void openLedByDefault() {
    if (led_renk_setting == 0) {
      openLed(255, 0, 0); // kırmızı
      //openLed(237, 109, 82);
    } else if (led_renk_setting == 1) {
      openLed(0, 0, 255); // mavi
      //openLed(104, 155, 210);
    } else if (led_renk_setting == 2) {
      openLed(0, 255, 0); // yeşil
      //openLed(90, 187, 159);
    } else if (led_renk_setting == 3) {
      //openLed(255, 255, 255); // beyaz
      //openLed(245, 191, 78); // sarı
      openLed(5, 5, 5);
      //openLed(204, 117, 170); // mor
    } else if (led_renk_setting == 4) {
      openLed(255, 255, 0); // sarı
    } else if (led_renk_setting == 5) {
      openLed(255, 0, 255); // pempe
    } else if (led_renk_setting == 6) {
      openLed(0, 255, 255); // çiyan
    } else if (led_renk_setting == 7) {
      openLed(255, 255, 255);
    }
}

void run_led() {
  if (led_status == 0) {
    openLedByDefault();
    led_status = 1;
  }
}

void stop_led() {
  if (led_status == 1) {
    pixels.clear();
    pixels.show();
    last_color_1 = 0;
    last_color_2 = 0;
    last_color_3 = 0;
    led_status = 0;
  }
}

void run_fan() {
  if (fan_status == 0) {
    digitalWrite(fan1_pin, HIGH); // co2 çıkış
    digitalWrite(fan2_pin, HIGH); // oksijen giriş
    fan_status = 1;
  }
}

void stop_fan() {
  if (fan_status == 1) {
    digitalWrite(fan1_pin, LOW);
    digitalWrite(fan2_pin, LOW);
    fan_status = 0;
  }
}

void run_nem() {
  if (nem_status == 0) {
    // Açmak için kısa tıklama yap.
    digitalWrite(nem_role_pin, HIGH);
    delay(200);
    digitalWrite(nem_role_pin, LOW);
    nem_status = 1;
  }
}

void stop_nem() {
  if (nem_status == 1) {
    // Kapatmak için iki kere kısa tıklama yap.
    digitalWrite(nem_role_pin, HIGH);
    delay(200);
    digitalWrite(nem_role_pin, LOW);
    delay(200);
    digitalWrite(nem_role_pin, HIGH);
    delay(200);
    digitalWrite(nem_role_pin, LOW);
    nem_status = 0;

    // Özel nem dağıtmayı da kapat.
    if (fan_status_setting == 2) {
      digitalWrite(fan1_pin, LOW);
      digitalWrite(fan2_pin, LOW);
      fan_status = 0;
    }
    
  }
}

void refresh_led_light() {
    // Eğer renk değiştirildiğinde; led çalışıyor ise manuel güncelle.
    // Çünkü, otomatikte olabilir ve kapanıp/açılmayacak aralıkta kalmış olabilir.
    if (led_status == 1) {
      openLedByDefault();
    }
}

void loop() {

  if (IR.decode()) {
    // Alınan IR kodunu seri monitöre yazdır
    Serial.println(IR.decodedIRData.decodedRawData, HEX);

    // LCD ekran ışığını aç ve açılma zamanını kaydet.
    lcd.backlight();
    button_clicked_time = millis();
    // lcd.noBacklight();
    
    if (IR.decodedIRData.decodedRawData == 0xAD52FF00) {
      // Aşağı ok
      page++;
      if (page > page_max) {
        page = 1;
      }
      
    } else if (IR.decodedIRData.decodedRawData == 0xE718FF00) { 
      // Yukarı ok
      page--;
      if (page < 1) {
        page = page_max;
      }

    } else if (IR.decodedIRData.decodedRawData == 0xA55AFF00) { 
      // Sağ ok
      if (page == 2) {
        fan_status_setting = setStatusSettingName(fan_status_setting, 1);
        fan_status_time = millis();
      } else if (page == 3) {
        led_status_setting = setStatusSettingName(led_status_setting, 1);
        led_status_time = millis();
      } else if (page == 4) {
        nem_status_setting = setStatusSettingName(nem_status_setting, 1);
        nem_status_time = millis();
      } else if (page == 5) {
        led_renk_setting = setLedRenkName(led_renk_setting, 1);
        refresh_led_light();

      }

    } else if (IR.decodedIRData.decodedRawData == 0xF708FF00) { 
      // Sol ok
      if (page == 2) {
        fan_status_setting = setStatusSettingName(fan_status_setting, -1);
        fan_status_time = millis();
      } else if (page == 3) {
        led_status_setting = setStatusSettingName(led_status_setting, -1);
        led_status_time = millis();
      } else if (page == 4) {
        nem_status_setting = setStatusSettingName(nem_status_setting, -1);
        nem_status_time = millis();
      } else if (page == 5) {
        led_renk_setting = setLedRenkName(led_renk_setting, -1);
        refresh_led_light();

      }

    } else if (IR.decodedIRData.decodedRawData == 0xE31CFF00) { 
      // Onaylama

    } else if (IR.decodedIRData.decodedRawData == 0xBA45FF00) { 
      // 1
      page = 1;

    } else if (IR.decodedIRData.decodedRawData == 0xB946FF00) { 
      // 2
      page = 2;

    } else if (IR.decodedIRData.decodedRawData == 0xB847FF00) { 
      // 3
      page = 3;

    } else if (IR.decodedIRData.decodedRawData == 0xBB44FF00) { 
      // 4
      page = 4;

    } else if (IR.decodedIRData.decodedRawData == 0xBF40FF00) {
      // 5
      page = 5;

    }

    IR.resume(); // Sonraki sinyali beklemek için alıcıyı yeniden etkinleştir
  }


  // SENSORLER:

  co2 = analogRead(co2_sensor_pin);
  isik = analogRead(isik_sensor_pin);
  sicaklik = int(dht.readTemperature()); // Sıcaklık değerini oku
  nem = int(dht.readHumidity()); // Nem değerini oku
  //Serial.println(String(nem));


  // LCD EKRANLARI:

  if (page == 1) {

    String nem_string = "%" + String(nem) + "nem";
    String co2_string = String(co2) + "co2";
    String sicaklik_string = String(sicaklik) + "C";
    String isik_string = String(isik);

    print4ToLCD(nem_string, co2_string, sicaklik_string, isik_string);

  } else if (page == 2) {

    String line1_string = "Fan: " + getStatusSettingName(fan_status_setting);
    String line2_string = getTimeString(fan_status_time);

    print2ToLCD(line1_string, line2_string);

    if (fan_status_setting == 2) {
      if (fan_status == 1) {
        printCharToLCD("*");
      } else {
        printCharToLCD(" ");
      }
    }

  } else if (page == 3) {

    String line1_string = "Led: " + getStatusSettingName(led_status_setting);
    String line2_string = getTimeString(led_status_time);

    print2ToLCD(line1_string, line2_string);

    if (led_status_setting == 2) {
      if (led_status == 1) {
        printCharToLCD("*");
      } else {
        printCharToLCD(" ");
      }
    }

  } else if (page == 4) {

    String line1_string = "Nem: " + getStatusSettingName(nem_status_setting);
    String line2_string = getTimeString(nem_status_time);

    print2ToLCD(line1_string, line2_string);

    if (nem_status_setting == 2) {
      if (nem_status == 1) {
        printCharToLCD("*");
      } else {
        printCharToLCD(" ");
      }
    }

  } else if (page == 5) {

    String line1_string = "Led Renk:";
    String line2_string = getLedRenkName(led_renk_setting);

    print2ToLCD(line1_string, line2_string);

  }


  // LED IŞIK:
  if (led_status_setting == 1) {
    run_led();

  } else if (led_status_setting == 0) {
    stop_led();

  } else if (led_status_setting == 2) {

    // OTOMATIK ISIK:
    // - ışık < 10 olduğunda; 4 saat aydınlık, ardından 8 saat bekleme ve kapatma.
    
    // Aç: Hava karardığında ışığı aç.
    if (led_status == 0 && isik < 7) {
      run_led();
      led_status_time = millis();
    }

    // Kapat: Verimli çalışmıyor. Açılan ışık, ışık değerini çok etkiliyor.
    /*
    if (led_status == 1 && isik > 40) {
      stop_led();
    }
    */

    if (led_status == 1) {
      // 12 saat sonra, led'i kapat.
      if (getTimeAsMinutes(led_status_time) > 720) {
        stop_led();
        led_status_time = millis();
      // 4 saat sonra, led'in ışığını 0 a ayarla.
      // Mantarlar için karanlık çok önemli, gece karanlık olmalı.
      } else if (getTimeAsMinutes(led_status_time) > 240) {
        openLed(0, 0, 0);
      }
    }

  }

  // FANLAR: 
  if (fan_status_setting == 1) {
    run_fan();
  } else if (fan_status_setting == 0) {
    stop_fan();
  } else if (fan_status_setting == 2) {

    // OTOMATIK FAN:
    // Rutin: 3590 sn kapat - 10sn çalış (Toplam 1 saat yani 3600sn)
    // Nemlendirme açılmış ise; onu yaymak için, temiz hava alma fanını çalıştır ama çıkış fanını kapat.

    if (nem_status == 1) {
      // Eğer nemlendirme cihazı çalışıyor ise, ve fanlar otomatik ise;
      // Fanlar sadece buharı kutu içine yaymak için özel bir durumda çalışsın.
      digitalWrite(fan1_pin, LOW);
      digitalWrite(fan2_pin, HIGH);
      fan_status = 1;
      // Not: Nemlendirme kapandığında; fanı da otomatik kapatacaktır.
      // Fan çalışma süresini değiştirme, fan kaldığı yerden devam etsin.

    } else {

      // Nemleme kapalı ise; fan belirlenen aralıklarda çalışmasını sürdürür.
      if (fan_status == 1) {
        if (getTimeAsSeconds(fan_status_time) > 10) {
          stop_fan();
          // Fan çalışma süresini sıfırla: 
          fan_status_time = millis();
        }
      } else {
        if (getTimeAsSeconds(fan_status_time) > 3590) {
          run_fan();
          // Fan çalışma süresini sıfırla:
          fan_status_time = millis();
        }
      }
      
    }

  }

  // NEMLENDIRME:
  if (nem_status_setting == 1) {

    // YETERINCE NEMLI: NEMLEMEYI ENGELLE:
    if (nem > 83) {
        stop_nem();
        nem_status_setting = 0; // Manuel olarak kapalıya ayarla; çünkü manuel olarak açık.
        nem_status_time = millis();

    } else {
      run_nem();
    }

  } else if (nem_status_setting == 0) {
    stop_nem();

  } else if (nem_status_setting == 2) {

    // OTOMATIK NEMLEME:
    // - nem %83'ü geçerse kapat ve tekrar çalışmasını engelle.
    // - Ilk açılmada; nem %65 in altında ise, 1dk 40sn nemlendir.
    // - nem %80 in altına düşerse; 10sn nemlendir.

    if (nem > 83) {
      if (nem_status == 1) {

        // YETERINCE NEMLI: NEMLEMEYI ENGELLE:
        stop_nem();
        nem_status_time = millis();
      }
    
    } else if (nem < 65) {
      if (nem_status == 0) {

        // ILK ÇALIŞTIRILMA NEMLENDIRMESI: AÇ
        run_nem();
        nem_time_to_run = 100;
        nem_status_time = millis();
      }

    } else if (nem < 80) {
      if (nem_status == 0) {

        // NEM KORUMA NEMLENDIRMESI: AÇ
        run_nem();
        nem_time_to_run = 10;
        nem_status_time = millis();
      }

    }

    // SURE TAMAMLANDIGINDA; NEMLENDIRMEYI KAPAT:
    if (nem_status == 1) {
      if (getTimeAsSeconds(nem_status_time) > nem_time_to_run) {
        stop_nem();
        // Nem çalışma süresini sıfırla:
        nem_status_time = millis();
      }
    }

  }

  // LCD ISIGINI KAPAT:
  // 5dk bir butona basılmaz ise ekran ışığını kapat.
  if(getTimeAsSeconds(button_clicked_time) > 300) {
    lcd.noBacklight();
  }

  delay(250);

}


