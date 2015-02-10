//exemple 1: +500 en 800µs

int firstAnIn = 4;
int pin_cue = 4;
int d = 2; //delta entre deux valeurs pour que la mseure soit prise en compte pour le serial
float tension;
int amp;
int values[2];
int print_indic = 0;


// constantes pour les capteurs
int nb = 1;
double derived = (double) 2000/800;  // derived at the beginning of a hit
long hit_lapse = 50000; // time after a hit with huge oscillations
long hit_still = 100000; //time for clean


// infos sur les sensors
boolean array_is_hit[1];            // est dans une phase de hit
boolean array_is_growing_phase[1];  // est dans la phase montante du hit
int array_last_tension[1];          // la derniere tension substancielle (stable ou la crête d'une oscillation)
long array_last_time[1];            // temps de la dernière mesure
long array_last_hit_time[1];
int velocity;



void setup()
{
  //Serial.begin(9600);
  Serial.begin(31250);
  for(int i = 0; i < nb; i++)
  {
    values[i] = 0;//values[i]*5/1024;
    array_is_hit[i] = false;
    array_last_tension[i] = 1024;
    array_last_time[i] = 0;
  }
  //Serial.println(derived);
  pinMode(pin_cue, INPUT);
  pinMode(13, OUTPUT);
}

void loop()
{
  int current = 0;
  while(current < nb)
  {
    /*tension = analogRead(firstAnIn+current);
    amp = float(tension);
    // mesure
    if(digitalRead(pin_cue) && ((amp < values[current] - d) || (amp > values[current] +d)))
    {
      //String msg = "Tension :  ";
      //msg = msg+amp;
      //Serial.print(msg);
      Serial.print(micros());
      Serial.print("ms : ");
      Serial.println(amp,10);
      /*Serial.print(" ");
      Serial.print(amp+0x1E,10);
      Serial.println(" ");* /
      //noteOn(0x90, current+0x1E, amp+0x00);
      //noteOn(0x90, 0x5A, amp+0x00);
      values[current] = amp;
    }*/
    sensor_action(current);
    current++;
  }
  //delay(1000);
  //Serial.println(" ");
}

void sensor_action(int i)
{
  // mesure et actions a faire pour le n°i
  //Serial.print("sensor action for i = ");
  //Serial.prinln(i);
  int pin_nb = i + firstAnIn;
  float V_fl = analogRead(pin_nb);
  int V = float(V_fl);
  //Serial.print(V);
  long t = micros();
  
  /*Serial.print(t);
  Serial.print("ms : ");
  Serial.println(V,10);*/
  
  if(!is_already_hit(i, V))
  {
    if(is_new_hit(i, V, t))
    {
      array_is_hit[i] = true;
      array_is_growing_phase[i] = true;
      array_last_hit_time[i] = t;
      //noteOn(0x90, 0x3C, 0x00);
      //noteOn(0x90, 0x3C, velocity);
      /*Serial.print("hit ");
      Serial.print(V);
      Serial.print(" ");
      Serial.print(t);
      Serial.print(" ");
      Serial.print(print_indic++);
      if(print_indic > 9)
        print_indic = 0;*/
      digitalWrite(13, HIGH);
    }
    array_last_tension[i] = V;
    array_last_time[i] = t;
  }else
  {
    //array_is_growing_phase
    if(array_is_growing_phase[i])
    {
      if(V > array_last_tension[i])
      {
        array_last_tension[i] = V;
      }else if(t - array_last_time[i] > 10000)
      {
        velocity = round(float(max(array_last_tension[i] - 200, 0))/(1024 - 200)*126);
        noteOn(0x90, 0x3C, 0x00);
        noteOn(0x90, 0x3C, velocity);
        array_is_growing_phase[i] = false;
      }
    }
    if(is_ended_hit(i, V, t))
    {
      array_is_hit[i] = false;
      digitalWrite(13, LOW);
      //noteOn(0x90, 0x1E, 0x00);
      //Serial.println("end");
      array_last_time[i] = t;
      array_last_tension[i] = V;
    }
  }
}

boolean is_already_hit(int i, int V)
{
  if(!array_is_growing_phase[i] && V > array_last_tension[i])
    return false;
  return array_is_hit[i];
}

boolean is_new_hit(int i, int V, long t)
{
  
  double current_derived =  (double) (V - array_last_tension[i]) / (t - array_last_time[i]);
  /*if(false && current_derived > derived)
  {
    Serial.println(" ");
    Serial.println(i);
    Serial.print(V);
    Serial.print(" - ");
    Serial.println(array_last_tension[i]);
    Serial.print(t);
    Serial.print(" - ");
    Serial.println(array_last_time[i]);
    Serial.print(current_derived);
    Serial.print(" > ");
    Serial.println(derived);
  }*/
  //velocity = (int) min(max(30, round(10 * 126 * (current_derived - derived))), 126);
  if(t - array_last_hit_time[i] > hit_still)
    return 10 * current_derived > derived;
  else
    return current_derived > derived;
}

boolean is_ended_hit(int i, int V, long t)
{
  return (array_last_tension[i] > V) && (t-array_last_time[i] > hit_lapse);
}

void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
  /*Serial.println(velocity);
  */
}


