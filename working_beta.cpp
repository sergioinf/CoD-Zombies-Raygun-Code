// Libraries
#include <Adafruit_NeoPixel.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


//DFPlayer initialization
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
/*
On sound    - 0, 1
Open sound  - 2
Close sound - 3
Shoot sound - 4
Empty sound - 5
*/



// LEDs
#define DIAL_PIN 8	        // input pin dial Neopixel is attached to
#define BARREL_PIN 9        // input pin base Neopixel is attached to
#define NUMPIXELS_DIAL 3    // number of neopixels in dial
#define NUMPIXELS_BARREL 18 // number of neopixels in barrel

Adafruit_NeoPixel dial = Adafruit_NeoPixel(NUMPIXELS_DIAL, DIAL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel base = Adafruit_NeoPixel(NUMPIXELS_BARREL, BARREL_PIN, NEO_GRB + NEO_KHZ800);



//Raygun colors
bool baseColor[] = {false, false, false, false};
long potentiometer;
int luminosity[] = {255, 135, 55, 5, 95, 195};
bool rising[] = {false, true, true, false, true, true};

//Shooting
int disparosActuales = 0;
int disparosMax = 20;
bool opened = false;
bool shooting = false;
const int timeThreshold = 200;
long startTime = 0;


void setup(){
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), setShoot, FALLING);
  
  // WIP
  //pinMode(3, INPUT);
  //attachInterrupt(digitalPinToInterrupt(3), AbreRecamara, RISING);
  //attachInterrupt (digitalPinToInterrupt (3), CierraRecamara, DOWN);

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  Serial.print(F("Number of files in SD: "));
  Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
 

  //Set volume
  myDFPlayer.volume(30);  //Set volume value (0~30).
  //Set different EQ
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  //Set device we use SD as default
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  
  //LEDs start
  dial.begin();
  base.begin();
  
  //Turn on green dial led
  dial.setPixelColor(0, dial.Color(0, 255, 0));
  //Turn on yellow dial led
  dial.setPixelColor(1, dial.Color(255, 255, 0));
  //Turn on red dial led
  dial.setPixelColor(2, dial.Color(255, 0, 0));
  dial.show(); // This sends the updated pixel color to the hardware.

  //Turning on barrel leds
  potentiometer = analogRead(A0);
  if (potentiometer < 250){
    baseColor[0] = true;
  }else if(potentiometer >= 250 && potentiometer < 500){
    baseColor[1] = true;
  }else if(potentiometer >= 500 && potentiometer < 750){
    baseColor[2] = true;
  }else{
    baseColor[3] = true;
  }
  
  barrelInit();
  
}

void loop(){
  
  Serial.println(digitalRead(3));
  if(digitalRead(3)==1 && !opened){
    myDFPlayer.play(2);  //Play the open chamber sound
    opened = true;
  }else if(digitalRead(3)==0 && opened){
    myDFPlayer.play(3);  //Play the close chamber sound
    disparosActuales = 0;
    opened = false;
  }

  if(shooting){
    shoot();
  }
  
  readPotentiometer();
  //Es un problema que este proceso este ocupando el procesador de la placa
  changeBarrelColor();
}

void setShoot(){
  if (millis() - startTime > timeThreshold)
  {
    shooting = true;
    startTime = millis();
  }
}

void shoot(){
  //First it checks if the chamber is not open
  if (!opened){
    // The second checking is if I have rounds left.
    if(disparosActuales != disparosMax){
      myDFPlayer.play(4);  //Play the shoot sound
      // If I have, it shoots as normal.
      disparosActuales += 1;
      Serial.println(disparosActuales);
      Serial.println(F("Entro"));
      

      if (disparosActuales == (disparosMax/3)){
        //Apaga el led verde
        dial.setPixelColor(0, dial.Color(0, 0, 0)); // Moderately bright green color.
        dial.show(); // This sends the updated pixel color to the hardware.
      }else if(disparosActuales == (disparosMax/3)*2){
        //Apaga el led amarillo
        dial.setPixelColor(1, dial.Color(0, 0, 0)); // Moderately bright green color.
        dial.show(); // This sends the updated pixel color to the hardware.
      }else if(disparosActuales == disparosMax){
        //Apaga el led rojo
        dial.setPixelColor(2, dial.Color(0, 0, 0)); // Moderately bright green color.
        dial.show(); // This sends the updated pixel color to the hardware.
      }
    }else{
      // If not, it just play the empty sound.
      myDFPlayer.play(5);  //Play the empty sound
    }
  }
  shooting = false;
}

