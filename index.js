const tentativi = 6;
        const numeri = 3;

        // Stato di gioco
        const state = {
            grid: Array(tentativi).fill().map(() => Array(numeri).fill('')),
            currentRow: 0,
            currentCol: 0,
            gameOver: false
        };
        
        // Gestione WebSocket
        let websocket;

        // Inizializza WebSocket e gestisce eventi 
        function initWebSocket() {
            const host = window.location.hostname; // Ottiene l'host corrente
            websocket = new WebSocket('ws://' + host + ':81/'); // Porta 81 per WebSocket

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

            // Gestione messaggi in arrivo
            websocket.onmessage = function(event) {
                console.log("Ricevuto:", event.data); // Debug
                
                // Gestione messaggi specifici
                if (event.data.startsWith("COLORS:")) { //Gestione messaggio COLORS per colorare le caselle
                  // Trim globale per rimuovere \r\n finali
                  const colorString = event.data.substring(7).trim();
                  // rimuove spazi e CR/LF da ogni elemento
                  const colors = colorString.split(',').map(c => c.trim());
                  console.log("Parsed COLORS:", colors);
                  applyColorsToCurrentRow(colors);
                }
                else if (event.data.startsWith("GAME_OVER:")) { // Gestione messaggio GAME_OVER per capire vittoria/sconfitta
                    const result = event.data.substring(10).trim();
                    handleGameOver(result);
                }
                else if (event.data === "GAME_START") {// Gestione inizio nuova partita per resettare lo stato e sinctronizzare esp32 e arduino
                    resetGame();
                    updateStatus("Nuova partita - Inserisci " + numeri + " cifre");
                }
                
                else {
                    updateStatus(event.data);
                }
            };
        }

        // Gestione fine partita
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

        // Applica i colori alle caselle della riga corrente con animazioni
        function applyColorsToCurrentRow(colors) {
        const row = state.currentRow;
        const animationDuration = 500; // ms

        for (let col = 0; col < numeri; col++) {
            const box = document.getElementById('box-' + row + '-' + col);
            // pulisci e normalizza il colore
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
            return state.grid[state.currentRow].join('');// Ottiene la stringa delle cifre inserite nella riga corrente
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
            updateStatus("Nuova partita - Inserisci " + numeri + " cifre");
            
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
