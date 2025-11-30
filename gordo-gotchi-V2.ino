#include <U8g2lib.h>
#include <Wire.h>
#include <Preferences.h>

#define I2C_SDA 21
#define I2C_SCL 22

#define FEED_BUTTON 2
#define PLAY_BUTTON 4
#define SLEEP_BUTTON 5

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA);
Preferences preferences;

enum PetState {
  HAPPY,
  HUNGRY,
  SLEEPY,
  PLAYING,
  SICK,
  DEAD
};

enum PetMood {
  EXCITED,
  CONTENT,
  NEUTRAL,
  SAD,
  ANGRY
};

struct AIPet {
  String name;
  int hunger;
  int energy;
  int happiness;
  int health;
  int age;
  PetState state;
  PetMood mood;
  unsigned long lastUpdate;
  unsigned long birthTime;
  int intelligence;
  int playfulness;
};

struct MessageDisplay {
  bool active;
  String leftWord;
  String rightWord;
  unsigned long startTime;
};

AIPet myPet;
MessageDisplay messageDisplay = {false, "", "", 0};
unsigned long lastButtonCheck = 0;
unsigned long animationTimer = 0;
int animationFrame = 0;
unsigned long lastSaveTime = 0;
const unsigned long SAVE_INTERVAL = 30000;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(FEED_BUTTON, INPUT_PULLUP);
  pinMode(PLAY_BUTTON, INPUT_PULLUP);
  pinMode(SLEEP_BUTTON, INPUT_PULLUP);

  Serial.println("\n\nInitializing display...");
  Serial.println("Feed button wired correctly: GPIO2 -> Button -> GND");
  
  u8g2.begin();
  
  Serial.println("Display initialized successfully!");

  preferences.begin("tamagotchi", false);
  
  if (preferences.isKey("initialized")) {
    loadPet();
    Serial.println("Loaded existing pet from memory!");
  } else {
    initializePet();
    showBirthScreen();
    delay(3000);
    savePet();
  }
}

void loop() {
  unsigned long currentTime = millis();

  checkButtons();
  updatePetAI(currentTime);
  updateMessageDisplay(currentTime);
  updateDisplay();
  
  if (currentTime - lastSaveTime > SAVE_INTERVAL) {
    savePet();
    lastSaveTime = currentTime;
  }

  delay(100);
}

void initializePet() {
  myPet.name = "Gordo-Gachi";
  myPet.hunger = 80;
  myPet.energy = 100;
  myPet.happiness = 85;
  myPet.health = 100;
  myPet.age = 0;
  myPet.state = HAPPY;
  myPet.mood = CONTENT;
  myPet.lastUpdate = millis();
  myPet.birthTime = millis();
  myPet.intelligence = 50;
  myPet.playfulness = 70;

  Serial.println("AI Tamagotchi Born!");
  Serial.println("Name: " + myPet.name);
}

void resetGame() {
  Serial.println("Resetting game...");
  
  // Clear saved data
  preferences.clear();
  
  // Reinitialize pet
  initializePet();
  
  // Show birth screen
  showBirthScreen();
  delay(3000);
  
  // Save new pet
  savePet();
  
  Serial.println("Game reset complete!");
}

void savePet() {
  preferences.putInt("hunger", myPet.hunger);
  preferences.putInt("energy", myPet.energy);
  preferences.putInt("happiness", myPet.happiness);
  preferences.putInt("health", myPet.health);
  preferences.putInt("age", myPet.age);
  preferences.putInt("state", (int)myPet.state);
  preferences.putInt("mood", (int)myPet.mood);
  preferences.putInt("intelligence", myPet.intelligence);
  preferences.putInt("playfulness", myPet.playfulness);
  preferences.putULong("birthTime", myPet.birthTime);
  preferences.putBool("initialized", true);
  
  Serial.println("Pet saved to NVS!");
}

