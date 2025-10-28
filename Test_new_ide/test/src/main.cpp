#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Configurazione Access Point
const char* ssid = "ESP32-Wordle";
const char* password = "12345678";

WebServer server(80);
WebSocketsServer webSocket(81);

String serialBuffer = "";
bool gameActive = false;

// HTML page
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wordle ESP32+Arduino</title>
    <style>
        html, body {
            background: #121213;
            margin: 0;
            padding: 0;
            height: 100%;
            font-family: Arial, sans-serif;
        }
        
        #game {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            padding: 20px;
            color: white;
        }
        
        .game-title {
            color: white;
            text-align: center;
            margin-bottom: 30px;
            font-size: 2em;
        }
        
        #game-status {
            color: #b59f3b;
            text-align: center;
            margin: 20px;
            font-size: 1.2em;
            min-height: 25px;
            font-weight: bold;
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(3, 70px);
            gap: 8px;
            margin: 20px 0;
        }
        
        .box {
            width: 70px;
            height: 70px;
            border: 2px solid #3a3a3c;
            color: white;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 1.8em;
            font-weight: bold;
            border-radius: 8px;
            background: #3a3a3c;
        }
        
        .box.empty {
            background: #3a3a3c;
            border-color: #3a3a3c;
        }
        
        .box.wrong {
            background: #b59f3b;
            border-color: #b59f3b;
        }
        
        .box.right {
            background: #538d4e;
            border-color: #538d4e;
        }
        
        .animated {
            animation-name: flip;
            animation-duration: 0.5s;
            animation-timing-function: ease;
            animation-fill-mode: forwards;
        }
        
        @keyframes flip {
            0% {
                transform: scaleY(1);
            }
            50% {
                transform: scaleY(0);
            }
            100% {
                transform: scaleY(1);
            }
        }
        
        .instructions {
            color: #888;
            text-align: center;
            margin-top: 30px;
            max-width: 400px;
            line-height: 1.4;
        }

        #restart-btn {
            background: #538d4e;
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1.1em;
            margin-top: 20px;
            display: none;
        }

        #restart-btn:hover {
            background: #4a7c45;
        }

        #restart-btn:disabled {
            background: #3a3a3c;
            cursor: not-allowed;
        }
    </style>
