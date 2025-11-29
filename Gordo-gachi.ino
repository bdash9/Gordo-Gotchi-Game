// ===== PART 1/3 =====

#include <TFT_eSPI.h>
#include <Wire.h>
#include <Adafruit_seesaw.h>
#include <math.h>

// -- Function Prototypes --
bool anyButtonPressed();
void clearGordoRegion(float x, float y);
void drawSectionalCouchPartial(int bx, int by, int bw, int bh);
void drawSectionalCouch();
void drawGlassTablePartial(int bx, int by, int bw, int bh);
void drawGlassTable();
void drawCarrot(int x, int y, int size=24, bool blank=false);
void drawTube(int x, int y, int w=22, int h=60, bool blank=false);
void drawTreatMeter(int num_eaten, bool isTube=false);
void spawnCarrot();
void spawnTube();
void drawCuteDutchRabbitFaceWithSmile(int cx, int cy, int sizex, int sizey);
void drawSplash();
void drawGordo(float x, float y, int frame=0, bool hide=false);
void drawLitterBox(int drawPoops = -1);
void drawHayBowl(bool showHay = true);
void drawWaterBowl(bool showWater = true);
void draw_StatusBars();
void drawMainScreen();
void drawBlanketRight();
void drawBlanketLeft();
void flashLivingRoomOpened();

#define JOY_ADDR      0x49
#define JOY_SDA_PIN   21
#define JOY_SCL_PIN   22
#define BUTTON_A      6
#define BUTTON_B      7
#define BUTTON_X      9
#define BUTTON_Y      10
#define BUTTON_SELECT 14
#define JOY_X         3
#define JOY_Y         2

#define SCREEN_W      480
#define SCREEN_H      320

#define GORDO_W     56
#define GORDO_H     50
#define GORDO_HOP_DY -18
#define GORDO_SPEED  5
#define GORDO_FAST   18
#define GORDO_EAR_ABOVE 62
#define GORDO_FOOT_BELOW 25

#define LIT_W        144
#define LIT_H        86
#define LIT_X        10
#define LIT_Y        16
#define LIT_D        18

#define HAY_W        130
#define HAY_H        36
#define HAY_X        (SCREEN_W/2-HAY_W/2)
#define HAY_Y        54

#define WAT_R        56
#define WAT_X        (SCREEN_W-WAT_R-40)
#define WAT_Y        38

#define BLANKET_W    (SCREEN_W/3)
#define BLANKET_H    90
#define BLANKET_Y    170

#define STATUS_BAR_W 110
#define STATUS_BAR_H 16
#define STATUS_BAR_Y (SCREEN_H-22)
#define POOP_MAX   8
#define HAY_MAX    8
#define WATER_MAX  8

#define COLOR_LIGHTBROWN 0xBC40
#define COLOR_DARKBROWN 0x7B60
#define COLOR_BLUE      0x129F
#define COLOR_YELLOW    0xFFE0
#define COLOR_FRONTGRAY 0xC618
#define COLOR_TOPGRAY   0xC618
#define COLOR_LEFTGRAY  0x528A
#define COLOR_RIGHTGRAY 0x528A
#define COLOR_TRIANGLEGRAY 0x630C
#define COLOR_PINKTOWEL  0xF81F
#define COLOR_DARKTEAL   0x232D
#define COLOR_CARROT_ORANGE 0xFBE0
#define COLOR_CARROT_GREEN  0x0320
#define COLOR_TUBE_BROWN 0xBC40
#define COLOR_TUBE_END   0x7B60

TFT_eSPI tft = TFT_eSPI();
Adafruit_seesaw ss;

enum State {
  STATE_SPLASH, STATE_GAME, STATE_LIVINGROOM, STATE_TREATFLASH,
  STATE_ZOOMIES_OVERLAY, STATE_TUBE, STATE_NAPTIME
} state = STATE_SPLASH;

float gordo_x = SCREEN_W/2, gordo_y = SCREEN_H-54;
int poop_bar = 0, hay_bar = 0, water_bar = 0;
bool gordo_in_hop = false;
unsigned long gordo_hop_start_time = 0, gordo_hop_duration = 210;
int joyX = 512, joyY = 512;

int current_treat_x = 120, current_treat_y = 140;
bool treat_present = false;
int treat_flash_count = 0;
int carrots_eaten = 0;
int tubes_eaten = 0;
bool show_meter = false;
bool treat_time_started = false;
bool treat_meter_delay = false;
unsigned long treat_meter_delay_start = 0;
bool show_tube_meter = false;
bool tube_present = false;
bool tube_meter_delay = false;
unsigned long tube_meter_delay_start = 0;
bool needLivingroomRedraw = true;
unsigned long zoomies_start = 0;
bool print_zoomies_message = false;
bool tubes_phase_started = false;
unsigned long zoomies_print_time = 0;
bool livingRoomJustOpened = false;
unsigned long nap_time_started = 0;
bool gordo_hidden = false;

const int table_cx = SCREEN_W/2 + 60;
const int table_cy = SCREEN_H/2;
const int COUCH_THICKNESS = 72;

// -- Function Implementations 1/3 ----------------

