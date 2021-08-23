#include <TimerOne.h>

#include <Adafruit_NeoPixel.h>

#include <HCSR04.h>

#define echoPin 6
#define trigPin 7
#define echoPin2 4
#define trigPin2 5

#define CLK 10
#define DIO 9

#define averageLength 10

#define switchPin 2

#define stripLength 16
#define ledPin 8
#define hLedPin 11
#define hStripLength 4

#define timerSetInterval .1
#define parkingInterval .1
#define standbyInterval 5
#define waitingInterval 1

#define parkedTime 5
#define parkedTolerance 20

#define standbyTolerance 200

#define waitingTolerance 50

#define Brightness 100

float timerInterval = parkingInterval;

int mode = 3;
int timerCounter = 0;
int times = 1;
int parkingCounter = 0;

int vertDistLast = 0;
int vertDistLastLast = 0;

double target = 15;
double hTarget = 10;
double vertDist = 0;
double hDist = 0;


double a1 = 0;
double a2 = 0;
double a3 = 0;
double a4 = 0;
double a5 = 0;

boolean flash = true;

#include <Arduino.h>
#include <TM1637TinyDisplay.h>

// Define Digital Pins


// Initialize TM1637TinyDisplay
TM1637TinyDisplay display(CLK, DIO);


Adafruit_NeoPixel pixels(stripLength, ledPin, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel hPixels(hStripLength, hLedPin, NEO_GRB + NEO_KHZ800);

HCSR04 sense1(trigPin, echoPin);

HCSR04 sense2(trigPin2, echoPin2);

uint32_t vertColor;// = pixels.Color(0, 25 ,0);
uint32_t hColor;

int getVertDist()
{
  int x = sense1.dist();
  return x;
}

void updateVertDisp(int vertDist)
{
  double percent = target / vertDist;

  if (percent < .70)
  {
    vertColor = pixels.Color(Brightness, 0, 0);
  }
  else if (percent >= .70 && percent < .95)
  {
    vertColor = pixels.Color(Brightness, 0, 0);
  }
  else
  {
    vertColor = pixels.Color(0, Brightness, 0);
  }

  int numLit = (stripLength / 2) * percent;
  //Serial.println(percent);
  //pixels.clear();
  for (int i = 0; i <= numLit; i++)
  {
    pixels.setPixelColor(i, vertColor);
    pixels.setPixelColor(stripLength - i - 1, vertColor);
  }

  for (int j = numLit + 1; j < stripLength - numLit - 1; j++)
  {
    pixels.setPixelColor(j, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

double getAverage(double n)
{
  double average = 0;
  a1 = a2;
  a2 = a3;
  a3 = a4;
  a4 = a5;
  a5 = n;

  average = (a1 + a2 + a3 + a4 + a5) / 5;

  //Serial.print(a1);
  //Serial.print(" ");
  //Serial.print(a2);
  //Serial.print(" ");
  //Serial.print(a3);
  //Serial.print(" ");
  //Serial.print(a4);
  //Serial.print(" ");
  //Serial.println(a5);
  //Serial.println(average);
  return average;

}

int doAverage(char x)
{
  int avg = 0;

  if (x == 'H')
  {
    for (int j = 1; j < 7; j++)
    {
      avg = getAverage(getHDist());
    }
  }
  else
  {
    for (int j = 1; j < 7; j++)
    {
      avg = getAverage(getVertDist());
    }
  }

  return avg;

}

int getHDist()
{
  int x = sense2.dist();
  return x;
}

void updateHDisp(int hDist)
{
  double percent = hTarget / hDist;

  if (percent < .70 || percent > 1.3)
  {
    hColor = hPixels.Color(Brightness, 0, 0);
  }
  else if ((percent >= .70 && percent < .90) || (percent > 1.1 && percent <= 1.3))
  {
    hColor = hPixels.Color(Brightness, 0, 0);
  }
  else
  {
    hColor = hPixels.Color(0, Brightness, 0);
  }
  //Serial.println(percent);

  if (percent < .9)
  {
    hPixels.setPixelColor(3, hColor);
    hPixels.setPixelColor(2, hColor);
    hPixels.setPixelColor(1, pixels.Color(0, 0, 0));
    hPixels.setPixelColor(0, pixels.Color(0, 0, 0));
  }
  else if (percent > 1.1)
  {
    hPixels.setPixelColor(0, hColor);
    hPixels.setPixelColor(1, hColor);
    hPixels.setPixelColor(2, pixels.Color(0, 0, 0));
    hPixels.setPixelColor(3, pixels.Color(0, 0, 0));
  }
  else
  {
    hPixels.setPixelColor(0, hColor);
    hPixels.setPixelColor(1, hColor);
    hPixels.setPixelColor(2, hColor);
    hPixels.setPixelColor(3, hColor);
  }

  hPixels.show();
}


void doSwitch()
{

  hTarget = doAverage('H');

  Serial.println(hTarget);

  target = doAverage('V');

  Serial.println(target);
  Serial.println(" ");

  for(int i = 0; i<10; i++)
  {
    pixels.fill(pixels.Color(Brightness, 0, 0), 0, stripLength);
    hPixels.fill(pixels.Color(Brightness, 0, 0), 0, hStripLength);
    pixels.show();
    hPixels.show();
    delay(50);
    pixels.fill();
    hPixels.fill();
    pixels.show();
    hPixels.show();
    delay(50);
  }
}

void checkState()
{
  switch (mode)
  {
    case 1: //standby
      Serial.println("Standing By");
      times = standbyInterval/timerSetInterval;

      vertDist = doAverage('V');

      if((vertDist-target) > standbyTolerance)
      {
        mode = 2;
        vertDistLast = vertDist;
        times = waitingInterval/timerSetInterval;
        delay(60000);
      }
      
    break; 
    

    case 2: //waiting

      Serial.println("Waiting");

      times = waitingInterval/timerSetInterval;
      
      vertDist = doAverage('V');

      if(flash)
      {
        pixels.setPixelColor(0, pixels.Color(Brightness/2, 0, 0));
      }
      else
      {
        pixels.clear();
      }

      pixels.show();

      flash = !flash;

      Serial.println(abs(vertDist-vertDistLast));

      if((vertDist-vertDistLast) >= waitingTolerance && (vertDist-vertDistLastLast) >= waitingTolerance)
      {
        mode = 3;
        times = parkingInterval/timerSetInterval;
      }
      else
      {
        vertDistLastLast = vertDistLast;
        vertDistLast = vertDist;
      }
      
      
    break;
    

    case 3: //parking

      
      
      times = parkingInterval/timerSetInterval;
      vertDist = doAverage('V');
      hDist = doAverage('H');

      updateVertDisp(vertDist);
      updateHDisp(hDist);
      
      if(abs(vertDist-target) < parkedTolerance)
      {
        parkingCounter++;
        Serial.println("Parked");
      }
      else
      {
        parkingCounter = 0;
        Serial.println("Parking");
      }

      if(parkingCounter >= parkedTime/parkingInterval)
      {
        //pixels.fill(pixels.Color(0, 0, 0), 0, stripLength);
        //hPixels.fill(pixels.Color(0, 0, 0), 0, hStripLength);
        pixels.clear();
        hPixels.clear();
        pixels.show();
        hPixels.show();
        mode = 1;
        parkingCounter = 0;
      }
      
    break;
  }
}

void timer()
{

  if(timerCounter >= times)
  {
    checkState();
    timerCounter = 0;
  }
  else
  {
    timerCounter++;
  }

}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pixels.begin();
  hPixels.begin();
  delay(100);
  pixels.clear();
  hPixels.clear();
  display.setBrightness(0x0f);
  display.clear();

  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switchPin), doSwitch, RISING);

  Timer1.initialize(timerSetInterval*1000000);
  Timer1.attachInterrupt(timer);
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(10);
  //vertDist = getVertDist();
  //hDist = getHDist();
  //vertDist = int(getAverage(vertDist));
  //Serial.println(vertDist);
  //updateVertDisp(vertDist);
  //updateHDisp(hDist);
  //display.showNumber(vertDist);
  delay(1);

}