void loadPet() {
  myPet.name = "Gordo-Gachi";
  myPet.hunger = preferences.getInt("hunger", 80);
  myPet.energy = preferences.getInt("energy", 100);
  myPet.happiness = preferences.getInt("happiness", 85);
  myPet.health = preferences.getInt("health", 100);
  myPet.age = preferences.getInt("age", 0);
  myPet.state = (PetState)preferences.getInt("state", HAPPY);
  myPet.mood = (PetMood)preferences.getInt("mood", CONTENT);
  myPet.intelligence = preferences.getInt("intelligence", 50);
  myPet.playfulness = preferences.getInt("playfulness", 70);
  myPet.birthTime = preferences.getULong("birthTime", millis());
  myPet.lastUpdate = millis();
  
  Serial.println("Pet loaded from NVS!");
  Serial.println("Age: " + String(myPet.age) + " Health: " + String(myPet.health));
}

void drawSimpleRabbitFace(int x, int y, int size) {
  u8g2.drawCircle(x, y, size);
  
  int earLen = size + 4;
  u8g2.drawLine(x-4, y-size, x-4, y-earLen);
  u8g2.drawLine(x-3, y-size, x-3, y-earLen);
  u8g2.drawLine(x+3, y-size, x+3, y-earLen);
  u8g2.drawLine(x+4, y-size, x+4, y-earLen);
  
  u8g2.drawPixel(x-2, y-2);
  u8g2.drawPixel(x+2, y-2);
  
  u8g2.drawPixel(x, y+1);
  u8g2.drawLine(x-1, y+2, x+1, y+2);
  
  u8g2.drawLine(x-6, y, x-8, y-1);
  u8g2.drawLine(x+6, y, x+8, y-1);
}

void showBirthScreen() {
  u8g2.clearBuffer();
  
  u8g2.setFont(u8g2_font_ncenB14_tr);
  int w1 = u8g2.getStrWidth("BIRTH!");
  u8g2.drawStr((128 - w1) / 2, 18, "BIRTH!");
  
  drawSimpleRabbitFace(64, 35, 10);
  
  u8g2.setFont(u8g2_font_ncenB08_tr);
  int w3 = u8g2.getStrWidth("Gordo-Gachi!");
  u8g2.drawStr((128 - w3) / 2, 58, "Gordo-Gachi!");
  
  u8g2.sendBuffer();
}

void checkButtons() {
  unsigned long currentTime = millis();
  
  // Better debouncing - require 300ms between presses
  if (currentTime - lastButtonCheck < 300) return;

  // If pet is dead, any button resets the game
  if (myPet.state == DEAD) {
    if (digitalRead(FEED_BUTTON) == LOW || 
        digitalRead(PLAY_BUTTON) == LOW || 
        digitalRead(SLEEP_BUTTON) == LOW) {
      Serial.println("Button pressed - resetting game after death");
      resetGame();
      lastButtonCheck = currentTime;
      delay(100);
    }
    return;
  }

  // Normal button handling when alive
  if (digitalRead(FEED_BUTTON) == LOW) {
    Serial.println("Feed button pressed!");
    feedPet();
    lastButtonCheck = currentTime;
    delay(100);
  }
  else if (digitalRead(PLAY_BUTTON) == LOW) {
    Serial.println("Play button pressed!");
    playWithPet();
    lastButtonCheck = currentTime;
    delay(100);
  }
  else if (digitalRead(SLEEP_BUTTON) == LOW) {
    Serial.println("Sleep button pressed!");
    petSleep();
    lastButtonCheck = currentTime;
    delay(100);
  }
}

void showMessage(String leftWord, String rightWord) {
  messageDisplay.active = true;
  messageDisplay.leftWord = leftWord;
  messageDisplay.rightWord = rightWord;
  messageDisplay.startTime = millis();
}

void updateMessageDisplay(unsigned long currentTime) {
  if (messageDisplay.active) {
    if (currentTime - messageDisplay.startTime > 1500) {
      messageDisplay.active = false;
    }
  }
}

