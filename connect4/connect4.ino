#include <Adafruit_NeoPixel.h>
// Connect-4 Board
// empty: 0
// player one: 1
// player two: 2
const int rows = 6;
const int cols = 7;
int board[rows][cols] = {0}; // initialize empty board

// setup inputs
const int buttonPins[cols] = {6, 7, 8, 9, 10, 11, 12};
const int colorSwitchPin = 5;
// brightness on a scale from 0-255
const int brightness = 60;

// time between clicking and releasing button
int initialTime;
int finalTime;

// color schemes: 0 = orange+blue, 1 = green+magenta, 2 = yellow+purple

int colorScheme = 0;


int player = 1; // start with player one
bool gameOver = false; // continue game until true
bool singlePlayer = false; // chosen on the mode-select screen (false = 2 players)

// setup addressible leds
const int numLEDs = rows*cols;
const int ledPin = 3;
Adafruit_NeoPixel leds(
  numLEDs, ledPin, NEO_GRB + NEO_KHZ800
);


// player colors
// Call with player #, returns with their color
uint32_t playerColor(int p) {
  if (p == 0) {
    return leds.Color(0, 0, 0);
  }
  if (p == 3) {
    return leds.Color(255, 255, 255);
  }

  switch (colorScheme) {
    case 0: // orange + blue
      return (p == 1)
      ? leds.Color(255, 100, 0)
      : leds.Color(0, 0, 255);

    case 1: // green + magenta
      return (p == 1)
      ? leds.Color(0, 255, 0)
      : leds.Color(255, 0, 255);

    case 2: // yellow + purple
      return (p == 1)
      ? leds.Color(255, 200, 0)
      : leds.Color(128, 0, 255);
  }

  return leds.Color(0, 0, 0);
}

// returns led location based on wiring
// currently starts bottom left and snakes up to down right
int ledIndex(int row, int col) {
  if (col % 2 == 0) {
    // even columns: bottom to top
    return col * rows + (rows - 1 - row);
  }
  else {
    // odd columns: top to bottom
    return col * rows + row;
  }
}

// symbols shown on the empty board for mode select (1 = light on, 0 = off)
const uint8_t symbolOne[rows][cols] = {
  {0, 0, 0, 1, 0, 0, 0},
  {0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 1, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0}
};

const uint8_t symbolTwo[rows][cols] = {
  {0, 0, 1, 1, 1, 0, 0},
  {0, 1, 0, 0, 0, 1, 0},
  {0, 0, 0, 0, 1, 0, 0},
  {0, 0, 0, 1, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 0}
};

// true if no pieces have been placed (checking the bottom row is enough,
// since pieces always fall to the bottom)
bool boardEmpty() {
  for (int c = 0; c < cols; c++) {
    if (board[rows - 1][c] != 0) return false;
  }
  return true;
}

// lights up the board with board state for the turn
// when the board is empty, shows the mode-select symbol instead
void drawBoard() {
  bool selecting = boardEmpty();
  int modePlayer = singlePlayer ? 1 : 2; // symbol is drawn in that player's color

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      uint32_t color;
      if (selecting) {
        bool on = singlePlayer ? symbolOne[r][c] : symbolTwo[r][c];
        color = on ? playerColor(modePlayer) : playerColor(0);
      } else {
        color = playerColor(board[r][c]);
      }
      leds.setPixelColor(ledIndex(r, c), color);
    }
  }
  leds.show();
}




// updates board with piece placement and returns row number, or -1 if full
int placePiece(int col) {
  for(int r = rows-1; r >= 0; r--) {
    if(board[r][col] == 0) {
      board[r][col] = player; // "places" the piece
      return r; // returns the row of the piece
    }
  }
  return -1;
}

