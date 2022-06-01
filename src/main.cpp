#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(3, 4); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// LA La Sol Fa Fa mi re mi
// A A G F F E D E

#define LED_PIN 10
#define ORANGE 0xFFFFFF  //0xFF4500
#define VIOLET 0xFFFFFF  //0x9400D3
#define CYAN 0xFFFFFF    //0x00FFFF
#define MAGENTA 0xFFFFFF //0xFF1493

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, LED_PIN, NEO_GRB + NEO_KHZ800);
#define LED_XBTN 6
int btn_add[4] = {0, 6, 12, 18};
uint32_t btn_color[4] = {ORANGE, VIOLET, CYAN, MAGENTA};

#define BTN_COUNT 4
#define BTN_TIMEOUT 5000
int btn_pins[BTN_COUNT] = {A0, A1, A2, A3};
int ledPin = 13;
int pressed_btns = 0;
// #define DO 0
enum notes
{
  H,
  C,
  D,
  E,
  F,
  G,
  A,
  B
};

#define MELODY_STEPS 8 //16
#define TEMPO 500
int melody[MELODY_STEPS] = {A, A, F, G, F, F, E, D};            // ,E, E, C, D, E, E, D, C};
int duration[MELODY_STEPS] = {1, 1, 2, 2, 1, 2, 2, 1};          // ,1, 1, 2, 2, 1, 2, 2, 1};
int randomBag[MELODY_STEPS] = {-1, -1, -1, -1, -1, -1, -1, -1}; // ,-1, -1, -1, -1, -1, -1, -1, -1};

enum states
{
  idle,
  game,
  win,
  lost
};

byte gameStep = 0;
byte state = idle;