void feedPet() {
  if (myPet.state == DEAD) return;

  int foodType = random(0, 5);

  myPet.hunger = min(100, myPet.hunger + 25);
  myPet.happiness = min(100, myPet.happiness + 10);

  String leftWord = "Yummy";
  String rightWord = "";

  if (myPet.hunger > 90) {
    myPet.health = max(0, myPet.health - 5);
    leftWord = "Too";
    rightWord = "full!";
  } else {
    myPet.health = min(100, myPet.health + 5);
    
    switch(foodType) {
      case 0: rightWord = "carrot!"; break;
      case 1: rightWord = "pellet!"; break;
      case 2: rightWord = "greens!"; break;
      case 3: rightWord = "banana!"; break;
      case 4: rightWord = "apple!"; break;
    }
  }

  showMessage(leftWord, rightWord);

  Serial.println("Fed " + myPet.name + " - Hunger: " + String(myPet.hunger));
  savePet();
}

void playWithPet() {
  if (myPet.state == DEAD) return;

  if (myPet.energy < 20) {
    showMessage("Too", "tired!");
    return;
  }

  myPet.state = PLAYING;
  myPet.happiness = min(100, myPet.happiness + 20);
  myPet.energy = max(0, myPet.energy - 15);
  myPet.intelligence = min(100, myPet.intelligence + 2);

  showMessage("Fun", "time!");
  Serial.println("Playing with " + myPet.name + " - Happiness: " + String(myPet.happiness));
  savePet();
}

void petSleep() {
  if (myPet.state == DEAD) return;

  myPet.state = SLEEPY;
  myPet.energy = min(100, myPet.energy + 30);
  myPet.health = min(100, myPet.health + 10);

  showMessage("Sweet", "dreams!");
  Serial.println(myPet.name + " is sleeping - Energy: " + String(myPet.energy));
  savePet();
}

void updatePetAI(unsigned long currentTime) {
  if (currentTime - myPet.lastUpdate < 5000) return;

  myPet.age = (currentTime - myPet.birthTime) / 60000;

  if (myPet.state != DEAD) {
    myPet.hunger = max(0, myPet.hunger - 2);
    myPet.energy = max(0, myPet.energy - 1);

    if (myPet.hunger < 20) {
      myPet.happiness = max(0, myPet.happiness - 3);
      myPet.health = max(0, myPet.health - 2);
    }

    if (myPet.energy < 10) {
      myPet.happiness = max(0, myPet.happiness - 2);
    }

    if (myPet.health < 30) {
      myPet.state = SICK;
      myPet.happiness = max(0, myPet.happiness - 1);
    }

    if (myPet.health <= 0) {
      myPet.state = DEAD;
      Serial.println(myPet.name + " has died... RIP");
      Serial.println("Press any button to restart");
      savePet();
    }

    updateMood();
    updateState();
  }

  myPet.lastUpdate = currentTime;
}

void updateMood() {
  if (myPet.happiness > 80) {
    myPet.mood = EXCITED;
  } else if (myPet.happiness > 60) {
    myPet.mood = CONTENT;
  } else if (myPet.happiness > 40) {
    myPet.mood = NEUTRAL;
  } else if (myPet.happiness > 20) {
    myPet.mood = SAD;
  } else {
    myPet.mood = ANGRY;
  }
}

void updateState() {
  if (myPet.state == DEAD) return;

  if (myPet.state == PLAYING) {
    myPet.state = HAPPY;
  }

  if (myPet.hunger < 30 && myPet.state != SICK) {
    myPet.state = HUNGRY;
  } else if (myPet.energy < 20 && myPet.state != SICK) {
    myPet.state = SLEEPY;
  } else if (myPet.health > 30 && myPet.hunger > 30 && myPet.energy > 30) {
    myPet.state = HAPPY;
  }
}

void updateDisplay() {
  u8g2.clearBuffer();

  drawPet();
  drawStats();
  drawControls();
  drawMoodIndicator();
  
  if (messageDisplay.active) {
    drawSplitMessage();
  }

  u8g2.sendBuffer();
}

void drawMoodIndicator() {
  u8g2.setFont(u8g2_font_5x7_tr);
  
  const char* moodStr = "";
  
  switch(myPet.mood) {
    case EXCITED:
      moodStr = ":D";
      break;
    case CONTENT:
      moodStr = ":)";
      break;
    case NEUTRAL:
      moodStr = ":|";
      break;
    case SAD:
      moodStr = ":(";
      break;
    case ANGRY:
      moodStr = ">:(";
      break;
  }
  
  int moodWidth = u8g2.getStrWidth(moodStr);
  u8g2.drawStr((128 - moodWidth) / 2, 7, moodStr);
}