// checks all the axes of the last placed piece
bool checkBoard(int row, int col) {
  int p = board[row][col];
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Serial.print(board[i][j]);
      Serial.print(" "); // Tab spacing for readability
    }
    Serial.println(); // New line after each row
  }
  Serial.println();

  // check horizontal
  int count = 1;

  for(int c = col-1; c >= 0 && board[row][c] == p; c--) {
    count++; // add-up left side
  }
  for(int c = col+1; c < cols && board[row][c] == p; c++) {
    count++; // add-up right side
  }
  
  if(count >= 4) {
    Serial.println("horizontal");
    return true; // returns true if there is a 4-in-a-row for the horizontal axis
  }

  // check vertical
  count = 1;

  for(int r = row-1; r >= 0 && board[r][col] == p; r--) {
    count++; // add-up above
  }
  for(int r = row+1; r < rows && board[r][col] == p; r++) {
    count++; // add-up below
  }
  
  if(count >= 4) {
    Serial.println("vertical");
    return true; // returns true if there is a 4-in-a-row for the vertical axis
  }

  // check positive diagonal
  count = 1;

  for(int r = row-1, c = col+1; r >= 0 && c < cols && board[r][c] == p; r--, c++) {
    count++; // add-up upper right diagonal
  }
  for(int r = row+1, c = col-1; r < rows && c >= 0 && board[r][c] == p; r++, c--) {
    count++; // add-up lower left diagonal
  }
  
  if(count >= 4) {
    Serial.println("positive diagonal");
    return true; // returns true if there is a 4-in-a-row for the positive diagonal
  }

  // check negative diagonal
  count = 1;

  for(int r = row-1, c = col-1; r >= 0 && c >= 0 && board[r][c] == p; r--, c--) {
    count++; // add-up upper left diagonal
  }
  for(int r = row+1, c = col+1; r < rows && c < cols && board[r][c] == p; r++, c++) {
    count++; // add-up lower right diagonal
  }
  
  if(count >= 4) {
    Serial.println("negative diagonal");
    return true; // returns true if there is a 4-in-a-row for the negative diagonal
  }

  return false; // returns false if there is no 4-in-a-row
}

// returns true if no space left on board
bool boardFull() {
  for (int c = 0; c < cols; c++) {
    if (board[0][c] == 0) return false;
  }
  return true;
}

// resets game board
// (no longer calls setup(); loop() starts the next game via playGame())
void resetGame() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      board[r][c] = 0;
    }
  }

  player = 1;
  gameOver = false;
  drawBoard();
}

void endGame(int p) {
  // flashes lights in color of winner
  for(int i = 0; i < 6; i++) {
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        leds.setPixelColor(ledIndex(r, c), playerColor(p));
      }
    }
    leds.show();
    delay(500);
    drawBoard();
    delay(500);
  }
}

void turnCheck(int p){
  for(int i = 0; i < rows; i++){
    for(int r = 0; r < cols; r++){
      leds.setPixelColor(ledIndex(i, r), playerColor(p));
    }
  }
    leds.show();
    delay(1000);
    drawBoard();
}

// ---------------- Single-player / minimax ----------------
const int AI_PLAYER = 2;
const int HUMAN_PLAYER = 1;
const int SEARCH_DEPTH = 4;         // plies to look ahead; see timing print in aiTakeTurn()
const int WIN_SCORE = 10000;
const int INF_SCORE = 30000;

// search center columns first (helps tie-breaking now, and pruning later)
const int colOrder[cols] = {3, 2, 4, 1, 5, 0, 6};

