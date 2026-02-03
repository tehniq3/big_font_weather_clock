# big_font_weather_clock
based on https://www.hackster.io/viorelracoviteanu/big-characters-weather-clock-int-ext-temp-hum-press-304296 and https://racov.ro/index.php/2021/02/13/arduino-big-font-weather-clock-int-ext-temp-hum-press/

better adjustment mode based on http://andydoz.blogspot.ro/2014_08_01_archive.html

1st article: https://nicuflorica.blogspot.com/2025/10/ceas-rtc-cu-caractetere-mari-si-date.html

2nd article: https://nicuflorica.blogspot.com/2025/10/ceas-rtc-cu-caractere-mari-si.html

3th article: https://nicuflorica.blogspot.com/2025/10/ceas-rtc-cu-caractere-mari-si-date.html

4th article: https://nicuflorica.blogspot.com/2025/10/ceas-rtc-cu-caractere-mari-si-date_25.html

- v.1 - Nicu FLORICA (niq_ro) changed t i2c interface for LCD2004
- v.1.a - changed sensors to DHT22 
- v.2 - added Doz (A.G.Doswell) mode to change the time with encoder: http://andydoz.blogspot.ro/2014_08_01_archive.html
- v.2.a - replaced one DHT22 sensor with combi AHT20+BMP280: https://docs.cirkitdesigner.com/component/3d8e6da8-841a-47a6-a1aa-0aa902c905e1/aht20bmp280
- v.2.b - replaced both DHT22 sensors with two SHT31/AHT31: https://qqtrading.com.my/GY-SHT31-D
- v.3 - removed both DHT22 sensors and added one DS18B20 sensor 
- v.3a - added adjustment also for seconds, show temperature without delay 
- v.3b - changed DS18B20 request and set the resolution 
- v.4 - switch big fonts for clock with temperature 
- v.4a - small updates to not put date on undesired position after 0:00:00 clock


![real](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEgSaCjHGQp6TIyCe67EoBY1ol5XwbnARtiiksTL2pP0f6IIkK7erDOtVf6z-Hn8OlYAExpmdv7PT-7WYgXUgrs8oXQ8R_0tjQyU5ZrshjSxyJHoUk_v2jx4AYp_eoU9caiE238SIVJgTKczhqHuOO0hmsMAIndLo1ecykir8Kxew8MDSmdge8U8yCIpymgV/s320/ceas_01.jpg)
![real2](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEjAvWLpc9Lj-qhGVlLusupFX5937_S4Y-nUcy3rDVFQitqt6y_tkHZCGRWTKssT62EXQK8tDA0AuU_YgKkB3tuw9xzxGXSn9-6xzNqJKatSihzl93s6S8kdmgJ7n0R1RK-c_8FPti1TmT5d59bkxStgUWrNbJa9NvAjYGwuVtIpYkEePtzBmf_ZN1q0YShL/w320-h149/ceas_02.jpg)

![temp1](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEhkSKQ2Z99Kj_DfVvLa_MD2nRdfoP8q5gmjJlxb1C6JpTKA8lsZVwqSyOMCfXygn1x6BM5SLXlJGaFlw_K_srfdmab_PlmpBZ2IEpXtrjqtnXB4OTHvUMNMrp8stA2-gMZ66NYud_A3yaJHqpQ9x_bwsxJWqRT2owE764GaM2yCObgIRF_bpCf1L7BA5bSE/w200-h93/temperatura_pozitiva_mare_mare.jpg)
![temp2](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEjb9ZujkXS-vmfhiuBA7QrNjddE4noo4Vq5iSpFoILLnl0-Lr7T8YKGZJHmvZtfuPO7G_r7yC2P8_qk_RIPTLg0BZgADdl-4bKGZ6kUPAH2-2Pmdd2iwBiWn4zFfcXdqPQrkCiP4RZT5RNqBz195a2YlpKz8QywPfEAuFBLbkb0DiyWa5s_68f6BXbfvvQU/w200-h93/temperatura_negativa_mare_mare.heic)
