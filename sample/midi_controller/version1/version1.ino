int firstAnIn = A4;
int pinClockShift = 3;
int pinClockData = 4;
int pinClockLock = 5;
int pinPlus = 7;
int clockDelay = 0;
int mesureWait = 0;
int loopWait = 0;
int const nb = 8;
float tension;
int amp;
int values[nb];


void setup()
{
  Serial.begin(9600);
  /*for(int i = 0; i < nb; i++)
  {
    pinMode(firstPin+i, OUTPUT);
  }
  pinMode(led, OUTPUT);*/
  pinMode(pinClockShift, OUTPUT);
  pinMode(pinClockData, OUTPUT);
  pinMode(pinClockLock, OUTPUT);
  pinMode(pinPlus, OUTPUT);
  digitalWrite(pinPlus, LOW);
  delay(5000);
  digitalWrite(pinPlus, HIGH);
  //analogReference(firstAnIn);
  for(int i = 0; i < nb; i++)
  {
    //values[i] = analogRead(firstAnIn+i);
    values[i] = 0;//values[i]*5/1024;
  }
}

void loop()
{
  int current = 0;
  while(current < nb)
  {
    setByte(current);
    tension = analogRead(firstAnIn);
    amp = int(tension*float(127)/float(1024));
    if(amp != values[current] && (amp < (values[current]-5) || amp > (values[current]+5)))
    {
      values[current] = amp;
      noteOn(0xB0, current+0x00, amp+0x00);
    }
    delay(mesureWait);
    current++;
  }
  delay(loopWait);
}

void noteOn(int cmd, int pitch, int velocity) {
  
  /*String msg = "MIDI amp for ";
  msg = msg+current+" : ";
  Serial.print(msg);
  Serial.print(amp,10);
  Serial.print(" ");
  Serial.print(amp+0x1E,10);
  Serial.println(" ");
  //noteOn(0x90, current+0x1E, amp+0x00);
  //noteOn(0x90, 0x5A, amp+0x00);*/
  //Serial.write(cmd);
  //Serial.write(pitch);
  //Serial.write(velocity);
  int truc = 123;
  Serial.print(cmd);
  Serial.print(" - ");
  Serial.print(pitch);
  Serial.print(" - ");
  Serial.println(velocity);
}

void setByte(int current){
  /*Serial.print("setByte");
  Serial.print(" - ");
  Serial.print(current);
  Serial.print(" - ");*/
  for(int i = 7; i >= 0 ; i--)
  //for(int i = 0; i <= 7 ; i++)
  {
    /*if(bitRead(current, i))<
    {
      Serial.print("1");
    }else
    {
      Serial.print("0");
    }*/
    clockWrite(bitRead(current, i));
  }
  //Serial.print(" | ");
  digitalWrite(pinClockLock, LOW);
  delay(clockDelay);
  digitalWrite(pinClockLock, HIGH);
}

void clockWrite(boolean b){
  digitalWrite(pinClockData, b);
  //digitalWrite(pinClockData, HIGH);
  digitalWrite(pinClockShift, HIGH);
  delay(clockDelay);
  digitalWrite(pinClockShift, LOW);
  delay(clockDelay);
}