</head>
<body>
    <div id="game">
        <h1 class="game-title">WORDLE ESP32+ARDUINO</h1>
        <div id="game-status">Connettendo...</div>
        <div class="grid" id="grid"></div>
        <button id="restart-btn" onclick="startNewGame()">Nuova Partita</button>
        <div class="instructions">
            <strong>COME GIOCARE:</strong><br>
            • Inserisci 3 cifre e premi INVIO<br>
            • Verde: cifra corretta e posizione giusta<br>
            • Giallo: cifra corretta ma posizione sbagliata<br>
            • Grigio: cifra non presente
        </div>
    </div>
    
    <script>
        const tentativi = 6;
        const numeri = 3;
        
        const state = {
            grid: Array(tentativi).fill().map(() => Array(numeri).fill('')),
            currentRow: 0,
            currentCol: 0,
            gameOver: false
        };
        
        let websocket;

        function initWebSocket() {
            const host = window.location.hostname;
            websocket = new WebSocket('ws://' + host + ':81/');

            websocket.onopen = function(event) {
                updateStatus("Connesso! Inserisci " + numeri + " cifre");
                createGrid();
                handleKeyPress();
                updateGrid();
            };

            websocket.onclose = function(event) {
                updateStatus("Connessione persa - riconnessione...");
                setTimeout(initWebSocket, 3000);
            };

            websocket.onerror = function(event) {
                updateStatus("Errore di connessione");
            };

            websocket.onmessage = function(event) {
                console.log("Ricevuto:", event.data);
                
                if (event.data.startsWith("COLORS:")) {
                  // Trim globale per rimuovere \r\n finali
                  const colorString = event.data.substring(7).trim();
                  // rimuove spazi e CR/LF da ogni elemento
                  const colors = colorString.split(',').map(c => c.trim());
                  console.log("Parsed COLORS:", colors);
                  applyColorsToCurrentRow(colors);
                }
                else if (event.data.startsWith("GAME_OVER:")) {
                    const result = event.data.substring(10).trim();
                    handleGameOver(result);
                }
                else if (event.data === "GAME_START") {
                    resetGame();
                    updateStatus("Nuova partita - Inserisci " + numeri + " cifre");
                }
                else {
                    updateStatus(event.data);
                }
            };
        }

        function handleGameOver(result) {
            state.gameOver = true;
            
            if (result === "WIN") {
                updateStatus("🎉 HAI VINTO! 🎉");
            } else {
                updateStatus("😞 HAI PERSO! 😞");
            }
            
            // Mostra il pulsante dopo un breve delay
            setTimeout(() => {
                document.getElementById('restart-btn').style.display = 'block';
            }, 1500);
        }

        function applyColorsToCurrentRow(colors) {
        const row = state.currentRow;
        const animationDuration = 500; // ms

        for (let col = 0; col < numeri; col++) {
            const box = document.getElementById('box-' + row + '-' + col);
            // safe read + normalizzazione
            const raw = (colors[col] || '').trim().toLowerCase();
            const color = raw; // es. 'right', 'wrong', 'empty' o '' se mancante

            if (box) {
                // rimuovi eventuali classi di colore precedenti
                box.classList.remove('right', 'wrong', 'empty');

                // imposta ritardo animazione (in secondi)
                box.style.animationDelay = (col * animationDuration / 1000) + 's';

                // riavvia animazione (remove + forced reflow + add)
                box.classList.remove('animated');
                void box.offsetWidth;
                box.classList.add('animated');

                // assegna classe colore dopo delay per sincronizzare con flip
                setTimeout(() => {
                    const allowed = ['right', 'wrong', 'empty'];
                    if (color && allowed.includes(color)) {
                        box.classList.add(color);
                    } else if (color) {
                        console.warn('COLORS: valore non riconosciuto:', JSON.stringify(color));
                        box.classList.add('empty'); // fallback
                    } else {
                        // fallback se mancante
                        box.classList.add('empty');
                    }
                }, col * animationDuration);
            }
        }

        // avanzamento riga dopo tutte le animazioni
        setTimeout(() => {
            if (!state.gameOver) {
                state.currentRow++;
                state.currentCol = 0;
                if (state.currentRow < tentativi) {
                    updateStatus("Tentativo " + (state.currentRow + 1) + "/" + tentativi + " - Inserisci " + numeri + " cifre");
                }
            }
        }, numeri * animationDuration);
    }


        function updateStatus(message) {
            document.getElementById('game-status').textContent = message;
        }

        function createGrid() {
            const gridElement = document.getElementById('grid');
            gridElement.innerHTML = '';
            
            for (let row = 0; row < tentativi; row++) {
                for (let col = 0; col < numeri; col++) {
                    const box = document.createElement('div');
                    box.className = 'box';
                    box.id = 'box-' + row + '-' + col;
                    box.textContent = state.grid[row][col];
                    gridElement.appendChild(box);
                }
            }
        }

        function handleKeyPress() {
            document.body.onkeydown = (e) => {
                if (state.gameOver) return;

                const key = e.key;

                if (key === 'Backspace') {
                    removeNumber();
                } else if (key === 'Enter') {
                    if (state.currentCol === numeri) {
                        const number = getCurrentNumber();
                        revealNumber(number);
                    }
                } else if (isNumber(key)) {
                    addNumber(key);
                }

                updateGrid();
            };
        }

        function getCurrentNumber() {
            return state.grid[state.currentRow].join('');
        }

        function revealNumber(number) {
            websocket.send('TRY:' + number);
            updateStatus("Invio tentativo ad Arduino...");
        }

        function startNewGame() {
            websocket.send("GAME_START");
            resetGame();
            document.getElementById('restart-btn').style.display = 'none';
        }

        function resetGame() {
            state.grid = Array(tentativi).fill().map(() => Array(numeri).fill(''));
            state.currentRow = 0;
            state.currentCol = 0;
            state.gameOver = false;
            
            // Reset interfaccia
            for (let row = 0; row < tentativi; row++) {
                for (let col = 0; col < numeri; col++) {
                    const box = document.getElementById('box-' + row + '-' + col);
                    if (box) {
                        box.className = 'box';
                        box.style.animationDelay = '';
                        box.textContent = '';
                    }
                }
            }
            
            updateGrid();
        }

        function isNumber(key) {
            return key.length === 1 && '0123456789'.includes(key);
        }

        function addNumber(number) {
            if (state.currentCol >= numeri) return;
            state.grid[state.currentRow][state.currentCol] = number;
            state.currentCol++;
        }

        function removeNumber() {
            if (state.currentCol === 0) return;
            state.grid[state.currentRow][state.currentCol - 1] = '';
            state.currentCol--;
        }

        function updateGrid() {
            for (let row = 0; row < state.grid.length; row++) {
                for (let col = 0; col < state.grid[row].length; col++) {
                    const box = document.getElementById('box-' + row + '-' + col);
                    if (box) {
                        box.textContent = state.grid[row][col];
                    }
                }
            }
        }

        // Avvia tutto quando la pagina è caricata
        window.onload = initWebSocket;
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);   // Comunicazione con Arduino
  
  delay(1000);
  
  Serial.println("=== AVVIO ESP32 ACCESS POINT ===");
  
  // Crea Access Point
  WiFi.softAP(ssid, password);
  delay(2000);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Access Point creato!");
  Serial.printf("SSID: %s\n", ssid);
  Serial.printf("Password: %s\n", password);
  Serial.printf("IP: %s\n", myIP.toString().c_str());

  // Setup server HTTP
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage);
  });

  server.begin();
  
  // Setup WebSocket
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
      case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnesso\n", num);
        break;
      case WStype_CONNECTED:
        {
          IPAddress ip = webSocket.remoteIP(num);
          Serial.printf("[%u] Client connesso: %s\n", num, ip.toString().c_str());
          webSocket.sendTXT(num, "Benvenuto! Inserisci 3 cifre e premi INVIO");
          
          // Avvia il gioco quando si connette il primo client
          if (!gameActive) {
            gameActive = true;
            Serial2.println("GAME_START");
            Serial.println("Inviato GAME_START ad Arduino");
          }
        }
        break;
      case WStype_TEXT:
        {
          String message = String((char*)payload);
          Serial.printf("[%u] Ricevuto: %s\n", num, message.c_str());
          
          if (message.startsWith("TRY:")) {
            String attempt = message.substring(4);
            Serial2.println("TRY:" + attempt);
            Serial.println("Inviato ad Arduino: TRY:" + attempt);
          }
          else if (message == "GAME_START") {
            Serial2.println("GAME_START");
            Serial.println("Inviato GAME_START ad Arduino");
            gameActive = true;
          }
        }
        break;
    }
  });
  
  Serial.println("Server avviato! In attesa di client...");
}

void processArduinoMessage(String message) {
  Serial.println("Ricevuto da Arduino: " + message);
  
  if (message.startsWith("COLORS:")) {
    webSocket.broadcastTXT(message);
  }
  else if (message.startsWith("GAME_OVER:")) {
    webSocket.broadcastTXT(message);
    gameActive = false;
  }
  else if (message == "GAME_START") {
    webSocket.broadcastTXT("GAME_START");
    gameActive = true;
  }
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  // Leggi dalla seriale Arduino
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      processArduinoMessage(serialBuffer);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}