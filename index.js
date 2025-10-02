const tentativi = 10;
const numeri = 4;

const state = {
    soluzione: '1234',
    grid: Array(tentativi).fill().map(() => Array(numeri).fill('')),
    currentRow: 0,
    currentCol: 0,
};

// ---- Funzioni principali ----
function updateGrid() {
    for (let row = 0; row < state.grid.length; row++) {
        for (let col = 0; col < state.grid[row].length; col++) {
            const box = document.getElementById(`box-${row}-${col}`);
            box.textContent = state.grid[row][col];
        }
    }
}
//funzione disegno di una singola box
function drawBox(container, row, col, number = '') {
    const box = document.createElement('div');
    box.className = 'box';
    box.id = `box-${row}-${col}`;
    box.textContent = number;
    container.appendChild(box);
}
//funzione creazione griglia completa
function createGrid(container) {
    const grid = document.createElement('div');
    grid.className = 'grid';
    grid.style.gridTemplateRows = `repeat(${tentativi}, auto)`;
    grid.style.gridTemplateColumns = `repeat(${numeri}, auto)`;

    for (let row = 0; row < tentativi; row++) {
        for (let col = 0; col < numeri; col++) {
            drawBox(grid, row, col);
        }
    }

    container.appendChild(grid);
}

// ---- Input numerico ----
function handleKeyPress() {
    document.body.onkeydown = (e) => {
        const key = e.key;

        if (key === 'Backspace') removeNumber();
        else if (key === 'Enter') {
            if (state.currentCol === numeri) {
                const number = getCurrentNumber();
                revealNumber(number);
                state.currentRow++;
                state.currentCol = 0;
            }
        } else if (isNumber(key)) addNumber(key);

        updateGrid();
    };
}

// ---- Logica griglia ----
function getCurrentNumber() {
    return state.grid[state.currentRow].join(''); //ritorna il numero corrente come stringa
}
// ----- Funzione di rivelazione del numero -----
function revealNumber(number) {
    const row = state.currentRow;
    const animationDuration = 500; // ms

    //prendo ogni box della riga corrente
    for (let col = 0; col < numeri; col++) {
        const box = document.getElementById(`box-${row}-${col}`);
        const digit = state.grid[row][col];


        // aggiungo la classe animated con delay progressivo
        box.style.animationDelay = `${col * (animationDuration / 1250)}s`;
        box.classList.add('animated');

        // cambia colore **a metà animazione**
        setTimeout(() => {
            if (digit === state.soluzione[col]) box.classList.add('right');
            else if (state.soluzione.includes(digit)) box.classList.add('wrong');
            else box.classList.add('empty');
        }, col * animationDuration); 
        // delay iniziale + metà animazione
    }

    // controllo vincita/fine gioco
    const guess = getCurrentNumber();
    const isWinner = guess === state.soluzione;
    const isGameOver = state.currentRow === tentativi - 1;

    setTimeout(() => {
        if (isWinner) alert('Hai vinto!');
        else if (isGameOver) alert(`Hai perso! La soluzione era ${state.soluzione}`);
    }, numeri * animationDuration); // dopo che tutte le caselle hanno finito l'animazione
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

// ---- Avvio ----
function startApp() {
    const game = document.getElementById('game');
    createGrid(game);
    handleKeyPress();
    updateGrid();
}

startApp();
