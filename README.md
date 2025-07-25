Smart LoRa Watch — ESP32 Project

🌟 Описание
Это прошивка для умных часов, построенных на базе LilyGO T-Watch S3 / T-Display S3 / T-RGB или другой платы из серии LilyGO Display Series.
Часы работают на ESP32-S3, используют дисплей, сенсор касания, батарею и беспроводной модуль LoRa для обмена текстовыми уведомлениями.
Вся логика написана с использованием ESP-IDF v5.3+, с поддержкой OTA-обновлений и энергосбережения.

✅ Основные функции
Функция	Описание
📡 LoRa Связь	Часы принимают и отправляют короткие текстовые уведомления через LoRa-модуль. Поддержка адресации: групповая и индивидуальная.
🔔 Центр уведомлений	На экране хранится история до 20 последних сообщений. Можно листать, удалять или сбрасывать все.
⏰ Часы и дата	Постоянный дисплей времени и даты. Поддержка NTP или RTC для синхронизации.
🖱️ Сенсорный экран	Управление жестами: свайп для пролистывания сообщений, тап для открытия деталей.
🔋 Энергосбережение	Автоотключение дисплея, режим глубокого сна при неактивности.
⚡ OTA обновление	Возможность обновлять прошивку по воздуху.
🔒 Безопасность	Шифрование сообщений LoRa при необходимости.
🔧 Расширяемость	Подключение дополнительных сенсоров или модулей — например, шагомер или датчик температуры.
