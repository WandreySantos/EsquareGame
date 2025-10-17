#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

#define DATA_PIN 25
#define WIDTH 16
#define HEIGHT 16
#define NUM_LEDS (WIDTH * HEIGHT)

CRGB leds[NUM_LEDS];
WebServer server(80);

// -------------------- VARIÁVEIS DO JOGO --------------------
struct Point {
  int x;
  int y;
};
Point snake[256];
int snakeLength = 3;
String direction = "RIGHT";
String lastDirection = "RIGHT";
Point food;
int score = 0;
int gameSpeed = 150;  // velocidade padrão
String currentDifficulty = "medium";
bool start_game = true;


const char* ssid = "Celio";
const char* password = "12345678";

// -------------------- MAPA LED --------------------
int XY(int x, int y) {
  if (x % 2 == 0) return x * HEIGHT + y;
  else return x * HEIGHT + (HEIGHT - 1 - y);
}

// -------------------- COMIDA --------------------
void spawnFood() {
  bool valid = false;
  while (!valid) {
    food.x = random(0, WIDTH);
    food.y = random(0, HEIGHT);
    valid = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        valid = false;
        break;
      }
    }
  }
}

// -------------------- CONEXÃO --------------------
void connectWiFi() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando-se ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
}

// -------------------- PÁGINA WEB --------------------
void handleRoot() {
  String page = R"rawliteral(
      <html>
      <meta charset="UTF-8" />
      <meta name="viewport" content="width=device-width, initial-scale=1.0" />
      <title>Snake Game</title>
      <style>
  body {
    background-color: #000;
    color: #0f0;
    text-align: center;
    font-family: 'Courier New', monospace;
    margin: 0;
    padding: 20px;
  }

  h1 {
    font-size: 2.5em;
    margin-bottom: 10px;
    text-shadow: 0 0 5px #0f0;
  }

  #score {
    font-size: 1.5em;
    margin: 10px 0 20px 0;
    text-shadow: 0 0 5px #0f0;
  }

  .controls {
    display: flex;
    flex-direction: column;
    align-items: center;
    margin-bottom: 20px;
  }

  .direction-buttons {
    display: grid;
    grid-template-columns: repeat(3, 70px);
    grid-template-rows: repeat(3, 70px);
    gap: 10px;
    justify-content: center;
    align-items: center;
  }

  .direction-buttons button {
    background: #111;
    color: #0f0;
    border: 2px solid #0f0;
    border-radius: 12px;
    font-size: 24px;
    cursor: pointer;
    transition: all 0.2s ease-in-out;
    box-shadow: 0 0 5px #0f0;
  }

  .direction-buttons button:hover {
    background: #0f0;
    color: #000;
    transform: scale(1.1);
    box-shadow: 0 0 10px #0f0;
  }

  .difficulty-buttons {
    margin-top: 15px;
    display: flex;
    gap: 15px;
    justify-content: center;
  }

  .difficulty-buttons button {
    background: #111;
    color: #0f0;
    border: 2px solid #0f0;
    border-radius: 12px;
    font-size: 16px;
    padding: 8px 15px;
    cursor: pointer;
    transition: all 0.2s ease-in-out;
    box-shadow: 0 0 5px #0f0;
  }

  .difficulty-buttons button:hover {
    background: #0f0;
    color: #000;
    transform: scale(1.05);
    box-shadow: 0 0 10px #0f0;
  }

  canvas {
    margin-top: 30px;
    border: 3px solid #0f0;
    box-shadow: 0 0 20px #0f0;
    border-radius: 8px;
  }
