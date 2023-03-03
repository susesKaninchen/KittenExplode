# KittenExplode
Das Projekt besteht aus einem I2C MAster und einer Reihe von I2C Slaves.

Momentan ist es auf ESP32 ausgelegt, diese haben kein I2C stretching und deswegen verhalten sie sich anders als ESP32 C3 oder STM32 oder Arduinos. Der Slave code müste dann angepasst werden.

Der Slave kann das Status Byte (statusChar) anpassen um dem Master bescheid zu geben wie der aktuelle Status ist.
Der Master kann das EndByte (endByte) anpassen um den Slaves den Stand des Spiel mitzuteilen.
Der vom Master gesetzte Seed (seed) soll genutzt werden um die Slaves zu konfigurieren.