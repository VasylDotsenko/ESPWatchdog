# Coding Style

## Загальні принципи

Мета цього документа — забезпечити єдиний стиль коду, простоту читання та легкість підтримки проєкту.

Основні принципи:

* Простота.
* Читабельність.
* Передбачуваність.
* Один стиль у всьому проєкті.
* Мінімальна зв'язаність модулів.
* Відсутність дублювання коду.

---

# Структура проєкту

Кожен модуль складається з двох файлів:

```text
ModuleName.h
ModuleName.cpp
```

Приклади:

```text
Application.h
Application.cpp

Logger.h
Logger.cpp

Storage.h
Storage.cpp

Config.h
Config.cpp

SystemInfo.h
SystemInfo.cpp

WiFiService.h
WiFiService.cpp

PingService.h
PingService.cpp

WatchdogService.h
WatchdogService.cpp
```

---

# Один клас — одна відповідальність

Кожен клас виконує лише одну задачу.

Приклади:

| Клас            | Відповідальність      |
| --------------- | --------------------- |
| Logger          | Логування             |
| Storage         | Робота з LittleFS     |
| Config          | Конфігурація          |
| SystemInfo      | Інформація про ESP    |
| WiFiService     | Керування Wi-Fi       |
| PingService     | Перевірка доступності |
| PlugController  | Керування розеткою    |
| WatchdogService | Логіка watchdog       |

---

# Іменування класів

Класи використовують PascalCase.

```cpp
Logger

Storage

Config

SystemInfo

WiFiService

PingService

WatchdogService
```

---

# Глобальні екземпляри

Для кожного сервісу існує один глобальний об'єкт.

```cpp
Log

Storage

Config

SystemInfo

Network

Ping

Watchdog

WebServer

OTA
```

Не створювати додаткових глобальних екземплярів.

---

# Життєвий цикл сервісу

Кожен сервіс реалізує однаковий API.

```cpp
bool begin();

void loop();

const DataType& data() const;
```

---

# Структури даних

Кожен сервіс має власну структуру стану.

Назва структури:

```cpp
ConfigData

SystemData

NetworkData

PingData

WatchdogData
```

Доступ до стану:

```cpp
Config.data();

Network.data();

Ping.data();
```

---

# Машини станів

Кожен сервіс, який працює у фоновому режимі, використовує enum class.

Приклади:

```cpp
enum class NetworkState;

enum class PingState;

enum class WatchdogState;
```

Не використовувати загальні назви:

```cpp
State

Status

Mode
```

---

# Приватні поля

Усі приватні поля починаються з префікса:

```cpp
m_
```

Приклади:

```cpp
m_state

m_data

m_timer

m_retryCounter

m_lastUpdate
```

---

# Методи

Методи використовують camelCase.

Приклади:

```cpp
begin()

loop()

connect()

disconnect()

load()

save()

print()

updateData()

setDefaults()
```

---

# Константи

Не використовувати магічні числа.

Правильно:

```cpp
static constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;
```

Неправильно:

```cpp
delay(10000);
```

---

# constexpr

Для всіх внутрішніх констант використовувати:

```cpp
constexpr
```

або

```cpp
static constexpr
```

Не використовувати:

```cpp
#define
```

крім випадків, коли цього вимагають сторонні бібліотеки або системні макроси.

---

# Таймери

Усі часові операції виконуються через клас Timer.

Не використовувати:

```cpp
millis() - lastTime > interval
```

Правильно:

```cpp
if (timer.expired())
{
    ...
}
```

---

# delay()

Не використовувати `delay()` у логіці сервісів.

Виняток:

* аварійна зупинка;
* початкове завантаження обладнання, якщо цього неможливо уникнути.

---

# Логування

Усі повідомлення проходять через Logger.

Правильно:

```cpp
Log.info("Connected");

Log.warning("Reconnect");

Log.error("Configuration error");
```

Неправильно:

```cpp
Serial.println(...);
```

---

# Робота з конфігурацією

Усі налаштування отримуються тільки через Config.

Приклад:

```cpp
Config.data().hostname;

Config.data().wifiSSID;
```

Не читати файли напряму.

---

# Робота з LittleFS

Єдиний модуль, який працює з LittleFS:

```text
Storage
```

Інші модулі не повинні викликати LittleFS напряму.

---

# Залежності

Сервіси взаємодіють тільки через публічний API.

Приклад:

```cpp
Network.connected();

Ping.data();

Config.data();
```

Не використовувати внутрішні поля інших сервісів.

---

# Include

Порядок include:

1. Власний заголовок.
2. Заголовки проєкту.
3. Стандартні бібліотеки Arduino.
4. Сторонні бібліотеки.

Приклад:

```cpp
#include "WiFiService.h"

#include "Config.h"
#include "Logger.h"

#include <ESP8266WiFi.h>
```

---

# Коментарі

Коментарі використовуються лише для пояснення причин, а не очевидних дій.

Добре:

```cpp
// Повторна спроба підключення після таймауту.
```

Погано:

```cpp
// Збільшуємо i.
i++;
```

---

# Форматування

* Відступ — 4 пробіли.
* Відкриваюча дужка `{` на новому рядку.
* Один порожній рядок між логічними блоками.
* Один оператор на рядок.

---

# Архітектурні принципи

* Один клас — одна відповідальність.
* Мінімальна зв'язаність.
* Максимальна модульність.
* Відсутність блокуючих викликів.
* Відсутність дублювання коду.
* Передбачуваний життєвий цикл сервісів.
* Розширення без зміни існуючих модулів.
* Код повинен бути придатним до повторного використання в інших проєктах.

---

# Нові модулі

Перед створенням нового сервісу необхідно визначити:

* відповідальність;
* структуру даних;
* машину станів (за потреби);
* API (`begin()`, `loop()`, `data()`);
* залежності від інших сервісів.

Після цього створюються:

```text
ModuleName.h
ModuleName.cpp
```

і модуль реєструється в `Application`.
