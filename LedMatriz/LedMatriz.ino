#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 256
#define DATA_PIN 25
#define WIDTH 16
#define HEIGHT 16

CRGB leds[NUM_LEDS];

const char* ssid = "Celio";
const char* password = "12345678";

WebServer server(80);

int board[3][3];   // Jogo da velha 3x3
int cellSize = 5;  // Cada célula ocupa 5x5 LEDs

int winLine[4];  // startRow, startCol, endRow, endCol da linha vencedora

// Converte coordenadas x,y para índice do LED (zigue-zague)
int XY(int x, int y) {
  if (x % 2 == 0) {
    return x * WIDTH + y;
  } else {
    return x * WIDTH + (WIDTH - 1 - y);
  }
}

// Desenha X 4x4
void drawX(int row, int col) {
  int startX = col * cellSize + 1;
  int startY = row * cellSize + 1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if ((i == j && (i == 1 || i == 2) == false) || (i + j == 3 && (i == 1 || i == 2) == false) || (i == 1 && j == 1) || (i == 1 && j == 2) || (i == 2 && j == 1) || (i == 2 && j == 2)) {
        leds[XY(startX + i, startY + j)] = CRGB::Red;
      }
    }
  }
}

// Desenha O 4x4
void drawO(int row, int col) {
  int startX = col * cellSize + 1;
  int startY = row * cellSize + 1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (i == 0 || i == 3 || j == 0 || j == 3) {
        leds[XY(startX + i, startY + j)] = CRGB::Blue;
      }
    }
  }
}

// Desenha o tabuleiro
void drawBoard() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  for (int i = 1; i < 3; i++) {
    int pos = i * cellSize;
    for (int j = 0; j < WIDTH; j++) {
      leds[XY(pos, j)] = CRGB::White;  // linhas horizontais
      leds[XY(j, pos)] = CRGB::White;  // colunas verticais
    }
  }
}

// Atualiza matriz com X e O
void updateMatrix() {
  drawBoard();  // redesenha fundo e linhas

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      if (board[r][c] == 1) drawX(r, c);
      else if (board[r][c] == 2) drawO(r, c);
    }
  }

  FastLED.show();  // mostra sempre após cada atualização
}

// Verifica vencedor e guarda linha vencedora
int checkWinner() {
  // Linhas
  for (int r = 0; r < 3; r++) {
    if (board[r][0] != 0 && board[r][0] == board[r][1] && board[r][1] == board[r][2]) {
      winLine[0] = r;
      winLine[1] = 0;
      winLine[2] = r;
      winLine[3] = 2;
      return board[r][0];
    }
  }
  // Colunas
  for (int c = 0; c < 3; c++) {
    if (board[0][c] != 0 && board[0][c] == board[1][c] && board[1][c] == board[2][c]) {
      winLine[0] = 0;
      winLine[1] = c;
      winLine[2] = 2;
      winLine[3] = c;
      return board[0][c];
    }
  }
  // Diagonal principal
  if (board[0][0] != 0 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
    winLine[0] = 0;
    winLine[1] = 0;
    winLine[2] = 2;
    winLine[3] = 2;
    return board[0][0];
  }
  // Diagonal secundária
  if (board[0][2] != 0 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
    winLine[0] = 0;
    winLine[1] = 2;
    winLine[2] = 2;
    winLine[3] = 0;
    return board[0][2];
  }
  // Empate
  bool full = true;
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      if (board[r][c] == 0) full = false;
  if (full) return 3;

  return 0;
}

// Desenha linha verde sobre vitória (centralizada nas células)
void drawWinningLine() {
  int startRow = winLine[0];
  int startCol = winLine[1];
  int endRow = winLine[2];
  int endCol = winLine[3];

  if (startRow == endRow) {
    // Linha horizontal
    int r = startRow * cellSize + cellSize / 2;
    for (int c = startCol * cellSize; c <= endCol * cellSize + cellSize - 1; c++) {
      leds[XY(c, r)] = CRGB::Green;  // inverti x e y
    }
  } else if (startCol == endCol) {
    // Linha vertical
    int c = startCol * cellSize + cellSize / 2;
    for (int r = startRow * cellSize; r <= endRow * cellSize + cellSize - 1; r++) {
      leds[XY(c, r)] = CRGB::Green;  // inverti x e y
    }
  } else if (startRow == startCol && endRow == endCol) {
    // Diagonal principal
    for (int i = 0; i < cellSize * 3; i++)
      leds[XY(i, i)] = CRGB::Green;
  } else if (startRow + startCol == 2 && endRow + endCol == 2) {
    // Diagonal secundária
    for (int i = 0; i < cellSize * 3; i++)
      leds[XY(i, (WIDTH - 1) - i)] = CRGB::Green;
  }

  FastLED.show();
}
// Limpa o tabuleiro
void resetBoard() {
  memset(board, 0, sizeof(board));
  drawBoard();
  FastLED.show();
}

void handlePlay() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");

  if (!(server.hasArg("x") && server.hasArg("y"))) {
    server.send(400, "text/plain", "Faltam parametros x e y");
    return;
  }

  int x = server.arg("x").toInt();
  int y = server.arg("y").toInt();
  String pArg = server.hasArg("player") ? server.arg("player") : "X";
  pArg.toUpperCase();
  int player = (pArg == "O") ? 2 : 1;

  if (x < 0 || x > 2 || y < 0 || y > 2) {
    server.send(400, "text/plain", "Posição inválida");
    return;
  }

  // Verifica se a posição está ocupada
  if (board[x][y] != 0) {
    server.send(400, "text/plain", "Posição ocupada!");
    return;
  }

  // Atualiza tabuleiro
  board[x][y] = player;
  updateMatrix();

  // Verifica se houve vencedor ou empate
  int winner = checkWinner();
  if (winner == 1 || winner == 2) drawWinningLine();  // linha verde se houver vencedor

  // Cria JSON de resposta
  String result = "{";
  if (winner == 0)
    result += "\"status\":\"ongoing\"";
  else if (winner == 1)
    result += "\"status\":\"X venceu\"";
  else if (winner == 2)
    result += "\"status\":\"O venceu\"";
  else if (winner == 3)
    result += "\"status\":\"Empate\"";
  result += "}";

  server.send(200, "application/json", result);
}

void handleReset() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  resetBoard();
  server.send(200, "text/plain", "Tabuleiro limpo!");
}

void handleOff() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  FastLED.show();
  server.send(200, "text/plain", "Matriz apagada!");
}

void handleNotFound() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  if (server.method() == HTTP_OPTIONS) {
    server.send(204);
  } else {
    server.send(404, "text/plain", "Rota não encontrada");
  }
}

// --- Setup e Loop ---
void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado ao WiFi!");
  Serial.print("📶 IP do ESP32: ");
  Serial.println(WiFi.localIP());

  server.on("/play", handlePlay);
  server.on("/reset", handleReset);
  server.on("/off", handleOff);
  server.onNotFound(handleNotFound);

  resetBoard();
  server.begin();
}

void loop() {
  server.handleClient();
}