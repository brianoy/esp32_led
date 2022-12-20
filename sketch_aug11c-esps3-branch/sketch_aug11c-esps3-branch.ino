//最新版2022/8/12
//app inventor 拉桿並沒有touch up 的功能 於是就直接讓一大串資訊先傳進來，在裡面再分類及挑選
//使用micro usb插電腦不可使電流超過1A，即255,255,255的brightness不可超過100

/*
COOLING低SPARKING低:整根白 整根紅 甚至會出現全部熄滅的交替狀態
COOLING低SPARKING高:整根都會是白色的 因為不好熄滅又有很多火
COOLING高SPARKING高:100/210感覺是比例關係 跟50/120感覺差不多 最底下幾顆閃爍快速
COOLING高SPARKING低:火很難爬到頂端 快熄滅的火 很難再發起來 紅黑閃爍交替
35/50 黃色的地方會白色
*/
#include <EEPROM.h>
#include "BluetoothSerial.h" 
#include "FastLED.h"

#define MAX_NUM_LIGHT 500 //寫死
#define DATA_PIN 22 //esp32-s3不可在pin22上
#define BUTTON_PIN  27
#define YELLOWKTEMP 0xFF6C00
#define BLUEKTEMP 0xA9C0FF
#define SATURATION 255
#define BRIGHTNESS  180
#define FRAMES_PER_SECOND 60  //120?閃爍的時候眼睛會不舒服 需要多次調試 跟實體LED長度比較有關 越長需要越快的更新率

DEFINE_GRADIENT_PALETTE(aurora_gp) {
   0,     0,   100,    0,  //black green
   89,    20,  232,  30,   //dark green
   135,   0,   234,  141,  //bright green
   175,   1,   126,  213,  //blue
   225,   190, 61,   255,  //purple
   255,   141, 1,    196   //dark purple
};

BluetoothSerial SerialBT;
CRGB leds[MAX_NUM_LIGHT];//要注意，如果要調整燈光數量，max_num_light必須要為最大值，否則卡bug
CRGBPalette16 aurora_object = aurora_gp;

int COOLING = 35;//DEFALT VALUE
int SPARKING = 50;//DEFALT VALUE
int allR = 0;
int allG = 0;
int allB = 0;
int bright = 0;
int counter = 1;
int button_state = 0;
int num_led;//LED的數目
String eeprom_numled = "";
uint8_t brightness = 200;
uint8_t maxChanges = 24; 
bool gReverseDirection = false;
/*//sketch too big
#include <WiFi.h>
*/



void setup() {
  EEPROM.begin(4096);//寫死申請4096個byte位子 實際上沒有用那麼多(目前1~5位為LED燈珠)
  pinMode(BUTTON_PIN, INPUT_PULLUP);//按鈕，內建上拉電阻，當按下按鈕，變為低電平
  SerialBT.begin("brianoyLED");
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, MAX_NUM_LIGHT).setCorrection( TypicalLEDStrip );
  //創立一個叫做leds的物件(容器)，.setCorrection是fastled自己建立的，嘗試將藍色及綠色的亮度做細微的調整
  for (int i = 0; i < 4; ++i) //四位字串
    {
      eeprom_numled += char(EEPROM.read(i));
    }
  num_led = eeprom_numled.toInt();
}