</style>

      <body>
      <h1>Snake Game</h1>
      <p id="score">Score: 0</p>
      <div class="controls">
        <div>
          <button id="up" class="btn">↑</button>
          <button id="left" class="btn">←</button>
          <button id="down" class="btn">↓</button>
          <button id="right" class="btn">→</button>
        </div>
        <p>Dificuldade:</p>
        <button onclick="setDifficulty('easy')">Fácil</button>
        <button onclick="setDifficulty('medium')">Médio</button>
        <button onclick="setDifficulty('hard')">Difícil</button>
      </div>
      <canvas id="matrix" width="320" height="320"></canvas>
      <script>
      const canvas = document.getElementById('matrix');
      const ctx = canvas.getContext('2d');
      const PIXEL_SIZE = 20;

      function sendDir(d) { fetch('/dir?move=' + d); setActive(d); }
      function setActive(d) { ['up','down','left','right'].forEach(id => document.getElementById(id).classList.remove('active')); document.getElementById(d).classList.add('active'); }
      function setDifficulty(level){ fetch('/difficulty?level=' + level); }

      document.querySelectorAll('.btn').forEach(b => { b.addEventListener('click', () => sendDir(b.id)); });
      document.addEventListener('keydown', e => { const map = {ArrowUp:'up', ArrowDown:'down', ArrowLeft:'left', ArrowRight:'right'}; if(map[e.key]) sendDir(map[e.key]); });

      async function updateMatrix() {
        try {
          const res = await fetch('/matrix');
          const data = await res.json();
          for (let y=0;y<data.matrix.length;y++){
            for(let x=0;x<data.matrix[y].length;x++){
              const [r,g,b]=data.matrix[y][x];
              ctx.fillStyle=`rgb(${r},${g},${b})`;
              ctx.fillRect(x*PIXEL_SIZE,y*PIXEL_SIZE,PIXEL_SIZE,PIXEL_SIZE);
            }
          }
          document.getElementById('score').innerText='Score: '+data.score;

          ['easy','medium','hard'].forEach(lvl=>{
            const btn = document.querySelector("button[onclick*='"+lvl+"']");
            if(btn) btn.style.background = (data.difficulty===lvl) ? '#0f0' : '#111';
            if(btn) btn.style.color = (data.difficulty===lvl) ? '#000' : '#0f0';
          });
        } catch(e){ console.log('Erro ao atualizar matriz:',e); }
        requestAnimationFrame(updateMatrix);
      }
      updateMatrix();
      </script>
      </body>
      </html>
      )rawliteral";

  server.send(200, "text/html", page);
}

// -------------------- ROTAS --------------------
void handleDirection() {
  if (server.hasArg("move")) {
    String newDir = server.arg("move");
    if (!((lastDirection == "up" && newDir == "down") || (lastDirection == "down" && newDir == "up") || (lastDirection == "left" && newDir == "right") || (lastDirection == "right" && newDir == "left"))) {
      direction = newDir;
      lastDirection = direction;
    }
  }
  server.send(200, "text/plain", "ok");
}

void handleDifficulty() {
  if (server.hasArg("level")) {
    String level = server.arg("level");
    currentDifficulty = level;
    if (level == "easy") gameSpeed = 250;
    else if (level == "medium") gameSpeed = 150;
    else if (level == "hard") gameSpeed = 75;
  }
  server.send(200, "text/plain", "ok");
}

void handleMatrix() {
  String json = "{\"matrix\":[";
  for (int y = 0; y < HEIGHT; y++) {
    json += "[";
    for (int x = 0; x < WIDTH; x++) {
      CRGB c = leds[XY(x, y)];
      json += "[" + String(c.r) + "," + String(c.g) + "," + String(c.b) + "]";
      if (x < WIDTH - 1) json += ",";
    }
    json += "]";
    if (y < HEIGHT - 1) json += ",";
  }
  json += "],\"score\":" + String(score) + ",\"difficulty\":\"" + currentDifficulty + "\"}";
  server.send(200, "application/json", json);
}

