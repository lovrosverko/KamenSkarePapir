#include <ArduinoMqttClient.h>
#include <WiFi101.h> // for MKR1000 change to: #include <WiFi101.h>

//definiranje pinova
#define vibro A1
#define Led11 4
#define Led12 3
#define Led13 2
#define Led21 5
#define Led22 7
#define Led23 6
#define Led31 10
#define Led32 9
#define Led33 8
#define LedPobjeda 11
#define LedGubitak 12

char ssid[] = "MW40V_2384";      //  your WiFi network SSID (name)
char pass[] = "04598995";       // your WiFi network key
//protokoli pomocu kojih se spajamo na server.
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);   
//informacije o tome na koji se server spajamo,koji je port servera,..., i definiranje varijabli.
const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic[]  = "sszc";
const char igra[] = "rezultat";
const long interval = 1000;
unsigned long previousMillis = 0;
String poruka = "";
bool game = false;
int count = 0;
int start;
int rezultat0;
int rezultat1;
int rezultat2;
int messageSize;
bool primljeno = false;
bool poslano = false;


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //Postavljamo ledice da budu output device i pozivamo funkciju iskljuci kako bi nam sve ledice bile ugasene napocetku.
  pinMode(Led11, OUTPUT);
  pinMode(Led12, OUTPUT);
  pinMode(Led13, OUTPUT);
  pinMode(Led21, OUTPUT);
  pinMode(Led22, OUTPUT);
  pinMode(Led23, OUTPUT);
  pinMode(Led31, OUTPUT);
  pinMode(Led32, OUTPUT);
  pinMode(Led33, OUTPUT);
  pinMode(LedPobjeda, OUTPUT);
  pinMode(LedGubitak, OUTPUT);
  iskljuci();

  // attempt to connect to Wifi network:
  Serial.print("Spajanje na WiFi:  ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // ako nije uspjelo čeka 5 sekundi i pokušavamo opet
    delay(5000);
  }

  Serial.println("Spojen na WiFi.");
  Serial.println();

  Serial.print("Pokušavam spajanje na MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT veza nije uspjela! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }
  Serial.println("Spojen na MQTT broker!");
  Serial.println();
  // pretplata na topic
  mqttClient.subscribe(igra);
}

void loop() {
  // MQTT keep alive funkcija poll() sprječava prekid veze s MQTT brokerom
  
  //provjeravamo dali se senzor za vibraciju zatresao i postavljamo rezultat nula da nam pretvara poruke koje dobiva od senzora u poruke koje se mogu koristiti u kodu.
  
  
  mqttClient.poll();
  slusaj();
  start = analogRead(vibro);
  if (start > 100) {
    rezultat1 = akcija();
    switch (rezultat1) {
        // u slucaju da je dobije 1 poziva funkciju koja pali ledice koje oznacavaju kamen.
      case 0:
        Serial.println("Kamen");
        poruka = "1";
        rezultat0 = 1;
        kamen();
        break;
        //u slucaju da je dobije 2 poziva funkciju koja pali ledice koje oznacavaju skare.
      case 1:
        Serial.println("Škare");
        poruka = "22";
        rezultat0 = 2;
        skare();
        break;
        //u slucaju da je dobije 3 poziva funkciju koja pali ledice koje oznacavaju papir.
      case 2:
        Serial.println("Papir");
        poruka = "333";
        rezultat0 = 3;
        papir();
        break;
    }
    //salje svoj dobiveni integer na server i poziva funkciju razmisljaj koja pali i gasi odredene ledica kako se nebi desilo da u krivo vrijeme igraci tresu senzore.
    posalji();
    razmisljaj();
    //ispisuje i poziva funkciju za usporedivanje rezultat0 i rezultat1
    Serial.println(rezultat0);
    Serial.println(rezultat2);
    usporedi(rezultat0, rezultat2);
    delay(5000);
    resetiranje();

  }

}
 //kreiranje funkcije usporedi (logika igre).
