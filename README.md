### Arduino Mantar Otomasyonu (v1.0)

Bu yazılım, 30Lt'lik bir plastik kutu içerisinde; Arduino kullanarak, tam otomatik istiridye mantarı yetiştirmek için geliştirilmiştir.<br />

<br>

### Otomasyon:

- Nem %80'in üzerinde kalacak şekilde nemlendirme.
- Saat başı, 10sn havalandırma.
- Hava karardığında, 4 saatliğine aydınlatmanın açılması.
- Sıcaklık konusunda bir otomasyon yoktur. Sadece LCD ekran üzerinden bilgi verilmektedir.

### Ekran Görüntüleri:

![arduino mantar otomasyonu](https://bug7a.github.io/arduino-mantar-otomasyonu/img1.jpeg)

### Kullanılan Malzemeler:

- Arduino Leonardo
- 16x2 IIC/I2C Seri LCD Display
- MQ-135 Hava Kalitesi Modülü
- LDR Modülü KY-018
- DHT11 Isı ve Nem Sensörü Kart
- 3.3V/5V Breadboard Power Module
- Neopixel 8 Rgb Led - 8'li Ws2812 5050 Rgb Led
- 30x30x10mm Fan 12V (2 Adet)
- L298N Voltaj Regulatörlü Çift Motor Sürücü Kartı(Kırmızı PCB)
- DC Güç Adaptörü Jak Soketi Modülü
- Arduino Uzaktan Kumanda Kiti
- Hepa Filtre
- Ultrasonik Hava Nemlendirme Cihazı

### Ek Notlar:

- Neopixel 8 Rgb Led - 8'li Ws2812 5050 Rgb Led ve Ultrasonik Hava Nemlendirme Cihazı enerjisini 3.3V/5V Breadboard Power Module üzerinden almaktadır.
- 3.3V/5V Breadboard Power Module ve L298N Voltaj Regulatörlü Çift Motor Sürücü Kartı(Kırmızı PCB) enerjisini DC Güç Adaptörü Jak Soketi Modülü üzerinden almaktadır.
- Arduino Leonardo enerjisini L298N Voltaj Regulatörlü Çift Motor Sürücü Kartı(Kırmızı PCB) üzerinden almaktadır.
- Havalandırma fanları, L298N Voltaj Regulatörlü Çift Motor Sürücü Kartı(Kırmızı PCB) ile yönetilmektedir.
- DC Güç Adaptörü Jak Soketi Modülü, 12V 1A lik bir adaptör ile beslenmektedir.
- Ultrasonik Hava Nemlendirme Cihazı, bir 2N2222 transistör ile açılıp, kapatılmaktadır.
- Fanlardan 1'i temiz havanın kutu içine alınması için, diğeri kirli havanın dışarı atılması için kullanılmaktadır.
- Aydınlatma, görsel amaçlıdır. Kutunun tam gün -gölgede- ışık gören bir ortamda bulunması gerekir.