bool anyButtonPressed() {
  return !ss.digitalRead(BUTTON_A) ||
         !ss.digitalRead(BUTTON_B) ||
         !ss.digitalRead(BUTTON_X) ||
         !ss.digitalRead(BUTTON_Y) ||
         !ss.digitalRead(BUTTON_SELECT);
}
void clearGordoRegion(float x, float y) {
  int bx = int(x) - GORDO_W/2 - 18;
  int by = int(y) - GORDO_EAR_ABOVE - 14;
  int bw = GORDO_W + 36;
  int bh = GORDO_H + GORDO_EAR_ABOVE + GORDO_FOOT_BELOW + 32;
  tft.fillRect(bx, by, bw, bh, TFT_BLACK);
}
void drawSectionalCouchPartial(int bx, int by, int bw, int bh) {
  int thickness = COUCH_THICKNESS;
  int couchHorizW = SCREEN_W / 2;
  int xh = SCREEN_W - couchHorizW, yh = 0, wh = couchHorizW, hh = thickness;
  int xv = SCREEN_W-thickness, yv = 0, wv = thickness, hv = SCREEN_H;
  int rx, ry, rw, rh;
  rx = std::max(xh,bx); ry = std::max(yh,by); rw = std::max(0,std::min(xh+wh,bx+bw)-rx); rh = std::max(0,std::min(yh+hh,by+bh)-ry);
  if(rw>0 && rh>0) tft.fillRect(rx, ry, rw, rh, COLOR_DARKTEAL);
  rx = std::max(xv,bx); ry = std::max(yv,by); rw = std::max(0,std::min(xv+wv,bx+bw)-rx); rh = std::max(0,std::min(yv+hv,by+bh)-ry);
  if(rw>0 && rh>0) tft.fillRect(rx, ry, rw, rh, COLOR_DARKTEAL);
}
void drawSectionalCouch() {
  int thickness = COUCH_THICKNESS;
  int couchHorizW = SCREEN_W/2;
  tft.fillRect(SCREEN_W-couchHorizW, 0, couchHorizW, thickness, COLOR_DARKTEAL);
  tft.fillRect(SCREEN_W-thickness, 0, thickness, SCREEN_H, COLOR_DARKTEAL);
}
void drawGlassTablePartial(int bx, int by, int bw, int bh) {
  int cx = table_cx;
  int cy = table_cy;
  int R1 = 54, R2 = 44, t = 7;
  int minx = cx-R1, miny = cy-R1, maxx = cx+R1, maxy = cy+R1;
  if (bx+bw < minx || bx > maxx || by+bh < miny || by > maxy) return;
  tft.fillCircle(cx, cy, R1, COLOR_DARKBROWN);
  tft.fillCircle(cx, cy, R2, TFT_WHITE);
  tft.fillRect(cx-R2, cy-t/2, R2*2, t, COLOR_DARKBROWN);
  tft.fillRect(cx-t/2, cy-R2, t, R2*2, COLOR_DARKBROWN);
  tft.drawCircle(cx, cy, R1, COLOR_DARKBROWN);
  tft.drawCircle(cx, cy, R1-1, COLOR_DARKBROWN);
  tft.drawCircle(cx, cy, R2, COLOR_DARKBROWN);
}
void drawGlassTable() {
  int cx = table_cx;
  int cy = table_cy;
  int R1 = 54, R2 = 44, t = 7;
  tft.fillCircle(cx, cy, R1, COLOR_DARKBROWN);
  tft.fillCircle(cx, cy, R2, TFT_WHITE);
  tft.fillRect(cx-R2, cy-t/2, R2*2, t, COLOR_DARKBROWN);
  tft.fillRect(cx-t/2, cy-R2, t, R2*2, COLOR_DARKBROWN);
  tft.drawCircle(cx, cy, R1, COLOR_DARKBROWN);
  tft.drawCircle(cx, cy, R1-1, COLOR_DARKBROWN);
  tft.drawCircle(cx, cy, R2, COLOR_DARKBROWN);
}
void drawCarrot(int x, int y, int size, bool blank) {
  // Tip pointing down, green at top
  if(!blank) {
    tft.fillTriangle(x, y, x-size/2, y-size, x+size/2, y-size, COLOR_CARROT_ORANGE);
    tft.drawTriangle(x, y, x-size/2, y-size, x+size/2, y-size, TFT_BROWN);
    tft.drawLine(x, y-size, x, y-size-10, COLOR_CARROT_GREEN);
    tft.drawLine(x, y-size, x-3, y-size-7, COLOR_CARROT_GREEN);
    tft.drawLine(x, y-size, x+3, y-size-7, COLOR_CARROT_GREEN);
  } else {
    tft.drawTriangle(x, y, x-size/2, y-size, x+size/2, y-size, TFT_DARKGREY);
    tft.drawLine(x, y-size, x, y-size-10, TFT_DARKGREY);
    tft.drawLine(x, y-size, x-3, y-size-7, TFT_DARKGREY);
    tft.drawLine(x, y-size, x+3, y-size-7, TFT_DARKGREY);
  }
}
void drawTube(int x, int y, int w, int h, bool blank) {
  // Tall/elongated (h) and thinner (w)
  if(!blank) {
    tft.fillRect(x-w/2, y-h/2, w, h, COLOR_TUBE_BROWN);
    tft.fillEllipse(x, y-h/2, w/2, w/2, COLOR_TUBE_END);
    tft.drawRect(x-w/2, y-h/2, w, h, COLOR_DARKBROWN);
    tft.drawEllipse(x, y-h/2, w/2, w/2, COLOR_DARKBROWN);
  } else {
    tft.drawRect(x-w/2, y-h/2, w, h, TFT_DARKGREY);
    tft.drawEllipse(x, y-h/2, w/2, w/2, TFT_DARKGREY);
  }
}
// ===== PART 2/3 =====

