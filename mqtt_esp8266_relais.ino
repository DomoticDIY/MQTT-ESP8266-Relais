
/*
 ESP8266 MQTT - Déclenchement d'un relais
 Création Dominique PAUL.
 Dépot Github : https://github.com/DomoticDIY/MQTT-ESP8266-Relais

 Bibliothéques nécessaires :
 - pubsubclient : https://github.com/knolleary/pubsubclient
 - ArduinoJson : https://github.com/bblanchon/ArduinoJson
Télécharger les bibliothèques, puis dans IDE : Faire Croquis / inclure une bibliothéque / ajouter la bibliothèque ZIP.
Dans le gestionnaire de bibliothéque, charger le module ESP8266Wifi.

Installaer le gestionnaire de carte ESP8266 version 2.5.0 
Si besoin : URL à ajouter pour le Bord manager : http://arduino.esp8266.com/stable/package_esp8266com_index.json

Adaptation pour reconnaissance dans Domoticz :
Dans le fichier PubSubClient.h : La valeur du paramètre doit être augmentée à 512 octets. Cette définition se trouve à la ligne 26 du fichier.
Sinon cela ne fonctionne pas avec Domoticz

Pour prise en compte du matériel :
Installer si besoin le Driver USB CH340G : https://wiki.wemos.cc/downloads
dans Outils -> Type de carte : generic ESP8266 module
  Flash mode 'QIO' (régle générale, suivant votre ESP, si cela ne fonctionne pas, tester un autre mode.
  Flash size : 1M (no SPIFFS)
  Port : Le port COM de votre ESP vu par windows dans le gestionnaire de périphériques.

*/

// Inclure les librairies.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Déclaration des constantes, données à adapter à votre réseau.
// ------------------------------------------------------------
const char* ssid = "_MON_SSID_";                // SSID du réseau Wifi
const char* password = "_MOT_DE_PASSE_WIFI_";   // Mot de passe du réseau Wifi.
const char* mqtt_server = "_IP_DU_BROKER_";     // Adresse IP ou DNS du Broker.
const int mqtt_port = 1883;                     // Port du Brocker MQTT
const char* mqtt_login = "_LOGIN_";             // Login de connexion à MQTT.
const char* mqtt_password = "8PASSWORD_";       // Mot de passe de connexion à MQTT.
// ------------------------------------------------------------
// Variables et constantes utilisateur :
String nomModule = "Module relais";     // Nom usuel de ce module.
char* topicIn = "domoticz/out";         // Nom du topic envoyé par Domoticz
char* topicOut = "domoticz/in";         // Nom du topic écouté par Domoticz
int pinRelais = 2;                      // Pin sur lequel est connecté la commande du relais.
int idxDevice = 27;                     // Index du Device à actionner.
// ------------------------------------------------------------


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void setup() {
  pinMode(pinRelais, OUTPUT);   
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}


void setup_wifi() {
  // Connexion au réseau Wifi
  delay(10);
  Serial.println();
  Serial.print("Connection au réseau : ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.print("Addresse IP : ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Message reçu du Broker.
  String string;
  // On vérifie qu'il vient bien de Domoticz.
  int valeur = strcmp(topic, topicIn);
  if (valeur == 0) {
    Serial.print("Message arrivé [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      string+=((char)payload[i]);
    }
   // Affiche le message entrant - display incoming message
    Serial.println(string);

    // Parse l'objet JSON nommé "root"
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(string);
    if (root.success()) {
      int idx = root["idx"];
      int nvalue = root["nvalue"];
  
      // Activer la sortie du relais si 1a "nvalue" 1 est reçu.
      if (idx == idxDevice && nvalue == 1) {
        digitalWrite(pinRelais, LOW); 
        Serial.print("Device ");
        Serial.print(idx);
        Serial.println(" sur ON");
      } else if (idx == idxDevice && nvalue == 0) {
        digitalWrite(pinRelais, HIGH); 
        Serial.print("Device ");
        Serial.print(idx);
        Serial.println(" sur OFF");
      } else if (idx != idxDevice) {
        Serial.print("Reçu information du Device : ");
        Serial.println(idx);
      }
    } else {
      Serial.println("Erreur de lecture du JSON !");
    }
  }

}

void reconnect() {
  // Boucle jusqu'à la connexion MQTT
  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    // Création d'un ID client aléatoire
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Tentative de connexion
    if (client.connect(clientId.c_str(), mqtt_login, mqtt_password)) {
      Serial.println("connecté");
      
      // Connexion effectuée, publication d'un message...
      String message = "Connexion MQTT de "+ nomModule + " réussi sous référence technique : " + clientId + ".";
      // String message = "Connexion MQTT de "+ nomModule + " réussi.";
      StaticJsonBuffer<256> jsonBuffer;
      // Parse l'objet root
      JsonObject &root = jsonBuffer.createObject();
      // On renseigne les variables.
      root["command"] = "addlogmessage";
      root["message"] = message;
      
      // On sérialise la variable JSON
      String messageOut;
      if (root.printTo(messageOut) == 0) {
        Serial.println("Erreur lors de la création du message de connexion pour Domoticz");
      } else  {
        // Convertion du message en Char pour envoi dans les Log Domoticz.
        char messageChar[messageOut.length()+1];
        messageOut.toCharArray(messageChar,messageOut.length()+1);
        client.publish(topicOut, messageChar);
      }
        
      // On souscrit (écoute)
      client.subscribe("#");
    } else {
      Serial.print("Erreur, rc=");
      Serial.print(client.state());
      Serial.println(" prochaine tentative dans 5s");
      // Pause de 5 secondes
      delay(5000);
    }
  }
}