void usporedi(int player1, int player2) {
  if (player1 == player2) {
    Serial.println("Nerješeno");
    nerjeseno();
  }
  else if ((player1 == 1 && player2 == 2) || (player1 == 2 && player2 == 3) || (player1 == 3 && player2 == 1)) {
    Serial.println("Pobjeda!");
    pobjeda();
  }
  else {
    Serial.println("Booooo!");
    gubitak();
  }
}
//kreiranje funkcije slusaj koja provjerava dali je dosla poruka sa servera i govori nam kolika je velicina poruke,njen topic i koliko ima bajtova.
void slusaj() {
  messageSize = mqttClient.parseMessage();
  if (messageSize) {
    primljeno = true;
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");
    rezultat2 = messageSize;;
  }
}
//kreiranje funkcije posalji koja salje poruku na server do drugog igraca i govori nam njen topic i koja je poruka.
void posalji() {
  mqttClient.beginMessage(topic);
  mqttClient.print(poruka);
  mqttClient.endMessage();
  Serial.println();
  // debug
  Serial.print("Sending message to topic: ");
  Serial.println(topic);
  Serial.println(poruka);
  Serial.println();
  poslano = true;
}
//kreiranje funkcije za paljenje i gasenje ledica 5 puta kako igraci nebi u to vrijeme tresli senzor.
void razmisljaj() {
  for (int i = 0; i < 5; i++) {
    slusaj();
    digitalWrite(Led11, HIGH);
    delay(250);
    digitalWrite(Led12, HIGH);
    digitalWrite(Led21, HIGH);
    digitalWrite(Led11, LOW);
    delay(250);
    digitalWrite(Led31, HIGH);
    digitalWrite(Led13, HIGH);
    digitalWrite(Led22, HIGH);
    digitalWrite(Led12, LOW);
    digitalWrite(Led21, LOW);
    delay(250);
    digitalWrite(Led31, LOW);
    digitalWrite(Led13, LOW);
    digitalWrite(Led22, LOW);
    digitalWrite(Led23, HIGH);
    digitalWrite(Led32, HIGH);
    delay(250);
    digitalWrite(Led23, LOW);
    digitalWrite(Led32, LOW);
    digitalWrite(Led33, HIGH);
    delay(250);
    digitalWrite(Led33, LOW);
  }
}
//kreiranje funkcije koja pali ledice za skare.
void skare() {
  digitalWrite(Led11, HIGH);
  digitalWrite(Led13, HIGH);
  digitalWrite(Led22, HIGH);
  digitalWrite(Led31, HIGH);
  digitalWrite(Led33, HIGH);
  digitalWrite(Led12, LOW);
  digitalWrite(Led21, LOW);
  digitalWrite(Led23, LOW);
  digitalWrite(Led32, LOW);


}
//kreiranje funkcije koja pali ledice za papir.
void papir() {
  digitalWrite(Led11, HIGH);
  digitalWrite(Led13, HIGH);
  digitalWrite(Led22, LOW);
  digitalWrite(Led31, HIGH);
  digitalWrite(Led33, HIGH);
  digitalWrite(Led12, HIGH);
  digitalWrite(Led21, HIGH);
  digitalWrite(Led23, HIGH);
  digitalWrite(Led32, HIGH);

}
//kreiranje funkcije koja pali ledice za kamen.
void kamen() {
  digitalWrite(Led11, LOW);
  digitalWrite(Led13, LOW);
  digitalWrite(Led22, LOW);
  digitalWrite(Led31, LOW);
  digitalWrite(Led33, LOW);
  digitalWrite(Led12, HIGH);
  digitalWrite(Led21, HIGH);
  digitalWrite(Led23, HIGH);
  digitalWrite(Led32, HIGH);

}
//kreiranje funkcije za paljenje ledice koja oznacava pobjedu.
void pobjeda() {
  digitalWrite(LedPobjeda, HIGH);
  digitalWrite(LedGubitak, LOW);
}
//kreiranje funkcije za paljenje ledica koje oznacavaju nerjeseno.
void nerjeseno() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LedPobjeda, HIGH);
    digitalWrite(LedGubitak, LOW);
    delay(250);
    digitalWrite(LedPobjeda, LOW);
    digitalWrite(LedGubitak, HIGH);
    delay(250);
  }



}
//kreiranje funkcije za paljenje ledice koja oznacava gubitak.
void gubitak() {
  digitalWrite(LedPobjeda, LOW);
  digitalWrite(LedGubitak, HIGH);


}
//kreiranje funkcije za gasenje svih ledica da nebi ostale upaljene.
void iskljuci() {
  digitalWrite(Led11, LOW);
  digitalWrite(Led13, LOW);
  digitalWrite(Led22, LOW);
  digitalWrite(Led31, LOW);
  digitalWrite(Led33, LOW);
  digitalWrite(Led12, LOW);
  digitalWrite(Led21, LOW);
  digitalWrite(Led23, LOW);
  digitalWrite(Led32, LOW);
  digitalWrite(LedPobjeda, LOW);
  digitalWrite(LedGubitak, LOW);
}
// kreiranje funkcije koja resetira igru kako bi opet mogli igrati.
void resetiranje() {
  Serial.println("Reset");
  iskljuci();
  poslano = false;
  primljeno = false;
  rezultat0 = 0;
  rezultat1 = 0;
  rezultat2 = 0;
  for (int x = 0; x < 5; x++) {
    digitalWrite(LedPobjeda, HIGH);
    delay(500);
    digitalWrite(LedPobjeda, LOW);
    delay(500);

  }

}

// kreiranje funkcije koja vraca random integer od 1 do 3.
int akcija() {
  int akcija = random(3);
  return akcija;
}
