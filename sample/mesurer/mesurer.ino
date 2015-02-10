/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;
int X_mesure = 7; // Cross mesure: ligne de transmission commune
float tension;
int amp;



// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  //Serial.println("nouvelle boucle");
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(500);               // wait for a second
  tension = analogRead(X_mesure);
  amp = float(tension);
  if(true)
  {
    Serial.print(micros());
    String msg = "Tension :  ";
    Serial.print(msg);
    Serial.println(amp,10);
  }
}
