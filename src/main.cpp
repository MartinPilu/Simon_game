#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <pitches.h>
// #include "SoftwareSerial.h"
// #include "DFRobotDFPlayerMini.h"

// SoftwareSerial mySoftwareSerial(3, 4); // RX, TX
// DFRobotDFPlayerMini myDFPlayer;

// LA La Sol Fa Fa mi re mi
// A A G F F E D E

#define LED_PIN 2
#define LED_COUNT 87
#define ORANGE 0xFFFFFF  // 0xFF4500
#define VIOLET 0xFFFFFF  // 0x9400D3
#define CYAN 0xFFFFFF    // 0x00FFFF
#define MAGENTA 0xFFFFFF // 0xFF1493
#define LED_XBTN 6
int btn_add[4] = {0, 6, 12, 18};
uint32_t btn_color[4] = {ORANGE, VIOLET, CYAN, MAGENTA};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_BGR + NEO_KHZ800);

#define BTN_COUNT 4
#define BTN_TIMEOUT 5000
int btn_ligth_pins[BTN_COUNT] = {3, 5, 6, 7};
int btn_pins[BTN_COUNT] = {A0, A1, A2, A3};
int btn_notes[BTN_COUNT] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_A4};
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

// song method
#define MELODY_STEPS 10 // 16
#define TEMPO 500
int melody[MELODY_STEPS] = {A, A, F, G, F, F, E, D, A, A};              // ,E, E, C, D, E, E, D, C};
int duration[MELODY_STEPS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};            // ,1, 1, 2, 2, 1, 2, 2, 1};
int randomBag[MELODY_STEPS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // ,-1, -1, -1, -1, -1, -1, -1, -1};

// int melody[MELODY_STEPS] = {A, A};      //, F, G, F, F, E, D, A, A};              // ,E, E, C, D, E, E, D, C};
// int duration[MELODY_STEPS] = {1, 1};    //, 1, 1, 1, 1, 1, 1, 1, 1};            // ,1, 1, 2, 2, 1, 2, 2, 1};
// int randomBag[MELODY_STEPS] = {-1, -1}; //, -1, -1, -1, -1, -1, -1, -1, -1}; // ,-1, -1, -1, -1, -1, -1, -1, -1};

enum states
{
  idle,
  game,
  win,
  lost
};

byte gameStep = 0;
byte state = idle;

// ligths
void ligthBtn(uint32_t c, int b, int t = 0)
{
  for (int i = 0; i < BTN_COUNT; i++)
  {
    digitalWrite(btn_ligth_pins[i], LOW);
    noTone(4);
  }

  if (c > 0)
  {
    digitalWrite(btn_ligth_pins[b], HIGH);
    if (t > 0)
    {
      tone(4, btn_notes[b], t);
    }
    else
    {
      tone(4, btn_notes[b]);
    }
  }

  // strip.fill(c, btn_add[b], LED_XBTN);
  // strip.show();
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

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait)
{
  for (int a = 0; a < 10; a++)
  { // Repeat 10 times...
    for (int b = 0; b < 3; b++)
    {                //  'b' counts from 0 to 2...
      strip.clear(); //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < strip.numPixels(); c += 3)
      {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void randomize_bag()
{
  int r = analogRead(A7);
  randomSeed(r);
  Serial.print("SEED: ");
  Serial.println(r);

  int rndCandidate = 0;
  boolean repeated = true;
  // for (int i = 0; i < MELODY_STEPS; i++)
  // {
  //   randomBag[i] = -1;
  // }

  for (int i = 0; i < MELODY_STEPS; i++)
  {
    // repeated = true;
    // while (repeated)
    // {
    // repeated = false;
    // rndCandidate = random(BTN_COUNT);

    //   for (int j = 0; j < MELODY_STEPS; j++)
    //   {
    //     if (randomBag[j] == rndCandidate)
    //       repeated = true;
    //   }
    // }
    randomBag[i] = random(BTN_COUNT);
  }
  for (int i = 0; i < MELODY_STEPS; i++)
  {
    // randomBag[i] = randomBag[i] % BTN_COUNT;
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
      // myDFPlayer.playMp3Folder(melody[i]);
      ligthBtn(btn_color[randomBag[i]], randomBag[i], TEMPO / duration[i]);
    }
    delay(TEMPO / 2);
    ligthBtn(0, randomBag[i]);
    delay(TEMPO / 2);
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
      if (digitalRead(btn_pins[i]))
      {
        pressed = i;
        ligthBtn(btn_color[i], i);
        // myDFPlayer.playMp3Folder(melody[pressed_btns]);

        pressed_btns++;

        while (digitalRead(btn_pins[i]))
        {
          delay(10);
        }
        delay(20);

        ligthBtn(0, i);
        break;
      }
    }

    if (pressed >= 0)
      break;
  }
  return pressed;
}

boolean doGame()
{
  Serial.println("NEW GAME");
  randomize_bag();

  boolean fail = false;
  gameStep = 0;
  while (!fail)
  {
    // vas a sufrir asi:
    Serial.print("Step: ");
    Serial.print(gameStep);
    Serial.print(" PRESS: ");
    for (int i = 0; i <= gameStep; i++)
    {
      Serial.print(randomBag[i]);
      Serial.print(" ");
    }
    Serial.println(" - ");

    // buena suerte!
    playMelody(gameStep + 1);
    pressed_btns = 0;

    // vamos a ver si te sabes la secuencia paso a paso.
    for (int i = 0; i <= gameStep; i++)
    {
      // apretate un boton y vemos como te va.
      int btnP = readButtons();
      Serial.print("BUTTON: ");
      Serial.println(btnP);

      if (btnP != randomBag[i]) // no le pegaste?
      {
        fail = true; // Cagaste!!
        for(int j = 0; j < 4; j++){
          ligthBtn(1,randomBag[i]); // era este gil!
          delay(250);
          ligthBtn(1,btnP);
          delay(250);
        }
        break;
      }
    }

    // Zafaste
    //  a ver si ya sufriste suficiente?
    if (gameStep >= MELODY_STEPS - 1)
      break; // chaoo te escapas del sufrimetno !!

    gameStep++; // Banca un segundo y vamos un paso mas hasta que falles.
    delay(1000);
  }

  // y.. fallaste?
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
  randomSeed(analogRead(A7));

  for (int i = 0; i < BTN_COUNT; i++)
  {
    pinMode(btn_pins[i], INPUT_PULLUP);
    pinMode(btn_ligth_pins[i], OUTPUT);
  }

  Serial.begin(9600);

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

  // buton tester
  //  while (true)
  //  {
  //    for (int i = 0; i <= gameStep; i++)
  //    {
  //      int btnP = readButtons();
  //      Serial.print("BUTTON: ");
  //      Serial.println(btnP);
  //      if(btnP >-1)
  //        ligthBtn(ORANGE,btnP);
  //    }
  //  }

  // while (true)
  // {
  // colorWipe(strip.Color(255, 0, 200), 50); // pink
  colorWipe(strip.Color(255, 255, 255), 1); // pink
  // if (digitalRead(btn_pins[0]) || digitalRead(btn_pins[1]) || digitalRead(btn_pins[2]) || digitalRead(btn_pins[3]))
  //   break;
  // colorWipe(strip.Color(200, 255, 0), 50); // yellow
  colorWipe(strip.Color(0, 0, 0), 1); // pink

  // }

  /*
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

  myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms

  //----Set volume----
  myDFPlayer.volume(30); // Set volume value (0~30).
  */
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

// LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
void loop()
{
  // call to action
  // Serial.print(digitalRead(btn_pins[0]));
  // Serial.print(digitalRead(btn_pins[1]));
  // Serial.print(digitalRead(btn_pins[2]));
  // Serial.print(digitalRead(btn_pins[3]));

  while (!digitalRead(btn_pins[0]) || !digitalRead(btn_pins[2]))
  {
    Serial.println("CTA");
    // colorWipe(strip.Color(255, 0, 200), 50); // pink
    digitalWrite(btn_ligth_pins[0], HIGH);
    digitalWrite(btn_ligth_pins[1], HIGH);
    digitalWrite(btn_ligth_pins[2], HIGH);
    digitalWrite(btn_ligth_pins[3], HIGH);

    theaterChase(strip.Color(0, 0, 255), 100); // pink
    if (digitalRead(btn_pins[0]) && digitalRead(btn_pins[2]))
      break;
    // colorWipe(strip.Color(200, 255, 0), 50); // yellow
    // colorWipe(strip.Color(0, 0, 0), 50); // pink
  }
  Serial.println("START");

  // int this_melody[2] = {NOTE_C4, NOTE_C5}; // ,E, E, C, D, E, E, D, C};
  // for (int thisNote = 0; thisNote < 2; thisNote++)
  // {
  //   tone(4, this_melody[thisNote], TEMPO / 2);
  //   delay(TEMPO / 2);
  //   noTone(4);
  // }

  int samples = 2000;
  for (int i = 0; i < samples; i++)
  {
    // sweep up
    tone(4, map(i, samples, 0, 500, 100), 100);
  }

  digitalWrite(btn_ligth_pins[0], LOW);
  delay(250);
  digitalWrite(btn_ligth_pins[1], LOW);
  delay(250);
  digitalWrite(btn_ligth_pins[2], LOW);
  delay(250);
  digitalWrite(btn_ligth_pins[3], LOW);
  colorWipe(strip.Color(0, 0, 255), 10); // yellow
  delay(500);

  // secuencia de inicio luces
  for (int i = 0; i < BTN_COUNT; i++)
  {
    ligthBtn(btn_color[i], i);
    delay(200);
  }

  ligthBtn(0, 3);

  delay(1000);

  // juego
  if (doGame())
  // if(true)
  {
    // ------------------------------------------ WON
    // playMelody(MELODY_STEPS);
    delay(200);
    digitalWrite(btn_ligth_pins[0], HIGH);
    digitalWrite(btn_ligth_pins[2], HIGH);

    // won melody animation
    int this_melody[9] = {NOTE_C4, NOTE_E4, NOTE_A4, NOTE_C4, NOTE_E4, NOTE_A4, NOTE_E4, NOTE_A4, NOTE_C5}; // ,E, E, C, D, E, E, D, C};
    for (int thisNote = 0; thisNote < 9; thisNote++)
    {
      digitalWrite(btn_ligth_pins[0], !digitalRead(btn_ligth_pins[0]));
      digitalWrite(btn_ligth_pins[1], !digitalRead(btn_ligth_pins[1]));
      digitalWrite(btn_ligth_pins[2], !digitalRead(btn_ligth_pins[2]));
      digitalWrite(btn_ligth_pins[3], !digitalRead(btn_ligth_pins[3]));
      tone(4, this_melody[thisNote], TEMPO / 2);
      delay(TEMPO / 2);
      noTone(4);
    }

    digitalWrite(btn_ligth_pins[0], HIGH);
    digitalWrite(btn_ligth_pins[1], HIGH);
    digitalWrite(btn_ligth_pins[2], HIGH);
    digitalWrite(btn_ligth_pins[3], HIGH);

    for (int i = 0; i < 60; i++)
    {
      strip.clear();
      strip.fill(0x0000FF, 29, 29);
      strip.show();
      delay(250);
      strip.clear();
      strip.fill(0x0000FF, 0, LED_COUNT);
      strip.fill(0, 29, 29);
      strip.show();
      delay(250);
    }

    strip.fill(0, 0, 24);
    strip.show();
  }
  else
  {
    // ----------------------------------------------------- Lose
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(btn_ligth_pins[0], HIGH);
      digitalWrite(btn_ligth_pins[1], HIGH);
      digitalWrite(btn_ligth_pins[2], HIGH);
      digitalWrite(btn_ligth_pins[3], HIGH);
      delay(100);
      digitalWrite(btn_ligth_pins[0], LOW);
      digitalWrite(btn_ligth_pins[1], LOW);
      digitalWrite(btn_ligth_pins[2], LOW);
      digitalWrite(btn_ligth_pins[3], LOW);
      delay(100);
    }

    // int this_melody[2] = {NOTE_F4, NOTE_C3}; // ,E, E, C, D, E, E, D, C};
    // for (int thisNote = 0; thisNote < 2; thisNote++)
    // {
    //   // to calculate the note duration, take one second divided by the note type.
    //   // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    //   tone(4, this_melody[thisNote], TEMPO);

    //   // to distinguish the notes, set a minimum time between them.
    //   // the note's duration + 30% seems to work well:
    //   // int pauseBetweenNotes = noteDuration * 1.30;
    //   delay(TEMPO);
    //   // stop the tone playing:
    //   noTone(4);
    // }

    int samples = 2000;
    for (int i = 0; i < samples; i++)
    {
      // sweep down
      tone(4, map(i, 0, samples, 500, 100), 100);
    }

    for (int i = 0; i < 2000; i++)
    {
      // noise
      tone(4, random(0, 8000), random(0, 10));
      digitalWrite(btn_ligth_pins[0], HIGH);
      digitalWrite(btn_ligth_pins[1], HIGH);
      digitalWrite(btn_ligth_pins[2], HIGH);
      digitalWrite(btn_ligth_pins[3], HIGH);
      tone(4, random(0, 8000), random(0, 10));
      digitalWrite(btn_ligth_pins[0], LOW);
      digitalWrite(btn_ligth_pins[1], LOW);
      digitalWrite(btn_ligth_pins[2], LOW);
      digitalWrite(btn_ligth_pins[3], LOW);
    }
    noTone(4);

    strip.fill(0xFFFFFF, 0, 29);
    strip.show();
    delay(250);
    strip.fill(0xFFFFFF, 0, 29 * 2);
    strip.show();
    delay(250);
    strip.fill(0xFFFFFF, 0, LED_COUNT);
    strip.show();
    delay(250);

    delay(30000);
  }
}