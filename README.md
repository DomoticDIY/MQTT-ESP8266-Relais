# MQTT-ESP8266-Relais
Action d'un relais via MQTT depuis Domoticz

But : Commander un relais MQTT via Domoticz.

Voici comment créer un déclencheur, commander par MQTT. L'intéret est de pouvoir déclencher un relais à distance, ce dernier permettant d'activer l'alimentation d'un appareil électrique, que ce soit moteur, éclairage,...., tout est possible, dès lors que la consommation de l'appareil commandé ne dépasse pas la Tension (volts), et Intensité (Ampéres) maximum autorisé par le relais, et en fonction du type de courant.

Partie logiciel necessaire : 
- Driver USB CH340G : https://wiki.wemos.cc/downloads 
- Logiciel Arduino IDE : https://www.arduino.cc/en/Main/Software 
- URL à ajouter pour le Bord manager : http://arduino.esp8266.com/stable/package_esp8266com_index.json 

Installer la prise en charge des cartes ESP8266

Bibliothéques : 
 - pubsubclient : https://github.com/knolleary/pubsubclient 
 - ArduinoJson : https://github.com/bblanchon/ArduinoJson 
 
Dans IDE : Faire Croquis / inclure une bibliothéque / ajouter la bibliothèque ZIP. 

Adaptation pour reconnaissance dans Domoticz : 
Dans le fichier PubSubClient.h : La valeur du paramètre doit être augmentée à 512 octets. Cette définition se trouve à la ligne 26 du fichier, sinon cela ne fonctionne pas avec Domoticz



Vidéo explicative sur YouTube : https://www.youtube.com/watch?v=6HclvzhEWMg
