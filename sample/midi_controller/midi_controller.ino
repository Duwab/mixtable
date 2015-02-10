int firstPin = 8;
int firstAnIn = 4;
int nb = 2;
int led = 13;
float tension;
int amp;
int values[2];


void setup()
{
  Serial.begin(9600);
  //Serial.begin(31250);
  for(int i = 0; i < nb; i++)
  {
    pinMode(firstPin+i, OUTPUT);
  }
  pinMode(led, OUTPUT);
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
    for(int i = 0; i < nb; i++)
    {
      if(i == current)
      {
        digitalWrite(firstPin+i, HIGH);
      }else
      {
        digitalWrite(firstPin+i, LOW);
      }
    }
    tension = analogRead(firstAnIn+current);
    amp = int(tension*float(127)/float(1024));
    if(amp != values[current])
    {
      values[current] = amp;
      String msg = "MIDI amp for ";
      msg = msg+current+" : ";
      /*Serial.print(msg);
      Serial.print(amp,10);
      Serial.print(" ");
      Serial.print(amp+0x1E,10);
      Serial.println(" ");*/
      //noteOn(0x90, current+0x1E, amp+0x00);
      //noteOn(0x90, 0x5A, amp+0x00);
      noteOn(0xB0, current+0x00, amp+0x00);
    }
    delay(10);
    /*if(current == 0)
    {
      digitalWrite(led, HIGH);
    }else
    {
      digitalWrite(led, LOW);
    }*/
    current++;
  }
}

void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}