void loop() {
  //Serial.println(SerialBT.available());//defalt:0 直到device傳資料:為2
  while (SerialBT.available() > 0) {//當藍芽連接>0，並不見得是1
    String dataIn = SerialBT.readString();//藍芽輸入暫存
    Serial.println(dataIn);
    if (dataIn == "ALL_WHITE"){//全白
        Serial.println("all white");
        for( int i = 0; i < MAX_NUM_LIGHT; i++) {
          leds[i] = CRGB::White;
        }
        FastLED.show();
        dataIn = "";
     }
    else if (dataIn.startsWith("ALL_COLOR")) {
       if (dataIn.length() > 19){
          dataIn = dataIn.substring(dataIn.lastIndexOf("ALL_COLOR"), dataIn.length());//取最後一個傳入的ALL_COLOR
       }
       Serial.println("recognized" + dataIn);
       delay(10);
       FastLED.clear(true);
       String stringR = dataIn.substring(dataIn.indexOf("R") + 2, dataIn.indexOf("G"));//+2因為COLOR的R也會被計算到
       allR = stringR.toInt();
       if (allR < 0){allR =0;}
       String stringG = dataIn.substring(dataIn.indexOf("G") + 1, dataIn.indexOf("B"));
       allG = stringG.toInt();
       if (allG < 0){allG =0;}
       String stringB = dataIn.substring(dataIn.indexOf("B") + 1, dataIn.indexOf("E"));
       allB = stringB.toInt();
       if (allB < 0){allB =0;}
       String stringE = dataIn.substring(dataIn.indexOf("E") + 1, dataIn.indexOf("X"));
       bright = stringE.toInt();
       if (bright < 0){bright = 0 ;}
       Serial.println(String(num_led) + " lights");
       for (int i = 0; i <= num_led-1; i++) {
        leds[i] = CRGB( allR, allG, allB);
       }
       FastLED.setBrightness(bright);
       FastLED.show();
       }
    else if (dataIn.startsWith("ALL_BLACK")){//全黑
        all_black();
      }
    else if (dataIn.startsWith("LIGHT_CHANGE")) {//亮度調整
      if (dataIn.length() > 13){//13個字元
         dataIn = dataIn.substring(dataIn.lastIndexOf("LIGHT_CHANGE"), dataIn.length());//取最後一個傳入的ALL_COLOR
      }
      Serial.println("recognized" + dataIn);
      String stringBrightness = dataIn.substring(dataIn.indexOf("LIGHT_CHANGE") + 12, dataIn.length());//lightchange有12位
      brightness = stringBrightness.toInt();
      Serial.println("light value:" + stringBrightness);
      FastLED.setBrightness(brightness);
      FastLED.show();
    }
    else if (dataIn.startsWith("FIRE")) {//火焰普通燈珠
      while (SerialBT.available() == 0 && button_state == HIGH) {
        //dataIn = SerialBT.readString();//造成嚴重卡頓
        Fire();
        FastLED.show(); // display this frame
        FastLED.delay(1000 / FRAMES_PER_SECOND);
      }
    }
    else if (dataIn.startsWith("CYLON")) {//彩虹前後撞
      while (SerialBT.available() == 0 && button_state == HIGH) {
        cylon();
      }
    }
    else if (dataIn.startsWith("RAINBOW_FLOW")) {//彩虹流動
      while (SerialBT.available() == 0 && button_state == HIGH) {
        rainbow_one_path();
      }
    }
    else if (dataIn.startsWith("RAINBOW_ALL_LIGHT")) {//彩虹全部流動
      while (SerialBT.available() == 0 && button_state == HIGH) {
        rainbow_all_light();
      }
    }
    
    else if (dataIn.startsWith("NUM_LIGHT")) {//改變燈的數量
      String stringNUM_LIGHT = dataIn.substring(dataIn.indexOf("NUM_LIGHT") + 9, dataIn.length());
      num_led = stringNUM_LIGHT.toInt();
      all_black();
      Serial.print("writing eeprom :");
      for (int i = 0; i < 4; ++i)
      {
        EEPROM.write(i, stringNUM_LIGHT[i]);
        Serial.println(stringNUM_LIGHT[i]);
      }
      EEPROM.commit();
    }
    
    else if (dataIn.startsWith("ANTI_GRAVITY")) {//改變火的噴射方向
      if (gReverseDirection == true){
        gReverseDirection = false;
      }
      else{
        gReverseDirection = true;
      }
    }
    
    else if (dataIn.startsWith("BULB")) {//改變燈的色溫
      if (dataIn.length() > 6){
         dataIn = dataIn.substring(dataIn.lastIndexOf("BULB"), dataIn.length());//取最後一個傳入的ALL_COLOR
      }
      Serial.println("recognized" + dataIn);
      String stringBULB = dataIn.substring(dataIn.indexOf("BULB") + 4, dataIn.length());
      int percentage = stringBULB.toInt();
      for (int i = 0; i <= num_led-1; i++) {
       leds[i] = blend( YELLOWKTEMP, BLUEKTEMP, percentage);  //percentage is 0-255
      }
      FastLED.setBrightness(180);
      FastLED.show();
    }
    
    else if (dataIn.startsWith("SOS")) {//SOS燈光
      while (SerialBT.available() == 0 && button_state == HIGH) {
        SOS();
      }
    }

    else if (dataIn.startsWith("WEED")) {//大麻光
      for (int i = 0; i <= num_led-1; i++) {
       leds[i] = CRGB( 255, 31, 240);
      }
      FastLED.setBrightness(200);
      FastLED.show();
    }

    else if (dataIn.startsWith("AURORA")) {//極光
      FastLED.setBrightness(240);
      while (SerialBT.available() == 0 && button_state == HIGH) {
        aurora_colorpicker();
      }
    }
  }
 button_state = digitalRead(BUTTON_PIN);
 if (button_state == LOW){//沒有debounce
   delay(350);
   Serial.println(counter);
   counter++;
   switch(counter){
    case 1:
      RGB_SHOW(0,0,0);
    break;
    case 2:
      RGB_SHOW(255,0,0);
    break;
    case 3:
      RGB_SHOW(0,255,0);
    break;
    case 4:
      RGB_SHOW(0,0,255);
    break;
    case 5:
      RGB_SHOW(255,255,0);
    break;
    case 6:
      RGB_SHOW(255,0,255);
    break;
    case 7:
      RGB_SHOW(0,255,255);
    break;
    case 8:
      RGB_SHOW(255,108,0);
    break;
    case 9:
      RGB_SHOW(255,255,255);
    break;
    case 10:
      RGB_SHOW(150,130,255);
      counter = 1;
    break;
    default:
      //SOMETHING
    break;
 }
 }
}




