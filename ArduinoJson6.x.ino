/*
 ESP8266 MQTT - Déclenchement d'un relais
 Création Dominique PAUL.
 Dépot Github : https://github.com/DomoticDIY/MQTT-ESP8266-Relais

 Bibliothéques nécessaires :
 - pubsubclient : https://github.com/knolleary/pubsubclient
 - ArduinoJson v5.13.3 : https://github.com/bblanchon/ArduinoJson
Télécharger les bibliothèques, puis dans IDE : Faire Croquis / inclure une bibliothéque / ajouter la bibliothèque ZIP.
Dans le gestionnaire de bibliothéque, charger le module ESP8266Wifi.

Installaer le gestionnaire de carte ESP8266 version 2.5.0 
Si besoin : URL à ajouter pour le Bord manager : http://arduino.esp8266.com/stable/package_esp8266com_index.json

Pour prise en compte du matériel :
Installer si besoin le Driver USB CH340G : https://wiki.wemos.cc/downloads
dans Outils -> Type de carte : generic ESP8266 module
  Flash mode 'QIO' (régle générale, suivant votre ESP, si cela ne fonctionne pas, tester un autre mode.
  Flash size : 1M (no SPIFFS)
  Port : Le port COM de votre ESP vu par windows dans le gestionnaire de périphériques.

  Programateur : USBasp
*/

// Inclure les librairies.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// =================================================================================================

// ----------------------------------
// Définitions liées au module relais
// ----------------------------------

String      nomModule = "Module relais";    // Nom usuel de ce module.
int         pinRelais = 0;                  // Pin sur lequel est connecté la commande du relais.

// ------------------------------------
// Définitions liées à Domoticz et MQTT
// ------------------------------------

char*       topicIn =   "domoticz/out";     // Nom du topic envoyé par Domoticz
char*       topicOut =  "domoticz/in";      // Nom du topic écouté par Domoticz
int         idxDevice = idx_du_device_dans_domoticz;                 // Index du Device à actionner.
const char* mqtt_server = "Adresse_du_broker_MQTT";   // Adresse IP ou DNS du Broker MQTT.
const int   mqtt_port = 1883;               // Port du Brocker MQTT
const char* mqtt_login = "login_MQTT";            // Login de connexion à MQTT.
const char* mqtt_password = "Mot_de_passe_MQTT";    // Mot de passe de connexion à MQTT.

// -------------------------------------------------------------
// Définitions liées au WIFI
// -------------------------------------------------------------
const char* ssid = "SSID_de_votre_AP_WIFI";             // SSID du réseau Wifi
const char* password = "Mot_de_passe_wifi";     // Mot de passe du réseau Wifi.

// =================================================================================================

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(pinRelais, OUTPUT);
  digitalWrite(pinRelais, HIGH);
     
  Serial.begin(115200);
  
  client.setBufferSize(512);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  while (WiFi.status() != WL_CONNECTED)
    setup_wifi();
    
  if (!client.connected())
    reconnect();

  if (client.connected())
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
    // Tant que l'on n'est pas connecté, on boucle.
    delay(500);
    Serial.print(".");
  }

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
    // Serial.print("Message entrant [");
    // Serial.print(topic);
    // Serial.print("] ");
    for (int i = 0; i < length; i++) {
      string+=((char)payload[i]);
    }
   // Affiche le message entrant
   // Serial.println(string);
    
    // Parse l'objet JSON nommé "root"
    // StaticJsonBuffer<512> jsonBuffer;                  // ancienne librairie
    // JsonObject &root = jsonBuffer.parseObject(string); // ancienne librairie

    StaticJsonDocument<512> jsonDoc;
    DeserializationError err = deserializeJson(jsonDoc, string);
    
    // if (root.success()) { // ancienne librairie
    if(err) {
      Serial.println("Erreur de lecture du JSON !");
    } else {
      int idx = jsonDoc["idx"];
      int nvalue = jsonDoc["nvalue"];
  
      // Activer / désactiver la commande de relais.
      if (idx == idxDevice) {
        digitalWrite(pinRelais, (nvalue == 0) ? HIGH : LOW); 
        Serial.print("Device ");
        Serial.print(idx);
        Serial.println((nvalue == 0) ? " sur OFF" : " sur ON");
      } /* else {
        Serial.print("Reçu information du Device : ");
        Serial.println(idx);
      } */
    }
  }
}

void reconnect() {

  // Initialise la séquence Random
  randomSeed(micros());
  
  // Boucle jusqu'à la connexion MQTT
//  while (!client.connected()) {
    Serial.print("Tentative de connexion MQTT...");
    // Création d'un ID client aléatoire
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Tentative de connexion
    if (client.connect(clientId.c_str(), mqtt_login, mqtt_password)) {
      Serial.println("connecté");
      
      // Connexion effectuée, publication d'un message...
      String message = "Connexion MQTT de "+ nomModule + " (ID : " + clientId + ")";
      char messageChar[message.length()+1];
      message.toCharArray(messageChar,message.length()+1);
      
      // StaticJsonBuffer<512> jsonBuffer;  // ancienne librairie
      // JsonObject &root = jsonBuffer.createObject(); // ancienne librairie
      const int capacity = JSON_OBJECT_SIZE(2);
      StaticJsonDocument<capacity> root;
 
      // On renseigne les variables.
      root["command"] = "addlogmessage";
      root["message"] = (const char *) messageChar;
      
      // On sérialise la variable JSON
      String JsonStr;
      serializeJson(root, JsonStr);
      // Serial.println(JsonStr);
      
      // if (root.printTo(messageOut) == 0) {
      //   Serial.println("Erreur lors de la création du message de connexion pour Domoticz");
      // } else  {
        // Convertion du message en Char pour envoi dans les Log Domoticz.
        char JsonStrChar[JsonStr.length()+1];
        JsonStr.toCharArray(JsonStrChar,JsonStr.length()+1);
        client.publish(topicOut, JsonStrChar);
      // }
        
      // On souscrit
      client.subscribe("#");
    } else {
      Serial.print("Erreur, rc=");
      Serial.print(client.state());
      Serial.println(" prochaine tentative dans 5s");
      // Pause de 5 secondes
      delay(5000);
    }
//  }
}
