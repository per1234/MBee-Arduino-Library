 /**
  * Этот файл является частью библиотеки MBee-Arduino.
  * 
  * MBee-Arduino является бесплатным программным обеспечением. 
  * Подробная информация о лицензиях находится в файле mbee.h.
  * 
  * \author </i> von Boduen. Special thanx to Andrew Rapp.
  */
      
#include <MBee.h>

/** Скетч предназначен для демонстрации работы с библиотекой с помощью callback функций. 
  * Такой способ организации скетча позволяет избавиться от необходимости "вручную"
  * в функции loop() заниматься проверкой наличия поступивших от радиомодуля данных. Также 
  * автоматизируются многие действия, связанные с передачей пакетов.
  * Скетч предназначен для демонстрации передачи пакетов с данными удаленному модему.  
  * Передающий и принимающий модемы работают под управлением программного обеспечения SerialStar
  * для модулей MBee-868-x.0.
  * Действия, производимые скетчем подробно описаны в комментариях к соответствующим строкам.
  * Потребуются 2 модуля MBee-868-x.0. Первый модуль соедининяется с платой Arduino c помощью 
  * XBee-shield или любого другого совместимого устройств. Если доступного шилда нет, то возможно 
  * соединение Arduino и модуля с помощью проводов.
  * ВНИМАНИЕ!!! Модуль MBee-868-x.0 имеет номинальное значение напряжения питания 3,3В. Если Ваша
  * плата Arduino имеет выходы с логическими уровнями 5В, то необходимо предусмотреть делитель 
  * напряжения между выходом TX Arduino и входом RX модуля (вывод №3 для всех моделей). К выводу TX
  * Arduino подключается резистор 2К, с которым соединен резистор 1К, второй вывод последнего
  * сажается на землю. Точка соединения резисторов соединяется с выводом №3 модуля. 
  * Вывод №2 модуля (TX), подключается ко входу RX Arduino через последовательный резистор 1К.
  * При использовании для питания модуля собственного источника 3,3В Arduino, необходимо помнить о том,
  * что модули могут потреблять в режиме передачи токи до 200 мА. Поэтому необходимо уточнять 
  * нагрузочные характеристики применяемой Вами платы Arduino. При коротких эфирных пакетах для 
  * компенсации недостаточного выходного тока источника 3,3В можно применить конденсаторы с емкостью
  * не менее 2200 мкФ, устанавливаемые параллельно питанию модуля.
  * На обоих модулях, после загрузки программного обеспечения SerialStar, должен быть произведен возврат 
  * к заводским настройкам одним из двух способов:
  * 1. Быстрое 4-х кратное нажатие "SYSTEM BUTTON" (замыкание вывода №36 модуля на землю).
  * 2. С помощью командного режима: +++, AT RE<CR>, AT CN<CR>.
  * 
  * Первый модуль должен быть предварительно настроен для работы в пакетном режиме с escape-
  * символами AP=2. Режим аппаратного управления потоком (CTS/RTS) должен быть отключен.
  * Последовательность для настройки: +++, AT AP2<CR>, AT CN<CR>.
  * Для контроля процесса передачи, к Arduino должен быть подключен светодиод через резистор 1К.
  * 
  * Второй модуль устанавливается на плату MB-USBridge, или любой другой UART-USB/UART-COM 
  * преобразователь с выходными уровнями 3,3 В, для подключения к компьютеру. На компьютере
  * должна быть запущена любая терминальная программа с настройками порта 9600 8N1. 
  * Никакие дополнительные предварительные настройки второго модуля не требуются.
  */
 
SerialStarWithCallbacks mbee = SerialStarWithCallbacks();

uint16_t remoteAddress = 0x0001; //Адрес модема, которому будут передаваться данные.

char testString[] = "Hello world!";

#define ACKNOWLEDGE_TIMEOUT 1000 //Время ожидания подтверждения приема пакета от удаленного модема.

TxRequest tx = TxRequest(); //Пакет с данными для отправки удаленному модему. Для экономии памяти будем использовать один и тот же объект для отправки всех пакетов.
TxStatusResponse txStatus = TxStatusResponse(); //Локальный ответ со статусом команды.
RxAcknowledgeResponse remoteRxStatus = RxAcknowledgeResponse(); //Пакет с подтверждением от удаленного модема получение данных.

//int statusLed = 12;
int statusLed = LED_BUILTIN; //Используется встроенный в Вашу плату Arduino светодиод.

void setup() 
{
  pinMode(statusLed, OUTPUT);
  Serial.begin(9600);
  mbee.begin(Serial);
  delay(500); //Задержка не обязательна и вставлена для удобства работы с терминальной программой.
}

void loop() 
{
  tx.setRemoteAddress(remoteAddress); //Устанавливаем адрес удаленного модема.
  tx.setPayload((uint8_t*)testString); //Устанавливаем указатель на тестовую строку.
  tx.setPayloadLength(sizeof(testString) - 1); //Устанавливаем длину поля данных на 1 меньше, чем получаем по операции sizeof, чтобы не передавать терминирующий символ 0x00.
  //Далее приводятся возможные опции отправки. Опции действуют только если optionEnable = true. Если optionEnable = false, то режимом отправки управляет регистр Device Mode.
  //tx.setFrameId(0); //Отключает отправку локального подтверждения передачи/буферизации пакета.
  //tx.setOptionEnable(false); //Отключает опции отправки. При этом максимальный допустимый размер поля данных увеличивается на 1 байт.
  //tx.setAcknowledge(false); //Отключает подтверждение получения пакета удаленным модемом.  
  //tx.setCca(false); //Отключает режим CCA (Clear Channel Assessment) перед отправкой пакета.
  //tx.setEncryption(true); //Включает режим шифрования поля данных. Для расшифровки необходимо, чтобы на удаленном модеме совпадал ключ шифрования. Длина поля данных не должна превышать 32 байта.
  //tx.setSleepingDevice(true); //Сообщение будет помещено в буфер, и отправлено в эфир только после получения пакета от получателя.
  
  //Процесс доставки контролируется с помощью светодиода statusLed.
  //statusLed включился 3 раза на 200 мс - получено подтверждение доставки от удаленного модема.
  //statusLed включился 1 раз на 2000 мс - нет ответа от удаленного модема. 
  
  if(mbee.sendAndWaitForAcknowledge(tx, ACKNOWLEDGE_TIMEOUT) != MBEE_WAIT_TIMEOUT)
    flashLed(statusLed, 3, 200); //Данные успешно доставлены на удаленый модем.
  else
    flashLed(statusLed, 1, 2000); //Нет ответа от удаленного модема. 
  while(1); //Останавливаем скетч и ждем нажатия кнопки "RESET".
}

void flashLed(int pin, int times, int wait) 
{
  for (int i = 0; i < times; i++) 
  {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);
    if (i + 1 < times)
      delay(wait);
  }
}