//ligths
void ligthBtn(uint32_t c, int b)
{
  strip.fill(c, btn_add[b], LED_XBTN);
  strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
void randomize()
{
  int rndCandidate = 0;
  boolean repeated = true;
  for (int i = 0; i < MELODY_STEPS; i++)
  {
    randomBag[i] = -1;
  }

  for (int i = 0; i < MELODY_STEPS; i++)
  {
    repeated = true;
    while (repeated)
    {
      repeated = false;
      rndCandidate = random(MELODY_STEPS);

      for (int j = 0; j < MELODY_STEPS; j++)
      {
        if (randomBag[j] == rndCandidate)
          repeated = true;
      }
    }
    randomBag[i] = rndCandidate;
  }
  for (int i = 0; i < MELODY_STEPS; i++)
  {
    randomBag[i] = randomBag[i] % BTN_COUNT;
    Serial.print(randomBag[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void playMelody(byte steps = 0)
{
  Serial.print("MELODY: ");
  for (byte i = 0; i < steps; i++)
  {
    Serial.print(melody[i]);
    Serial.print(" ");
    if (melody[i] != 0)
    {
      myDFPlayer.playMp3Folder(melody[i]);
      ligthBtn(btn_color[randomBag[i]], randomBag[i]);
    }
    delay(TEMPO / duration[i]);
    ligthBtn(0, randomBag[i]);
    if (i == 3)
      delay(TEMPO * 2); // la magia
  }
  Serial.println(" - ");
}

int readButtons()
{
  int pressed = -1;
  unsigned long readTimeout = millis() + BTN_TIMEOUT;
  while (millis() < readTimeout && pressed == -1)
  {
    for (int i = 0; i < BTN_COUNT; i++)
    {
      if (!digitalRead(btn_pins[i]))
      {
        pressed = i;
        ligthBtn(btn_color[i], i);
        myDFPlayer.playMp3Folder(melody[pressed_btns]);
        pressed_btns++;

        while (!digitalRead(btn_pins[i]))
        {
          delay(300);
        }
        ligthBtn(0, i);

        break;
      }
    }
    if (pressed >= 0)
      return pressed;
  }
  return pressed;
}

boolean doGame()
{
  Serial.println("NEW GAME");
  randomize();

  boolean fail = false;

  while (!fail)
  {
    Serial.print("Step: ");
    Serial.print(gameStep);
    Serial.print(" PRESS: ");
    for (int i = 0; i <= gameStep; i++)
    {
      Serial.print(randomBag[i]);
      Serial.print(" ");
    }
    Serial.println(" - ");

    playMelody(gameStep + 1);
    pressed_btns = 0;

    for (int i = 0; i <= gameStep; i++)
    {
      int btnP = readButtons();
      Serial.print("BUTTON: ");
      Serial.println(btnP);

      if (btnP != randomBag[i])
      {
        fail = true;
        break;
      }
    }

    if (gameStep >= MELODY_STEPS - 1)
      break;

    gameStep++;

    delay(1000);
  }

  if (fail)
  {
    Serial.println("YOU LOSE");
    gameStep = 0;
    return false;
  }
  else
  {
    Serial.println("YOU WON");
    return true;
  }
}

void setup()
{
  randomSeed(analogRead(A0));

  for (int i = 0; i < BTN_COUNT; i++)
  {
    pinMode(btn_pins[i], INPUT_PULLUP);
  }

  Serial.begin(9600);

  strip.begin();
  strip.setBrightness(100);
  strip.show(); // Initialize all pixels to 'off'

 while (true)
  {
    // colorWipe(strip.Color(255, 0, 200), 50); // pink
    colorWipe(strip.Color(255, 255, 255), 50); // pink
    if (!(digitalRead(btn_pins[0]) || digitalRead(btn_pins[1])))
      break;
    // colorWipe(strip.Color(200, 255, 0), 50); // yellow
    colorWipe(strip.Color(0, 0, 0), 50); // pink
  }

  mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial))
  { //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true)
      ;
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

  //----Set volume----
  myDFPlayer.volume(30); //Set volume value (0~30).

  /* sound test
  for (int i = 1; i < 8; i++)
  {
    Serial.print(i);
    myDFPlayer.playMp3Folder(i);
    delay(1000);
    // myDFPlayer.stop();
  }
  */
}

void loop()
{
  while (digitalRead(btn_pins[0]) || digitalRead(btn_pins[1]))
  {
    // colorWipe(strip.Color(255, 0, 200), 50); // pink
    colorWipe(strip.Color(255, 255, 255), 50); // pink
    if (!(digitalRead(btn_pins[0]) || digitalRead(btn_pins[1])))
      break;
    // colorWipe(strip.Color(200, 255, 0), 50); // yellow
    colorWipe(strip.Color(0, 0, 0), 50); // pink
  }
  colorWipe(strip.Color(0, 0, 0), 50); // yellow
  delay(500);

  for (int i = 0; i < BTN_COUNT; i++)
  {
    ligthBtn(btn_color[i], i);
    delay(200);
  }
  delay(500);
  for (int i = 0; i < BTN_COUNT; i++)
  {
    ligthBtn(0, i);
    delay(200);
  }
  delay(1000);

  if (doGame())
  {
    //WON
    colorWipe(strip.Color(255, 255, 255), 50); // Green
    delay(500);
    strip.fill(0, 0, 24);
    strip.show();
    playMelody(MELODY_STEPS);
  }
  else
  {
    //Lose
    // colorWipe(strip.Color(255, 255, 255), 50); // Red
    strip.fill(0xFFFFFF, 0, 24);
    strip.show();
    delay(250);
    strip.fill(0, 0, 24);
    strip.show();
    delay(250);
    strip.fill(0xFFFFFF, 0, 24);
    strip.show();
    delay(250);
    strip.fill(0, 0, 24);
    strip.show();
    delay(250);
    strip.fill(0xFFFFFF, 0, 24);
    strip.show();
    delay(250);
    strip.fill(0, 0, 24);
    strip.show();
    delay(250);
  }
  delay(1000);
}