// -------------------- MOVIMENTO --------------------
void moveSnake() {
  for (int i = snakeLength - 1; i > 0; i--) snake[i] = snake[i - 1];
  if (direction == "up") snake[0].y--;
  else if (direction == "down") snake[0].y++;
  else if (direction == "left") snake[0].x--;
  else if (direction == "right") snake[0].x++;

  if (snake[0].x < 0) snake[0].x = WIDTH - 1;
  if (snake[0].x >= WIDTH) snake[0].x = 0;
  if (snake[0].y < 0) snake[0].y = HEIGHT - 1;
  if (snake[0].y >= HEIGHT) snake[0].y = 0;

  if (snake[0].x == food.x && snake[0].y == food.y) {
    snakeLength++;
    score++;
    spawnFood();
  }
}

// -------------------- CHECAGEM DE COLISÃO --------------------
bool checkCollision() {
  for (int i = 1; i < snakeLength; i++)
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) return true;
  return false;
}

// -------------------- DESENHO --------------------
void drawSnake() {
  FastLED.clear();
  leds[XY(food.x, food.y)] = CRGB::Red;
  for (int i = 0; i < snakeLength; i++)
    leds[XY(snake[i].x, snake[i].y)] = (i == 0) ? CRGB::White : CRGB(0, 100, 0);
  FastLED.show();
}

// -------------------- NÚMEROS SIMPLES (5x3) --------------------
const byte digits[10][5] = {
  { 0b111, 0b101, 0b101, 0b101, 0b111 },
  { 0b010, 0b110, 0b010, 0b010, 0b111 },
  { 0b111, 0b001, 0b111, 0b100, 0b111 },
  { 0b111, 0b001, 0b111, 0b001, 0b111 },
  { 0b101, 0b101, 0b111, 0b001, 0b001 },
  { 0b111, 0b100, 0b111, 0b001, 0b111 },
  { 0b111, 0b100, 0b111, 0b101, 0b111 },
  { 0b111, 0b001, 0b010, 0b100, 0b100 },
  { 0b111, 0b101, 0b111, 0b101, 0b111 },
  { 0b111, 0b101, 0b111, 0b001, 0b111 }
};

// -------------------- FUNÇÃO PARA DESENHAR NÚMEROS --------------------
void drawNumber(int num) {
  String s = String(num);
  int len = s.length();
  int startX = WIDTH / 2 - len * 4 / 2;
  int startY = HEIGHT / 2 - 5 / 2;
  for (int n = 0; n < len; n++) {
    int d = s[n] - '0';
    for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 3; col++) {
        if (digits[d][row] & (1 << (2 - col))) {
          int x = startX + n * 4 + col;
          int y = startY + row;
          if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) leds[XY(x, y)] = CRGB::Yellow;
        }
      }
    }
  }
  FastLED.show();
  delay(1000);
}

// -------------------- ANIMAÇÃO DE MORTE --------------------
void deathAnimation() {
  for (int i = snakeLength - 1; i >= 0; i--) {
    leds[XY(snake[i].x, snake[i].y)] = CRGB::Red;
    FastLED.show();
    delay(100);
    leds[XY(snake[i].x, snake[i].y)] = CRGB::Black;
  }
  FastLED.clear();
  drawNumber(score);
  start_game = true;
}

// -------------------- SETUP & LOOP --------------------
void setup() {
  connectWiFi();

  snake[0] = { 8, 8 };
  snake[1] = { 7, 8 };
  snake[2] = { 6, 8 };

  spawnFood();

  server.on("/", handleRoot);
  server.on("/dir", handleDirection);
  server.on("/matrix", handleMatrix);
  server.on("/difficulty", handleDifficulty);
  server.begin();
}

void loop() {
  server.handleClient();

  static unsigned long lastMove = 0;
  unsigned long now = millis();

  if (now - lastMove > gameSpeed) {
    moveSnake();

    if (start_game && checkCollision()) {
      start_game = false;
      deathAnimation();
      // reinicia jogo
      snakeLength = 3;
      score = 0;
      snake[0] = { 8, 8 };
      snake[1] = { 7, 8 };
      snake[2] = { 6, 8 };
      direction = "RIGHT";
      lastDirection = "RIGHT";
      spawnFood();
    }

    drawSnake();
    lastMove = now;
  }
}
