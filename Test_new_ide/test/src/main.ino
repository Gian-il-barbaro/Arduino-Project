const int NUM_DIGITS = 3;
const int MAX_ATTEMPTS = 6;

String solution;
int currentAttemptCount = 0;
bool gameActive = false;

void setup() {
  Serial.begin(9600);
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
  Serial.println("ARDUINO: Ricevuto: " + command);
  
  if (command == "GAME_START") {
    startNewGame();
  }
  else if (command.startsWith("TRY:")) {
    if (!gameActive) {
      Serial.println("ERROR: Gioco non attivo");
      return;
    }
    
    String attempt = command.substring(4);
    if (attempt.length() == NUM_DIGITS && isDigits(attempt)) {
      processAttempt(attempt);
    } else {
      Serial.println("ERROR: Tentativo non valido");
    }
  }
}

void startNewGame() {
  // Genera una soluzione casuale di 3 cifre
  solution = "";
  for (int i = 0; i < NUM_DIGITS; i++) {
    solution += String(random(0, 10));
  }
  
  currentAttemptCount = 0;
  gameActive = true;
  
  Serial.println("ARDUINO: Nuova partita - Soluzione: " + solution);
  Serial.println("GAME_START");
}

void processAttempt(String attempt) {
  currentAttemptCount++;
  
  Serial.println("ARDUINO: Tentativo " + String(currentAttemptCount) + ": " + attempt);
  
  // Calcola i colori per ogni cifra
  String colors[NUM_DIGITS];
  bool usedInSolution[NUM_DIGITS] = {false, false, false};
  
  // Prima passata: cifre corrette nella posizione giusta (right)
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (attempt[i] == solution[i]) {
      colors[i] = "right";
      usedInSolution[i] = true;
    }
  }
  
  // Seconda passata: cifre corrette ma in posizione sbagliata (wrong)
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (colors[i] == "right") continue; // Salta quelle già segnate come right
    
    for (int j = 0; j < NUM_DIGITS; j++) {
      if (!usedInSolution[j] && attempt[i] == solution[j]) {
        colors[i] = "wrong";
        usedInSolution[j] = true;
        break;
      }
    }
    
    // Se non è stato assegnato un colore, è empty
    if (colors[i] != "right" && colors[i] != "wrong") {
      colors[i] = "empty";
    }
  }
  
  // Costruisce la stringa di colori
  String colorString = "COLORS:";
  for (int i = 0; i < NUM_DIGITS; i++) {
    if (i > 0) colorString += ",";
    colorString += colors[i];
  }
  
  Serial.println(colorString);
  
  // DEBUG: Stampa i colori per verifica
  Serial.print("DEBUG Colors: ");
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
  } 
  else if (currentAttemptCount >= MAX_ATTEMPTS) {
    Serial.println("GAME_OVER:LOSE");
    gameActive = false;
    Serial.println("ARDUINO: SCONFITTA - Soluzione: " + solution);
  }
}

bool isDigits(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (!isDigit(str[i])) {
      return false;
    }
  }
  return true;
}