void readPotentiometer(){
  if (analogRead(A0) != potentiometer){
    potentiometer = analogRead(A0);
    if (potentiometer < 250){
      baseColor[0] = true;
      baseColor[1] = false;
      baseColor[2] = false;
      baseColor[3] = false;
    }else if(potentiometer >= 250 && potentiometer < 500){
      baseColor[0] = false;
      baseColor[1] = true;
      baseColor[2] = false;
      baseColor[3] = false;
    }else if(potentiometer >= 500 && potentiometer < 750){
      baseColor[0] = false;
      baseColor[1] = false;
      baseColor[2] = true;
      baseColor[3] = false;
    }else{
      baseColor[0] = false;
      baseColor[1] = false;
      baseColor[2] = false;
      baseColor[3] = true;
    }
  }
}
void barrelInit(){
  int entreLed[] = {6, 10,16, 23, 32, 67};
  
  for(int i=(NUMPIXELS_BARREL/3)-1; i>=0; i--){

    if(i == 3){
      //Play the startup sound
      myDFPlayer.play(0);
    }

    for(int j=0; j<=i; j++){
      if (baseColor[0]){
        base.setPixelColor(j, dial.Color(0, 0, 255));
      	base.setPixelColor(j+6, dial.Color(0, 0, 255));
      	base.setPixelColor(j+12, dial.Color(0, 0, 255));
      }else if (baseColor[1]){
        base.setPixelColor(j, dial.Color(0, 255, 0));
        base.setPixelColor(j+6, dial.Color(0, 255, 0));
        base.setPixelColor(j+12, dial.Color(0, 255, 0));
      }else if (baseColor[2]){
        base.setPixelColor(j, dial.Color(255, 0, 0));
        base.setPixelColor(j+6, dial.Color(255, 0, 0));
        base.setPixelColor(j+12, dial.Color(255, 0, 0));
      }else{
        base.setPixelColor(j, dial.Color(random(1,255), random(1,255), random(1,255)));
        base.setPixelColor(j+6, dial.Color(random(1,255), random(1,255), random(1,255)));
        base.setPixelColor(j+12, dial.Color(random(1,255), random(1,255), random(1,255)));
      }
      base.show(); // This sends the updated pixel color to the hardware.
  
      delay(entreLed[i]); // El tiempo que tarda en apagar un led y encender otro
      
      if (j!=i){
        base.setPixelColor(j, dial.Color(0, 0, 0)); // Moderately bright green color.
        base.setPixelColor(j+6, dial.Color(0, 0, 0)); // Moderately bright green color.
        base.setPixelColor(j+12, dial.Color(0, 0, 0)); // Moderately bright green color.
      	base.show(); // This sends the updated pixel color to the hardware.
      }
    }
    delay(100+entreLed[i]*4);
  }
}

void changeBarrelColor(){
  for(int j=0; j<(NUMPIXELS_BARREL/3); j++){
    int color = luminosity[j];
    if (baseColor[0]){
      base.setPixelColor(j, dial.Color(0, 0, color)); // Moderately bright green color.
      base.setPixelColor(j+6, dial.Color(0, 0, color)); // Moderately bright green color.
      base.setPixelColor(j+12, dial.Color(0, 0, color)); // Moderately bright green color.
      base.show(); // This sends the updated pixel color to the hardware.
    }else if (baseColor[1]){
      base.setPixelColor(j, dial.Color(0, color, 0)); // Moderately bright green color.
      base.setPixelColor(j+6, dial.Color(0, color, 0)); // Moderately bright green color.
      base.setPixelColor(j+12, dial.Color(0, color, 0)); // Moderately bright green color.
      base.show(); // This sends the updated pixel color to the hardware.
    }else if (baseColor[2]){
      base.setPixelColor(j, dial.Color(color, 0, 0)); // Moderately bright green color.
      base.setPixelColor(j+6, dial.Color(color, 0, 0)); // Moderately bright green color.
      base.setPixelColor(j+12, dial.Color(color, 0, 0)); // Moderately bright green color.
      base.show(); // This sends the updated pixel color to the hardware.
    }else{
      base.setPixelColor(j, dial.Color(random(1,255), random(1,255), random(1,255))); // Moderately bright green color.
      base.setPixelColor(j+6, dial.Color(random(1,255), random(1,255), random(1,255))); // Moderately bright green color.
      base.setPixelColor(j+12, dial.Color(random(1,255), random(1,255), random(1,255))); // Moderately bright green color.
      base.show(); // This sends the updated pixel color to the hardware.
    }
    if (color == 255 || color == 5){
      rising[j] = !rising[j];
    }
    if (rising[j]){
      luminosity[j] = color+10;
    }else{
      luminosity[j] = color-10;
    }
    delay(20);
  }
}