void drawTreatMeter(int num_eaten, bool isTube) {
  int tx = 120, ty = 28;
  tft.fillRect(0, ty-8, 260, 65, TFT_BLACK); // ensures no blue line at right
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Treats:", tx-55, ty, 2); // now Treats: is fully visible
  for (int i=0; i<3; ++i) {
    if (!isTube) {
      if (i < num_eaten)
        drawCarrot(tx+i*48, ty+28, 22, false);
    } else {
      if (i < num_eaten)
        drawTube(tx+i*55, ty+33, 22, 60, false);
    }
  }
}
void spawnCarrot() {
  int xc, yc, ntries=0;
  while (1) {
    xc = 40 + random(0,SCREEN_W-80);
    yc = 80 + random(0,SCREEN_H-120);
    if (xc < 200 && yc < 80) { ++ntries; if (ntries>10) break; continue; }
    int dx = xc-table_cx, dy = yc-table_cy;
    if (dx*dx+dy*dy < 5200) { ++ntries; if(ntries>20) break; continue; }
    if (xc > SCREEN_W - BLANKET_W && yc > BLANKET_Y-10) { ++ntries; if (ntries>20) break; continue; }
    if (abs(xc-SCREEN_W/2)<70 && abs(yc-SCREEN_H-54)<90) { ++ntries; if (ntries>24) break; continue; }
    break;
  }
  current_treat_x = xc;
  current_treat_y = yc;
  treat_present = true;
}
void spawnTube() {
  int xc, yc, ntries=0;
  while (1) {
    xc = 40 + random(0,SCREEN_W-80);
    yc = 80 + random(0,SCREEN_H-120);
    if (xc < 200 && yc < 80) { ++ntries; if (ntries>10) break; continue; }
    int dx = xc-table_cx, dy = yc-table_cy;
    if (dx*dx+dy*dy < 5200) { ++ntries; if(ntries>20) break; continue; }
    if (xc > SCREEN_W - BLANKET_W && yc > BLANKET_Y-10) { ++ntries; if (ntries>20) break; continue; }
    if (abs(xc-SCREEN_W/2)<70 && abs(yc-SCREEN_H-54)<90) { ++ntries; if (ntries>24) break; continue; }
    break;
  }
  current_treat_x = xc;
  current_treat_y = yc;
  tube_present = true;
}
// --- Splash, Gordo, and Environment Drawing ---
void drawCuteDutchRabbitFaceWithSmile(int cx, int cy, int sizex, int sizey) {
  int ear_h = sizey * 0.63, ear_w = sizex / 4, ear_gap = sizex / 7;
  tft.fillRect(cx-ear_gap-ear_w/2, cy-sizey/2-ear_h+10, ear_w, ear_h, TFT_DARKGREY);
  tft.fillEllipse(cx-ear_gap, cy-sizey/2-ear_h+10, ear_w/2, ear_w/2, TFT_DARKGREY);
  tft.fillRect(cx+ear_gap-ear_w/2, cy-sizey/2-ear_h+10, ear_w, ear_h, TFT_DARKGREY);
  tft.fillEllipse(cx+ear_gap, cy-sizey/2-ear_h+10, ear_w/2, ear_w/2, TFT_DARKGREY);
  tft.fillEllipse(cx, cy, sizex/2, sizey/2, TFT_WHITE );
  tft.fillEllipse(cx-sizex/4, cy+sizey/6, sizex/7, sizex/9, TFT_WHITE);
  tft.fillEllipse(cx+sizex/4, cy+sizey/6, sizex/7, sizex/9, TFT_WHITE);
  tft.fillEllipse(cx-sizex/4, cy, sizex/6, sizey/2-7, TFT_DARKGREY);
  tft.fillEllipse(cx+sizex/4, cy, sizex/6, sizey/2-7, TFT_DARKGREY);
  uint16_t darkbrown = 0x3200;
  tft.fillEllipse(cx-sizex/8, cy-sizey/12, sizex/13, sizex/13, darkbrown);
  tft.fillEllipse(cx+sizex/8, cy-sizey/12, sizex/13, sizex/13, darkbrown);
  tft.fillEllipse(cx-sizex/8+5, cy-sizey/12-4, 4, 4, TFT_WHITE);
  tft.fillEllipse(cx+sizex/8+5, cy-sizey/12-4, 4, 4, TFT_WHITE);
  tft.fillEllipse(cx, cy+sizey/8, sizex/18, sizex/13, TFT_WHITE);
  tft.fillTriangle(cx-7, cy+sizey/8+8, cx+7, cy+sizey/8+8, cx, cy+sizey/8+18, 0xF81F);
  tft.drawLine(cx-5, cy+sizey/8+22, cx, cy+sizey/8+13, 0xF81F);
  tft.drawLine(cx+5, cy+sizey/8+22, cx, cy+sizey/8+13, 0xF81F);
}
void drawSplash() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("GORDO-GACHI!", SCREEN_W/2, 56, 4);
  drawCuteDutchRabbitFaceWithSmile(SCREEN_W/2, SCREEN_H/2+42, 120, 92);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Press any button to play!", SCREEN_W/2, SCREEN_H-22, 2);
}
void drawGordo(float x, float y, int frame, bool hide) {
  if (hide) return;
  int dx = (frame ? 2 : 0);
  int dy = (frame ? -8 : 0);
  tft.fillEllipse(x, y+20+dy, 26, 22, TFT_WHITE);
  tft.fillEllipse(x-18, y+22+dy, 15, 20, TFT_DARKGREY);
  tft.fillEllipse(x+18, y+22+dy, 15, 18, TFT_DARKGREY);
  tft.fillEllipse(x, y, 17, 15, TFT_WHITE);
  tft.fillEllipse(x-12, y-2, 7, 12-dy/2, TFT_DARKGREY);
  tft.fillEllipse(x+12, y-2, 7, 12-dy/2, TFT_DARKGREY);
  tft.fillEllipse(x-8, y-28+dx, 6, 18-(frame?3:0), TFT_DARKGREY);
  tft.fillEllipse(x+7, y-28+dx, 6, 18-(frame?3:0), TFT_DARKGREY);
  tft.fillEllipse(x, y+5, 4, 5, TFT_WHITE);
  tft.fillEllipse(x-8, y-2, 3, 3, TFT_BLACK);
  tft.fillEllipse(x+8, y-2, 3, 3, TFT_BLACK);
  tft.fillTriangle(x-3, y+8, x+3, y+8, x, y+12, 0xF81F);
  tft.drawLine(x-3, y+12, x, y+8, 0xF81F);
  tft.drawLine(x+3, y+12, x, y+8, 0xF81F);
}
void drawLitterBox(int drawPoops) {
  tft.fillRect(LIT_X+LIT_D, LIT_Y+LIT_H-LIT_D, LIT_W-LIT_D*2, LIT_D, COLOR_FRONTGRAY);
  tft.fillRect(LIT_X, LIT_Y+LIT_D, LIT_D, LIT_H-LIT_D*2, COLOR_LEFTGRAY);
  tft.fillRect(LIT_X+LIT_W-LIT_D, LIT_Y+LIT_D, LIT_D, LIT_H-LIT_D*2, COLOR_RIGHTGRAY);
  tft.fillRect(LIT_X+LIT_D, LIT_Y, LIT_W-LIT_D*2, LIT_D, COLOR_TOPGRAY);
  tft.fillTriangle(LIT_X+LIT_D, LIT_Y, LIT_X, LIT_Y+LIT_D, LIT_X+LIT_D, LIT_Y+LIT_D, COLOR_TRIANGLEGRAY);
  tft.fillTriangle(LIT_X+LIT_W-LIT_D, LIT_Y, LIT_X+LIT_W-LIT_D, LIT_Y+LIT_D, LIT_X+LIT_W, LIT_Y+LIT_D, COLOR_TRIANGLEGRAY);
  tft.fillTriangle(LIT_X, LIT_Y+LIT_H-LIT_D, LIT_X+LIT_D, LIT_Y+LIT_H, LIT_X+LIT_D, LIT_Y+LIT_H-LIT_D, COLOR_TRIANGLEGRAY);
  tft.fillTriangle(LIT_X+LIT_W, LIT_Y+LIT_H-LIT_D, LIT_X+LIT_W-LIT_D, LIT_Y+LIT_H, LIT_X+LIT_W-LIT_D, LIT_Y+LIT_H-LIT_D, COLOR_TRIANGLEGRAY);
  tft.drawLine(LIT_X, LIT_Y+LIT_D, LIT_X, LIT_Y+LIT_H-LIT_D, COLOR_LEFTGRAY);
  tft.drawLine(LIT_X+LIT_W, LIT_Y+LIT_H-LIT_D, LIT_X+LIT_W, LIT_Y+LIT_D, COLOR_RIGHTGRAY);
  tft.drawLine(LIT_X+LIT_D, LIT_Y, LIT_X+LIT_W-LIT_D, LIT_Y, COLOR_TOPGRAY);
  tft.drawLine(LIT_X+LIT_D, LIT_Y+LIT_H, LIT_X+LIT_W-LIT_D, LIT_Y+LIT_H, COLOR_FRONTGRAY);
  tft.fillRect(LIT_X+LIT_D, LIT_Y+LIT_D, LIT_W-LIT_D*2, LIT_H-LIT_D*2, TFT_DARKGREY);
  if (drawPoops > 0) {
    for (int i=0; i < drawPoops; ++i) {
      int px = LIT_X + LIT_D + 14 + (i%4)*26;
      int py = LIT_Y + LIT_D + 12 + (i/4)*16;
      tft.fillCircle(px, py, 5, 0x8B22);
      tft.drawCircle(px, py, 5, 0x6331);
    }
  }
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COLOR_LIGHTBROWN, TFT_BLACK);
  tft.drawString("Litter", LIT_X+LIT_W/2, LIT_Y+LIT_H+18, 2);
}
void drawHayBowl(bool showHay) {
  tft.fillRect(HAY_X, HAY_Y, HAY_W, HAY_H, TFT_BLACK);
  if (showHay) {
    int sticks[][4] = {
      { HAY_X+24, HAY_Y+18, HAY_X+104, HAY_Y+9 },  { HAY_X+38, HAY_Y+26, HAY_X+103, HAY_Y+24 },
      { HAY_X+22, HAY_Y+14, HAY_X+81, HAY_Y+19 },  { HAY_X+41, HAY_Y+31, HAY_X+52, HAY_Y+9 },
      { HAY_X+65, HAY_Y+34, HAY_X+118, HAY_Y+13 }, { HAY_X+90, HAY_Y+32, HAY_X+92, HAY_Y+19 },
      { HAY_X+78, HAY_Y+29, HAY_X+117, HAY_Y+16 }, { HAY_X+57, HAY_Y+13, HAY_X+129, HAY_Y+29 },
      { HAY_X+70, HAY_Y+9, HAY_X+92, HAY_Y+29 },   { HAY_X+32, HAY_Y+13, HAY_X+94, HAY_Y+21 },
      { HAY_X+88, HAY_Y+21, HAY_X+131, HAY_Y+28 }, { HAY_X+52, HAY_Y+22, HAY_X+104, HAY_Y+24 },
      { HAY_X+30, HAY_Y+34, HAY_X+123, HAY_Y+13 }
    };
    for (int i=0; i<13; ++i) {
      uint16_t col = (i%2) ? 0xBDF7 : 0xFFE0;
      tft.drawLine(sticks[i][0], sticks[i][1], sticks[i][2], sticks[i][3], col);
    }
  }
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Hay", HAY_X+HAY_W/2, HAY_Y+HAY_H+18, 2);
}
void drawWaterBowl(bool showWater) {
  tft.fillRect(WAT_X-12, WAT_Y-12, WAT_R+28, WAT_R+32, TFT_BLACK);
  tft.fillCircle(WAT_X+WAT_R/2, WAT_Y+WAT_R/2, WAT_R/2, TFT_WHITE);
  tft.drawCircle(WAT_X+WAT_R/2, WAT_Y+WAT_R/2, WAT_R/2, TFT_DARKGREY);
  if (showWater) tft.fillCircle(WAT_X+WAT_R/2, WAT_Y+WAT_R/2, WAT_R/2-8, TFT_BLUE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("Water", WAT_X+WAT_R/2, WAT_Y+WAT_R+18, 2);
}
void draw_StatusBars() {
  int x1 = 10, x2 = SCREEN_W/2-STATUS_BAR_W/2, x3 = SCREEN_W-STATUS_BAR_W-10;
  int fills[3] = { poop_bar, hay_bar, water_bar };
  uint16_t colors[3] = { COLOR_LIGHTBROWN, TFT_YELLOW, TFT_BLUE };
  const char* labels[3] = { "Poops", "Hay", "Water" };
  tft.setTextDatum(MC_DATUM);
  for (int i=0;i<3;++i) {
    int xx = (i==0)?x1:(i==1)?x2:x3;
    tft.drawRect(xx, STATUS_BAR_Y, STATUS_BAR_W, STATUS_BAR_H, TFT_WHITE);
    tft.fillRect(xx+2, STATUS_BAR_Y+2, (STATUS_BAR_W-4)*fills[i] / POOP_MAX, STATUS_BAR_H-4, colors[i]);
    tft.setTextColor(colors[i], TFT_BLACK);
    tft.drawString(labels[i], xx+STATUS_BAR_W/2, STATUS_BAR_Y-18, 2);
  }
}
void drawMainScreen() {
  tft.fillScreen(TFT_BLACK);
  drawLitterBox(poop_bar);
  drawHayBowl(hay_bar < HAY_MAX);
  drawWaterBowl(water_bar < WATER_MAX);
  draw_StatusBars();
}
void drawBlanketRight() {
  tft.fillRect(SCREEN_W-BLANKET_W, BLANKET_Y, BLANKET_W, BLANKET_H, COLOR_PINKTOWEL);
}
void drawBlanketLeft() {
  tft.fillRect(0, BLANKET_Y, BLANKET_W, BLANKET_H, COLOR_PINKTOWEL);
}
void flashLivingRoomOpened() {
  tft.setTextDatum(MC_DATUM);
  for (int k=0; k<4; ++k) {
    tft.setTextColor(0xF81F, TFT_BLACK);
    tft.drawString("LIVING-ROOM OPENED!", SCREEN_W/2, 32, 4);
    delay(225);
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.drawString("LIVING-ROOM OPENED!", SCREEN_W/2, 32, 4);
    delay(180);
  }
}
// ===== PART 3/3 =====

void setup() {
  tft.begin();
  tft.setRotation(1);
  Wire.begin(JOY_SDA_PIN, JOY_SCL_PIN);
  delay(220);
  if (!ss.begin(JOY_ADDR)) { while(1); }
  ss.pinMode(BUTTON_A, INPUT_PULLUP);
  ss.pinMode(BUTTON_B, INPUT_PULLUP);
  ss.pinMode(BUTTON_X, INPUT_PULLUP);
  ss.pinMode(BUTTON_Y, INPUT_PULLUP);
  ss.pinMode(BUTTON_SELECT, INPUT_PULLUP);
  delay(100);
  tft.fillScreen(TFT_BLACK);
  drawSplash();
  state = STATE_SPLASH;
  randomSeed(analogRead(0));
}

void loop() {
  static unsigned long treat_time_msg_ms = 0;
  static unsigned long zoomies_start_time = 0;
  static bool zoomies_displayed = false;
  static unsigned long nap_time_started_local = 0;

  if (state == STATE_SPLASH) {
    drawSplash();
    while (!anyButtonPressed()) { delay(7);}
    while (anyButtonPressed()) { delay(7);}
    poop_bar = 0; hay_bar = 0; water_bar = 0;
    gordo_x = SCREEN_W/2; gordo_y = SCREEN_H-54;
    tft.fillScreen(TFT_BLACK);
    drawMainScreen();
    needLivingroomRedraw = true;
    treat_time_started = false;
    treat_present = false;
    carrots_eaten = 0;
    tubes_eaten = 0;
    show_meter = false;
    tube_present = false;
    show_tube_meter = false;
    tube_meter_delay = false;
    tubes_phase_started = false;
    print_zoomies_message = false;
    livingRoomJustOpened = false;
    gordo_hidden = false;
    state = STATE_GAME;
    delay(120);
    return;
  }

  if (state == STATE_GAME) {
    float prev_x = gordo_x, prev_y = gordo_y;
    joyX = ss.analogRead(JOY_X);
    joyY = ss.analogRead(JOY_Y);

    int vx = 0, vy = 0;
    if (joyX < 410) vx = -GORDO_SPEED;
    if (joyX > 610) vx = GORDO_SPEED;
    if (joyY < 410) vy = -GORDO_SPEED;
    if (joyY > 610) vy = GORDO_SPEED;
    bool moving = (vx != 0 || vy != 0);

    if (moving && !gordo_in_hop) {
      gordo_in_hop = true;
      gordo_hop_start_time = millis();
    }
    if (gordo_in_hop) {
      if (millis() - gordo_hop_start_time > gordo_hop_duration) gordo_in_hop = false;
    }

    gordo_x += vx; gordo_y += vy;
    if (gordo_x < 20) gordo_x = 20;
    if (gordo_x > SCREEN_W-20) gordo_x = SCREEN_W-20;
    if (gordo_y < 30) gordo_y = 30;
    if (gordo_y > SCREEN_H-16-GORDO_H/2) gordo_y = SCREEN_H-16-GORDO_H/2;

    if (moving) clearGordoRegion(prev_x, prev_y);

    drawLitterBox(poop_bar);
    drawHayBowl(hay_bar < HAY_MAX);
    drawWaterBowl(water_bar < WATER_MAX);
    draw_StatusBars();

    int hop_frame = gordo_in_hop ? 1 : 0;
    float hop_offset = hop_frame ? GORDO_HOP_DY : 0;
    int towel_left_edge = SCREEN_W-BLANKET_W;
    int towel_right_edge = SCREEN_W;
    int towel_warp_start = towel_left_edge+2*BLANKET_W/3;
    bool all_bars_full = (poop_bar >= POOP_MAX && hay_bar >= HAY_MAX && water_bar >= WATER_MAX);

    if (all_bars_full && !livingRoomJustOpened) {
      flashLivingRoomOpened();
      livingRoomJustOpened = true;
    }
    if (all_bars_full) {
      drawBlanketRight();
      if (gordo_x >= towel_warp_start && gordo_x < towel_right_edge &&
          gordo_y > BLANKET_Y-18 && gordo_y < BLANKET_Y+BLANKET_H+18) {
        tft.fillScreen(TFT_BLACK);
        drawBlanketLeft();
        gordo_x = BLANKET_W/2;
        gordo_y = BLANKET_Y+BLANKET_H/2;
        delay(120);
        state = STATE_LIVINGROOM;
        needLivingroomRedraw = true;
        treat_time_started = false;
        treat_present = false;
        carrots_eaten = 0;
        tubes_eaten = 0;
        show_meter = false;
        tube_present = false;
        show_tube_meter = false;
        tube_meter_delay = false;
        tubes_phase_started = false;
        print_zoomies_message = false;
        gordo_hidden = false;
        return;
      }
    }
    drawGordo(gordo_x, gordo_y+hop_offset, hop_frame, false);

    if (gordo_x > LIT_X-18 && gordo_x < LIT_X+LIT_W+20 &&
        gordo_y > LIT_Y+10 && gordo_y < LIT_Y+LIT_H+40) {
      if (poop_bar < POOP_MAX) { poop_bar++; delay(230);}
    }
    if (gordo_x > HAY_X-22 && gordo_x < HAY_X+HAY_W+12 &&
        gordo_y > HAY_Y && gordo_y < HAY_Y+HAY_H+14) {
      if (hay_bar < HAY_MAX) { hay_bar++; delay(210);}
    }
    if (gordo_x > WAT_X-24 && gordo_x < WAT_X+WAT_R+18 &&
        gordo_y > WAT_Y && gordo_y < WAT_Y+WAT_R+7) {
      if (water_bar < WATER_MAX) { water_bar++; delay(210);}
    }
    delay(14);
    return;
  }

  // Living Room, TreatFlash, Zoomies, Tube, Nap
  if (state == STATE_LIVINGROOM || state == STATE_TREATFLASH || state == STATE_ZOOMIES_OVERLAY || state == STATE_TUBE || state == STATE_NAPTIME) {
    float prev_x = gordo_x, prev_y = gordo_y;
    joyX = ss.analogRead(JOY_X);
    joyY = ss.analogRead(JOY_Y);
    int speed = (state == STATE_ZOOMIES_OVERLAY) ? GORDO_FAST : GORDO_SPEED;
    int vx = 0, vy = 0;
    if (!gordo_hidden) {
      if (joyX > 610) vx = speed;
      if (joyX < 410) vx = -speed;
      if (joyY < 410) vy = -speed;
      if (joyY > 610) vy = speed;
    }
    bool moving = (vx != 0 || vy != 0);

    if (moving && !gordo_in_hop) {
      gordo_in_hop = true;
      gordo_hop_start_time = millis();
      gordo_hop_duration = (state == STATE_ZOOMIES_OVERLAY) ? 50 : 210;
    }
    if (gordo_in_hop) {
      if (millis() - gordo_hop_start_time > gordo_hop_duration)
        gordo_in_hop = false;
    }

    gordo_x += vx; gordo_y += vy;
    if (gordo_x < 0) gordo_x = 0;
    if (gordo_x > SCREEN_W-20) gordo_x = SCREEN_W-20;
    if (gordo_y < 30) gordo_y = 30;
    if (gordo_y > SCREEN_H-16-GORDO_H/2) gordo_y = SCREEN_H-16-GORDO_H/2;

    if (needLivingroomRedraw) {
      tft.fillScreen(TFT_BLACK);
      drawSectionalCouch();
      drawGlassTable();
      drawBlanketLeft();
      needLivingroomRedraw = false;
    }
    if (moving && !gordo_hidden) {
      int bx = int(prev_x) - GORDO_W/2 - 18;
      int by = int(prev_y) - GORDO_EAR_ABOVE - 14;
      int bw = GORDO_W + 36;
      int bh = GORDO_H + GORDO_EAR_ABOVE + GORDO_FOOT_BELOW + 32;
      tft.fillRect(bx, by, bw, bh, TFT_BLACK);
      drawSectionalCouchPartial(bx, by, bw, bh);
      drawGlassTablePartial(bx, by, bw, bh);
      drawBlanketLeft();
    }

    int hop_frame = gordo_in_hop ? 1 : 0;
    float hop_offset = hop_frame ? GORDO_HOP_DY : 0;
    bool all_bars_full = (poop_bar >= POOP_MAX && hay_bar >= HAY_MAX && water_bar >= WATER_MAX);

    // Treat time at bottom
    if ((state == STATE_LIVINGROOM || state == STATE_TREATFLASH) && all_bars_full && !treat_time_started) {
      treat_time_started = true;
      show_meter = true;
      carrots_eaten = 0;
      treat_present = false;
      treat_meter_delay = true;
      treat_meter_delay_start = millis();
      tubes_eaten = 0;
      show_tube_meter = false;
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.fillRect(0,SCREEN_H-60, SCREEN_W,60, TFT_BLACK);
      tft.drawString("Treat Time!", SCREEN_W/2, SCREEN_H-34, 4);
      treat_time_msg_ms = millis();
      state = STATE_TREATFLASH;
      return;
    }
    if (state == STATE_TREATFLASH) {
      if (millis()-treat_time_msg_ms>1100) {
        show_meter = true;
        treat_meter_delay = true;
        treat_meter_delay_start = millis();
        state = STATE_LIVINGROOM;
        tft.fillRect(0,SCREEN_H-60,SCREEN_W,60,TFT_BLACK);
        return;
      }
      delay(14); return;
    }

    if (show_meter && carrots_eaten < 3) {
      drawTreatMeter(carrots_eaten, false);
      if (treat_meter_delay && millis()-treat_meter_delay_start>220) {
        if (!treat_present) spawnCarrot();
        treat_meter_delay = false;
      }
    }
    if (carrots_eaten < 3 && treat_present) drawCarrot(current_treat_x, current_treat_y, 28, false);

    drawGordo(gordo_x, gordo_y+hop_offset, hop_frame, gordo_hidden);

    if (show_meter && carrots_eaten < 3 && treat_present) {
      int dx = gordo_x-current_treat_x;
      int dy = gordo_y-current_treat_y;
      if ((dx*dx+dy*dy) < 1600) {
        treat_present = false;
        carrots_eaten += 1;
        if (carrots_eaten >= 3) {
          show_meter = false;
          zoomies_start_time = millis();
          state = STATE_ZOOMIES_OVERLAY;
          print_zoomies_message = false;
          zoomies_displayed = false;
          tubes_eaten = 0;
          tubes_phase_started = false;
          return;
        } else {
          delay(160);
          spawnCarrot();
        }
      }
    }

    if (state == STATE_ZOOMIES_OVERLAY) {
      drawTreatMeter(3, false);
      drawGordo(gordo_x, gordo_y+hop_offset, hop_frame, false);
      int tx = SCREEN_W/2, ty = SCREEN_H-34;
      if (!print_zoomies_message) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.fillRect(0, ty-30, SCREEN_W, 60, TFT_BLACK);
        tft.setTextFont(7);
        tft.drawString("ZOOMIES!", tx, ty, 4);
        tft.setTextFont(2);
        zoomies_print_time = millis();
        print_zoomies_message = true;
        return;
      }
      if (!zoomies_displayed && millis()-zoomies_print_time >= 1000) {
        tft.fillRect(0, ty-30, SCREEN_W, 60, TFT_BLACK);
        zoomies_displayed = true;
        zoomies_start = millis();
      }
      if (zoomies_displayed && millis()-zoomies_start > 5000) {
        show_tube_meter = true;
        tubes_eaten = 0;
        tube_present = false;
        tube_meter_delay = true;
        tube_meter_delay_start = millis();
        state = STATE_TUBE;
        gordo_hidden = false;
        return;
      }
      delay(16);
      return;
    }

    if (state == STATE_TUBE) {
      drawTreatMeter(tubes_eaten, true);
      if (show_tube_meter && tubes_eaten < 3) {
        if (tube_meter_delay && millis()-tube_meter_delay_start>220) {
          if (!tube_present) spawnTube();
          tube_meter_delay = false;
        }
      }
      if (tubes_eaten < 3 && tube_present) drawTube(current_treat_x, current_treat_y, 22, 60, false);

      drawGordo(gordo_x, gordo_y+hop_offset, hop_frame, gordo_hidden);

      if (show_tube_meter && tubes_eaten < 3 && tube_present) {
        int dx = gordo_x-current_treat_x;
        int dy = gordo_y-current_treat_y;
        if ((dx*dx+dy*dy) < 2500) {
          tube_present = false;
          tubes_eaten += 1;
          if (tubes_eaten >= 3) {
            // Hop to couch and disappear, then nap
            while (gordo_x < SCREEN_W-COUCH_THICKNESS-24) {
              clearGordoRegion(gordo_x, gordo_y);
              gordo_x += 12;
              gordo_in_hop = true;
              drawGordo(gordo_x, gordo_y, 1, false);
              delay(36);
              clearGordoRegion(gordo_x, gordo_y);
              gordo_in_hop = false;
              drawGordo(gordo_x, gordo_y, 0, false);
              delay(22);
            }
            clearGordoRegion(gordo_x, gordo_y);
            gordo_hidden = true;
            nap_time_started = millis();
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.fillRect(0,SCREEN_H-60, SCREEN_W,60, TFT_BLACK);
            tft.drawString("Nap time....", SCREEN_W/2, SCREEN_H-34, 4);
            state = STATE_NAPTIME;
            return;
          } else {
            delay(200);
            spawnTube();
          }
        }
      }
      delay(14);
      return;
    }

    // Nap time; reset bars after nap - can play again
    if (state == STATE_NAPTIME) {
      if (nap_time_started == 0) nap_time_started = millis();
      if (millis() - nap_time_started > 5000) {
        tft.fillRect(0,SCREEN_H-60, SCREEN_W,60, TFT_BLACK);
        nap_time_started = 0;
        poop_bar = 0; hay_bar = 0; water_bar = 0; // RESET BARS
        state = STATE_LIVINGROOM;
        gordo_hidden = false;
        tubes_eaten = 0;
        tubes_phase_started = false;
        show_tube_meter = false;
        tube_present = false;
        tube_meter_delay = false;
        needLivingroomRedraw = true;
        return;
      }
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.fillRect(0,SCREEN_H-60, SCREEN_W,60, TFT_BLACK);
      tft.drawString("Nap time....", SCREEN_W/2, SCREEN_H-34, 4);
      delay(14);
      return;
    }

    int towel_warp_end = BLANKET_W/3;
    if (gordo_x < towel_warp_end && gordo_y > BLANKET_Y-18 && gordo_y < BLANKET_Y+BLANKET_H+18) {
      tft.fillScreen(TFT_BLACK);
      drawMainScreen();
      gordo_x = SCREEN_W - BLANKET_W/2;
      gordo_y = BLANKET_Y+BLANKET_H/2;
      delay(120);
      state = STATE_GAME;
      needLivingroomRedraw = true;
      treat_time_started = false;
      treat_present = false;
      carrots_eaten = 0;
      tubes_eaten = 0;
      show_meter = false;
      show_tube_meter = false;
      tube_present = false;
      tube_meter_delay = false;
      tubes_phase_started = false;
      livingRoomJustOpened = false;
      gordo_hidden = false;
      return;
    }
    delay(14);
    return;
  }
}