void drawPet() {
  int x = 64;
  int y = 28;

  if (myPet.state == DEAD) {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int w1 = u8g2.getStrWidth("R.I.P");
    u8g2.drawStr((128 - w1) / 2, 25, "R.I.P");
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    int w2 = u8g2.getStrWidth(myPet.name.c_str());
    u8g2.drawStr((128 - w2) / 2, 40, myPet.name.c_str());
    
    u8g2.setFont(u8g2_font_5x7_tr);
    int w3 = u8g2.getStrWidth("Press any button");
    u8g2.drawStr((128 - w3) / 2, 55, "Press any button");
    return;
  }

  unsigned long currentTime = millis();
  if (currentTime - animationTimer > 500) {
    animationFrame = (animationFrame + 1) % 4;
    animationTimer = currentTime;
  }

  switch (myPet.state) {
    case HAPPY:
      drawHappyRabbit(x, y);
      break;
    case HUNGRY:
      drawHungryRabbit(x, y);
      break;
    case SLEEPY:
      drawSleepyRabbit(x, y);
      break;
    case PLAYING:
      drawPlayingRabbit(x, y);
      break;
    case SICK:
      drawSickRabbit(x, y);
      break;
  }

  u8g2.setFont(u8g2_font_6x10_tr);
  int nameWidth = u8g2.getStrWidth(myPet.name.c_str());
  u8g2.drawStr((128 - nameWidth) / 2, 48, myPet.name.c_str());
}

void drawHappyRabbit(int x, int y) {
  u8g2.drawCircle(x, y+2, 8);
  
  u8g2.drawLine(x-6, y-6, x-6, y-14);
  u8g2.drawLine(x-5, y-6, x-5, y-14);
  u8g2.drawLine(x+5, y-6, x+5, y-14);
  u8g2.drawLine(x+6, y-6, x+6, y-14);
  
  u8g2.drawPixel(x-6, y-15);
  u8g2.drawPixel(x+6, y-15);
  
  u8g2.drawPixel(x-3, y-1);
  u8g2.drawPixel(x+3, y-1);
  
  u8g2.drawPixel(x, y+2);
  u8g2.drawPixel(x-1, y+3);
  u8g2.drawPixel(x+1, y+3);
  
  u8g2.drawLine(x-2, y+4, x-1, y+5);
  u8g2.drawLine(x+1, y+5, x+2, y+4);
  
  u8g2.drawLine(x-8, y+1, x-12, y);
  u8g2.drawLine(x-8, y+3, x-12, y+3);
  u8g2.drawLine(x+8, y+1, x+12, y);
  u8g2.drawLine(x+8, y+3, x+12, y+3);
}

void drawHungryRabbit(int x, int y) {
  u8g2.drawCircle(x, y+2, 8);
  
  u8g2.drawLine(x-6, y-6, x-8, y-14);
  u8g2.drawLine(x-5, y-6, x-7, y-14);
  u8g2.drawLine(x+5, y-6, x+7, y-14);
  u8g2.drawLine(x+6, y-6, x+8, y-14);
  
  u8g2.drawCircle(x-3, y-1, 1);
  u8g2.drawCircle(x+3, y-1, 1);
  
  u8g2.drawCircle(x, y+4, 2);
  
  u8g2.drawLine(x-8, y+1, x-12, y);
  u8g2.drawLine(x-8, y+3, x-12, y+3);
  u8g2.drawLine(x+8, y+1, x+12, y);
  u8g2.drawLine(x+8, y+3, x+12, y+3);
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(x+10, y-3, "?");
}

