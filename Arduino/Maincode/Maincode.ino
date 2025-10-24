#define red1 3      // led 1 colore rosso
#define green1 2    // led 1 colore verde
#define red2 4      // led 2 colore rosso
#define green2 7    // led 2 colore verde
#define red3 9      // led 3 colore rosso
#define green3 10   // led 3 colore verde
#define piezo 8     // pin positivo del buzzer

#define numLed 3  // numero di led

#define BASE 10  // numeri in base 10

String soluzione;  // la soluzione
String input;      // l'input
int redPin;        // pin del colore rosso di un led
int greenPin;      // pin del colore verde di un led

// int countersInput[BASE];	// per contare quante volte le cifre da 0 a 9 compaiono nell'input
int countersSol[BASE];  // per contare quante volte le cifre da 0 a 9 compaiono nella soluzione

int cInput;  // l'i-esimo carattere della stringa input

int valutati[BASE];  // il numero di cifre dell'input già valutate

int x;             // per testare il funzionamento del codice, nella versione definitiva va eliminata
int combinazioni;  // numero di combinazioni possibili

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

  // per permettere al metodo random() di generare una soluzione casuale
  randomSeed(analogRead(0));

  x = 123;
  combinazioni = (int)(pow(BASE, numLed));
}

void loop() {

  soluzione = String(random(combinazioni));
  while (soluzione.length() < numLed)
    soluzione = "0" + soluzione;

  soluzione = "123";

  for (int i = 0; i < BASE; i++)
    countersSol[i] = 0;
  for (int i = 0; i < soluzione.length(); i++)
    // conta quante occorrenze di ogni cifra ci sono nella soluzione
    countersSol[(int)(soluzione.charAt(i)) - 48]++;

  // Serial.println(soluzione);

  do {
    // input = Serial.readString();
    // inserisci numero
    x = x % combinazioni;
    input = String(x);
    while (input.length() < numLed)
      input = "0" + input;
    // input = "122";
    Serial.println("Sol: " + soluzione + "\tIn: " + input);
    // Serial.println("In: " + input);

    /*for(int i=0; i<BASE; i++)
      // tutti i contatori dell'input vengono inzializzati a 0
      countersInput[i] = 0;
    
    for(int i=0; i<input.length(); i++)
      // tutti i contatori dell'input assumono un valore
      countersInput[(int)(input.charAt(i))-48]++;*/

  } while (input.length() != numLed || !isInteger(input));
  // l'input deve essere lungo esattamente quanto la soluzione e deve contenere solo cifre

  for (int i = 0; i < BASE; i++)
    valutati[i] = 0;

  // prima passata:
  // accendo solamente i led verdi e conto quanti ne ho accesi
  for (int i = 0; i < soluzione.length(); i++) {

    if (soluzione.charAt(i) == input.charAt(i)) {
      // se l'i-esimo carattere della soluzione e dell'input coincidono

      cInput = (int)(input.charAt(i)) - 48;

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

      valutati[cInput]++;
      // e incremento il numero di cInput valutati
    }
  }

  // seconda passata:
  // se il led non è stato acceso di verde
  for (int i = 0; i < soluzione.length(); i++) {

    if (soluzione.charAt(i) != input.charAt(i)) {
      // se l'i-esimo carattere della soluzione e dell'input non coincidono

      cInput = (int)(input.charAt(i)) - 48;

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

      if (/*contains(soluzione, input.charAt(i)) && */ valutati[cInput] < countersSol[cInput])
        // se l'i-esimo carattere è presente altre volte nella soluzione

        led(redPin, greenPin, 255, 127);
        // accendo il led di arancione
        
      else
        // se l'i-esimo carattere non è presente altre volte nella soluzione

        led(redPin, greenPin, 255, 0);
        // accendo il led di rosso

      valutati[cInput]++;
      // incremento il numero di cInput valutati
    }
  }
  
  if (soluzione == input) {
    // se l'input e la soluzione coincidono, faccio suonare il piezo
    for (int i = 0; i < 3; i++) {
      tone(piezo, 1000, 100);
      delay(250);
    }
  }
  else
    noTone(piezo);

  delay(2 * 1000);
  x++;
}

/**
	accende un led specifico:
    - redPin è il pin del rosso del led RGB
    - greenPin è il pin del verde del led RGB
    - red e green sono le quantità [0, 255] di colore
*/
void led(int redPin, int greenPin, int red, int green) {

  /*if (redPin == red1) {
    red = 255 - red;
    green = 255 - green;
  }*/

  analogWrite(redPin, red);
  analogWrite(greenPin, green);
}

/**
	data una stringa s, restituisce true se s può essere espressa tramite un numero intero,
    false altrimenti
*/
bool isInteger(String s) {

  for (int i = 0; i < s.length(); i++) {
    if (s.charAt(i) < '0' || s.charAt(i) > '9')
      return false;
  }
  return true;
}

/** 
	metodo bool;
    restituisce true se la stringa s contiene il carattere c,
    false altrimenti
*/
/*bool contains(String s, char c){
  
  for(int i=0; i<s.length(); i++){
    if(s.charAt(i) == c)
      return true;
  }
  return false;
  
}*/