// directions to check for 4-in-a-row: horizontal, vertical, two diagonals
const int8_t dirs[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

// drops a piece for player p into col; returns row, or -1 if column is full
int dropPiece(int col, int p) {
  for (int r = rows - 1; r >= 0; r--) {
    if (board[r][col] == 0) {
      board[r][col] = p;
      return r;
    }
  }
  return -1;
}

// silent win check for the piece at (row, col) -- no Serial output,
// so it's fast enough to call thousands of times during the search
bool winningPiece(int row, int col) {
  int p = board[row][col];

  for (int d = 0; d < 4; d++) {
    int count = 1;
    for (int s = -1; s <= 1; s += 2) { // walk both directions along the axis
      int dr = s * dirs[d][0];
      int dc = s * dirs[d][1];
      int r = row + dr;
      int c = col + dc;
      while (r >= 0 && r < rows && c >= 0 && c < cols && board[r][c] == p) {
        count++;
        r += dr;
        c += dc;
      }
    }
    if (count >= 4) return true;
  }
  return false;
}

// cheap heuristic for non-terminal leaves: favor the center column
int evaluateBoard() {
  int score = 0;
  for (int r = 0; r < rows; r++) {
    if (board[r][3] == AI_PLAYER) score += 3;
    else if (board[r][3] == HUMAN_PLAYER) score -= 3;
  }
  return score;
}

// plain minimax. AI maximizes, human minimizes.
int minimax(int depth, bool maximizing) {
  if (depth == 0) return evaluateBoard();

  int p = maximizing ? AI_PLAYER : HUMAN_PLAYER;
  int best = maximizing ? -INF_SCORE : INF_SCORE;
  bool moved = false;

  for (int i = 0; i < cols; i++) {
    int c = colOrder[i];
    int r = dropPiece(c, p);
    if (r == -1) continue; // column full

    moved = true;
    int score;
    if (winningPiece(r, c)) {
      // adding depth makes faster wins score higher (and slower losses better)
      score = maximizing ? (WIN_SCORE + depth) : -(WIN_SCORE + depth);
    } else {
      score = minimax(depth - 1, !maximizing);
    }
    board[r][c] = 0; // undo

    if (maximizing) best = max(best, score);
    else best = min(best, score);
  }

  if (!moved) return 0; // no legal moves = draw
  return best;
}

// returns the column the AI wants to play, or -1 if the board is full
int getBestMove() {
  int bestCol = -1;
  int bestScore = -INF_SCORE;

  for (int i = 0; i < cols; i++) {
    int c = colOrder[i];
    int r = dropPiece(c, AI_PLAYER);
    if (r == -1) continue;

    int score;
    if (winningPiece(r, c)) score = WIN_SCORE + SEARCH_DEPTH;
    else score = minimax(SEARCH_DEPTH - 1, false);
    board[r][c] = 0;

    if (score > bestScore) { // strict > keeps the earlier (more central) column on ties
      bestScore = score;
      bestCol = c;
    }
  }
  return bestCol;
}

// handles win/draw/turn-swap after any move (human or AI)
// returns true if the game ended
bool afterMove(int row, int col) {
  drawBoard();

  if (checkBoard(row, col)) {
    gameOver = true;
    endGame(player);
    resetGame();
    return true;
  }
  if (boardFull()) {
    gameOver = true;
    endGame(3);
    resetGame();
    return true;
  }
  player = (player == 1) ? 2 : 1;
  return false;
}

// the AI's whole turn; returns true if the game ended
bool aiTakeTurn() {
  unsigned long t = millis();
  int col = getBestMove();
  Serial.print("AI think time (ms): ");
  Serial.println(millis() - t);

  if (col == -1) return false;
  int row = placePiece(col); // player == AI_PLAYER at this point
  return afterMove(row, col);
}

// sets up buttons and leds
void setup() {
  for (int c = 0; c < cols; c++) {
    pinMode(buttonPins[c], INPUT_PULLUP);
  }

  pinMode(colorSwitchPin, INPUT_PULLUP);

  Serial.begin(9600);

  leds.begin();
  leds.setBrightness(brightness);
  leds.clear();
  leds.show();

  drawBoard();
  // playGame() is now called from loop()
}

void playGame() {
  while (!gameOver) {

    // ---- Color button ----
    if (digitalRead(colorSwitchPin) == LOW) {
      delay(25); // debounce

      if (digitalRead(colorSwitchPin) == LOW) {
        unsigned long pressTime = millis();
        bool flashed = false;

        // Wait for release; start the flash only once the hold reaches 500 ms
        while (digitalRead(colorSwitchPin) == LOW) {
          if (!flashed && millis() - pressTime >= 500) {
            turnCheck(player);   // flash the board in the current player's color
            flashed = true;
          }
          delay(5);
        }

        if (!flashed) {
          // Released in under 500 ms
          if (boardEmpty()) {
            singlePlayer = !singlePlayer; // mode-select screen: toggle 1/2 players
          } else {
            colorScheme = (colorScheme + 1) % 3; // mid-game: switch color scheme
          }
        }

        // Restore the board (clears the flash / shows the new scheme)
        drawBoard();

        delay(25); // release debounce
      }
    }

    // ---- Column buttons ----
    for (int c = 0; c < cols; c++) {
      if (digitalRead(buttonPins[c]) == LOW) {
        delay(25); // debounce

        if (digitalRead(buttonPins[c]) == LOW) {
          int row = placePiece(c);

          if (row != -1) {
            if (afterMove(row, c)) return;

            if (singlePlayer && player == AI_PLAYER) {
              if (aiTakeTurn()) return;
            }
          }

          // wait for button release
          while (digitalRead(buttonPins[c]) == LOW) {
            delay(5);
          }

          delay(25);
        }
      }
    }
  }
}

void loop() {
  playGame(); // returns after each finished game; loop() starts the next one
}
