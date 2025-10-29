const int NUM_DIGITS = 3;
const int MAX_ATTEMPTS = 6;

String soluzione;
int currentAttemptCount = 0;
bool gameActive = false;


//---- variabili per led ----
#define red1 3      // led 1 colore rosso
#define green1 2    // led 1 colore verde
#define red2 4      // led 2 colore rosso
#define green2 7    // led 2 colore verde
#define red3 9      // led 3 colore rosso
#define green3 10   // led 3 colore verde
#define piezo 8     // pin positivo del buzzer

#define numLed 3  // numero di led

#define BASE 10  // numeri in base 10
int redPin;        // pin del colore rosso di un led
int greenPin;      // pin del colore verde di un led

int countersSol[BASE];  // per contare quante volte le cifre da 0 a 9 compaiono nella soluzione
int valutati[BASE];  // il numero di cifre dell'input già valutate
int cInput;   // l'i-esimo carattere della stringa input

void setup() {
  Serial.begin(9600);
   // imposto tutti i pin di tutti i led in modalità OUTPUT
  pinMode(red1, OUTPUT);
  pinMode(green1, OUTPUT);
  // pinMode(blue1, OUTPUT);
  pinMode(red2, OUTPUT);
  pinMode(green2, OUTPUT);
  // pinMode(blue2, OUTPUT);
  pinMode(red3, OUTPUT);
  pinMode(green3, OUTPUT);
  // pinMode(blue3, OUTPUT);

  // imposto il pin del piezo in modalità OUTPUT
  pinMode(piezo, OUTPUT);
  digitalWrite(piezo, LOW);
  
  switchOffAllLEDs();
  
  while (!Serial) {
    ; // Attendi che la seriale sia pronta
  }
  
  randomSeed(analogRead(0));
  
  Serial.println("ARDUINO: Pronto per gioco Wordle");
  Serial.println("ARDUINO: In attesa di GAME_START...");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
}

void processCommand(String command) {
  //Serial.println("ARDUINO: Ricevuto: " + command);
  
  if (command == "GAME_START") {
    startNewGame();
  }
  else if (command.startsWith("TRY:")) {
    if (!gameActive) {
      Serial.println("ERROR: Gioco non attivo");
      return;
    }
    
    String attempt = command.substring(4);
    //if (attempt.length() == NUM_DIGITS && isDigits(attempt)) {
    processAttempt(attempt);
    //} else {
      //Serial.println("ERROR: Tentativo non valido");
    //}
  }
}

void startNewGame() {

  switchOffAllLEDs();

  // Genera una soluzione casuale di 3 cifre
  soluzione = "";
  
  for (int i = 0; i < NUM_DIGITS; i++) {
    soluzione += String(random(0, 10));
  }

  for (int i = 0; i < BASE; i++)
    countersSol[i] = 0;
    

  for (int i = 0; i < soluzione.length(); i++)
    // conta quante occorrenze di ogni cifra ci sono nella soluzione
    countersSol[(int)(soluzione.charAt(i)) - 48]++;

  currentAttemptCount = 0;
  gameActive = true;
  
  Serial.println("ARDUINO: Nuova partita - Soluzione: " + soluzione);
  Serial.println("GAME_START");
}

void processAttempt(String attempt) {
  for (int i = 0; i < BASE; i++)
    valutati[i] = 0;
  currentAttemptCount++;
  
//  Serial.println("ARDUINO: Tentativo " + String(currentAttemptCount) + ": " + attempt);
  
  // Calcola i colori per ogni cifra
  String colors[NUM_DIGITS];
 // bool usedInSolution[NUM_DIGITS] = {false, false, false};


  // Prima passata: cifre corrette nella posizione giusta (right)
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (attempt[i] == soluzione[i]) {
      cInput = (int)(attempt.charAt(i)) - 48;
      colors[i] = "right";
      valutati[cInput]++; // incremento il numero di cInput valutati

      // faccio capire al sistema quale led deve accendere
      switch (i) {
        case 0:
          redPin = red1;
          greenPin = green1;
          break;
        case 1:
          redPin = red2;
          greenPin = green2;
          break;
        case 2:
          redPin = red3;
          greenPin = green3;
          break;
      }

      led(redPin, greenPin, 0, 255);
      // accendo il led di verde
    }
  }
  
  // Seconda passata: cifre corrette ma in posizione sbagliata (wrong)
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (colors[i] == "right") continue; // Salta quelle già segnate come right
    cInput = (int)(attempt.charAt(i)) - 48;
    // faccio capire al sistema quale led deve accendere
    switch (i) {
      case 0:
        redPin = red1;
        greenPin = green1;
        break;
      case 1:
        redPin = red2;
        greenPin = green2;
        break;
      case 2:
        redPin = red3;
        greenPin = green3;
        break;
    }
    if (valutati[cInput] < countersSol[cInput]) {
      colors[i] = "wrong";
      led(redPin, greenPin, 255, 127);
        // accendo il led di arancione
    }
    else{
      // se l'i-esimo carattere non è presente altre volte nella soluzione
      led(redPin, greenPin, 255, 0);
      // accendo il led di rosso
      colors[i]= "empty";
    }
    valutati[(int)(attempt[i])]++;
  }
  
  // Costruisce la stringa di colori
  String colorString = "COLORS:";
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (i > 0) colorString += ",";
    colorString += colors[i];
  }
  
  Serial.println(colorString);
  
  // DEBUG: Stampa i colori per verifica
  //Serial.print("DEBUG Colors: ");
  for (int i = 0; i < NUM_DIGITS; i++) {
    Serial.print(colors[i]);
    Serial.print(" ");
  }
  Serial.println();
  
  // Controlla vittoria/sconfitta
  bool isWin = true;
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (colors[i] != "right") {
      isWin = false;
      break;
    }
  }
  
  if (isWin) {
    Serial.println("GAME_OVER:WIN");
    gameActive = false;
    Serial.println("ARDUINO: VITTORIA!");
    for (int i = 0; i < 3; i++) {
      tone(piezo, 1000, 100);
      delay(250);
    }
    noTone(piezo);
  }
  
  else if (currentAttemptCount >= MAX_ATTEMPTS) {
    Serial.println("GAME_OVER:LOSE");
    gameActive = false;
    Serial.println("ARDUINO: SCONFITTA - Soluzione: " + soluzione);
  }
}

/*bool isDigits(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (!isDigit(str[i])) {
      return false;
    }
  }
  return true;
}*/


/**
	accende un led specifico:
    - redPin è il pin del rosso del led RGB
    - greenPin è il pin del verde del led RGB
    - red e green sono le quantità [0, 255] di colore
*/
void led(int redPin, int greenPin, int red, int green) {

  /*if (redPin == red2) {
    red = 255 - red;
    green = 255 - green;
  }*/

  analogWrite(redPin, red);
  analogWrite(greenPin, green);
}

/**
  spegne tutti i led
*/
void switchOffAllLEDs(){
  led(red1, green1, 0, 0);
  led(red2, green2, 255, 255);
  led(red3, green3, 0, 0);
}