void RED(){
    for (int i = 0; i <= num_led-1; i++) {
      leds[i] = CRGB( 255, 0, 0);
      FastLED.setBrightness(255);
    }
      FastLED.show();
}



void GREEN(){
    for (int i = 0; i <= num_led-1; i++) {
      leds[i] = CRGB( 0, 255, 0);
      FastLED.setBrightness(255);
    }
      FastLED.show();
}


void BLUE(){
    for (int i = 0; i <= num_led-1; i++) {
      leds[i] = CRGB( 0, 0, 255);
      FastLED.setBrightness(255);
    }
      FastLED.show();
}



void RGB_SHOW(int R, int G, int B){
    for (int i = 0; i <= num_led-1; i++) {
      leds[i] = CRGB( R, G, B);
      FastLED.setBrightness(140);
    }
      FastLED.show();
}



void SOS(){
  for (int i = 0; i <= 3; i++) {
    RED();
    FastLED.delay(50);
    all_black();
    delay(5);
  }
  delay(250);
  for (int i = 0; i <= 3; i++) {
    BLUE();
    FastLED.delay(40);
    all_black();
    delay(5);
  }
  delay(50);
}



void all_black(){
  FastLED.clear(true);
  FastLED.show();
}


void Fire()
{
  static uint8_t heat[MAX_NUM_LIGHT];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < num_led; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / num_led) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= num_led - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < num_led; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (num_led-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}



void fadeall() { 
  for(int i = 0; i < num_led; i++) { 
    leds[i].nscale8(250); 
  } 
}



void cylon() { 
  static uint8_t hue = 0;
  for(int i = 0; i < num_led; i++) {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show(); 
    fadeall();
    delay(10);
  }
  for(int i = (num_led)-1; i >= 0; i--) {
    leds[i] = CHSV(hue++, 255, 255);
    FastLED.show();
    fadeall();
    delay(10);
  }
}


void rainbow_one_path() {
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < num_led; i++) {
      leds[i] = CHSV(i * 2 - (j * 2), SATURATION, BRIGHTNESS); /* The higher the value 4 the less fade there is and vice versa */ 
    }
    FastLED.show();
    delay(25); /* Change this to your hearts desire, the lower the value the faster your colors move (and vice versa) */
  }
}



void rainbow_all_light() {
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < num_led; i++) {
      leds[i] = CHSV(j*2, SATURATION, BRIGHTNESS); /* The higher the value 4 the less fade there is and vice versa */ 
    }
    FastLED.show();
    delay(25); /* Change this to your hearts desire, the lower the value the faster your colors move (and vice versa) */
  }
}


void aurora_colorpicker(){//讓藍色及紅色比較少出現 以及出現時會限制波的長度較短
  uint8_t color_picker = random8();//先選擇是5個顏色的哪一個顏色
  uint8_t color;//在範圍內選擇適當的中央色 
  uint8_t wavesize;//選擇一次波的長度 
  if      (0<color_picker and color_picker<130)   {wavesize = random8(10,45); color = random8(14,75);}    //black green  - dark green
  else if (130<color_picker and color_picker<170) {wavesize = random8(10,45); color = random8(103,121);}  //dark green   - bright green
  else if (170<color_picker and color_picker<200) {wavesize = random8(5,30);  color = random8(145,165);}  //bright green - blue
  else if (200<color_picker and color_picker<225) {wavesize = random8(5,20);  color = random8(181,219);}  //blue         - purple
  else                                            {wavesize = random8(5,20);  color = random8(231,249);}  //purple       - dark purple
  aurora(wavesize, color);
}


void aurora(uint8_t wavesize ,uint8_t color) {//發生器
  for (int i = 0; i < wavesize; i++) {
    for (int j = num_led-1; j >= 1 ; j--) {//先將所有的像素往後移一格
      leds[j] = (leds[j-1] + leds[j-2])/2;
    }
    leds[1] = leds[0];
    leds[0] = ColorFromPalette(aurora_object, color+0.3*i, 55+(200/wavesize*i));
    //Serial.println(leds[0]);
    FastLED.delay(25);
  }
  for (int k = 0; k < wavesize; k++) {
    for (int l = num_led-1; l >= 1; l--) {//先將所有的像素往後移一格
      leds[l] = (leds[l-1] + leds[l-2])/2;
    }
    leds[1] = leds[0];
    leds[0] = ColorFromPalette(aurora_object, color-0.3*k, 255-(200/wavesize*k));
    FastLED.delay(25);
  }
}
