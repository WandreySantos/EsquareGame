#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>

#define NUM_LEDS 256
#define DATA_PIN 25

CRGB leds[NUM_LEDS];

const char *ssid = "Celio";
const char *password = "12345678";

WebServer server(80);

// Exemplo: ligar todos os LEDs de vermelho
void handleRed()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  server.send(200, "text/plain", "Matriz vermelha!");
}

// Exemplo: apagar LEDs
void handleOff()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  server.send(200, "text/plain", "Matriz apagada!");
}

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");
  Serial.println(WiFi.localIP());

  server.on("/red", handleRed);
  server.on("/off", handleOff);

  server.begin();
}

void loop()
{
  server.handleClient();
}