void drawSleepyRabbit(int x, int y) {
  u8g2.drawCircle(x, y+2, 8);
  
  u8g2.drawLine(x-6, y-6, x-8, y+2);
  u8g2.drawLine(x-5, y-6, x-7, y+2);
  u8g2.drawLine(x+5, y-6, x+7, y+2);
  u8g2.drawLine(x+6, y-6, x+8, y+2);
  
  u8g2.drawLine(x-4, y-1, x-2, y-1);
  u8g2.drawLine(x+2, y-1, x+4, y-1);
  
  u8g2.drawPixel(x, y+2);
  
  u8g2.drawLine(x-2, y+4, x+2, y+4);
  
  u8g2.drawLine(x-8, y+1, x-12, y);
  u8g2.drawLine(x-8, y+3, x-12, y+3);
  u8g2.drawLine(x+8, y+1, x+12, y);
  u8g2.drawLine(x+8, y+3, x+12, y+3);
  
  if (animationFrame == 0) {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(x+10, y-5, "z");
  } else if (animationFrame == 1) {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(x+12, y-8, "Z");
  }
}

void drawPlayingRabbit(int x, int y) {
  int bounceY = y + (animationFrame < 2 ? -3 : 0);
  
  u8g2.drawCircle(x, bounceY+2, 8);
  
  u8g2.drawLine(x-6, bounceY-6, x-10, bounceY-10);
  u8g2.drawLine(x-5, bounceY-6, x-9, bounceY-10);
  u8g2.drawLine(x+5, bounceY-6, x+9, bounceY-10);
  u8g2.drawLine(x+6, bounceY-6, x+10, bounceY-10);
  
  u8g2.drawPixel(x-3, bounceY-1);
  u8g2.drawPixel(x+3, bounceY-1);
  
  u8g2.drawCircle(x, bounceY+4, 2);
  
  u8g2.drawLine(x-8, bounceY+1, x-12, bounceY);
  u8g2.drawLine(x-8, bounceY+3, x-12, bounceY+3);
  u8g2.drawLine(x+8, bounceY+1, x+12, bounceY);
  u8g2.drawLine(x+8, bounceY+3, x+12, bounceY+3);
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(x-18, bounceY-5, "!");
}

void drawSickRabbit(int x, int y) {
  u8g2.drawCircle(x, y+2, 8);
  
  u8g2.drawLine(x-6, y-6, x-10, y+4);
  u8g2.drawLine(x-5, y-6, x-9, y+4);
  u8g2.drawLine(x+5, y-6, x+9, y+4);
  u8g2.drawLine(x+6, y-6, x+10, y+4);
  
  u8g2.drawLine(x-4, y-2, x-2, y);
  u8g2.drawLine(x+2, y, x+4, y-2);
  
  u8g2.drawPixel(x, y+2);
  
  u8g2.drawLine(x-2, y+6, x+2, y+6);
  u8g2.drawPixel(x-2, y+5);
  u8g2.drawPixel(x+2, y+5);
  
  u8g2.drawLine(x-8, y+1, x-12, y);
  u8g2.drawLine(x-8, y+3, x-12, y+3);
  u8g2.drawLine(x+8, y+1, x+12, y);
  u8g2.drawLine(x+8, y+3, x+12, y+3);
  
  if (animationFrame % 2 == 0) {
    u8g2.drawCircle(x+9, y-3, 1);
    u8g2.drawPixel(x+9, y-2);
  }
  
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(x+10, y-2, "...");
}

void drawStats() {
  char buffer[20];
  
  u8g2.setFont(u8g2_font_5x7_tr);
  
  sprintf(buffer, "Age:%dm", myPet.age);
  u8g2.drawStr(0, 7, buffer);
  
  sprintf(buffer, "HP:%d", myPet.health);
  u8g2.drawStr(0, 15, buffer);
  
  sprintf(buffer, "Food:%d", myPet.hunger);
  u8g2.drawStr(79, 7, buffer);
  
  sprintf(buffer, "Energy:%d", myPet.energy);
  u8g2.drawStr(79, 15, buffer);
}

void drawControls() {
  u8g2.setFont(u8g2_font_5x7_tr);
  
  u8g2.drawStr(5, 63, "Feed");
  u8g2.drawStr(50, 63, "Play");
  u8g2.drawStr(95, 63, "Sleep");
}

void drawSplitMessage() {
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  u8g2.drawStr(5, 30, messageDisplay.leftWord.c_str());
  
  int rightWidth = u8g2.getStrWidth(messageDisplay.rightWord.c_str());
  u8g2.drawStr(123 - rightWidth, 30, messageDisplay.rightWord.c_str());
}