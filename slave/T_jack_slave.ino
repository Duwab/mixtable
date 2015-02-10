int led = 13;
/*
  Status
*/
boolean is_master = false;  // comportement master ou slave
//boolean connected = false;  // si slave, précise si a été reconnu par master
int T_status = 0;
    // 0 => waiting
    // 1 => needs to speak
    // 2 => listening
    // 3 => speaking
boolean testing_sending_ON = false;

/*
  Communication
*/
int send_bit_pos;
int send_bit_length;
boolean send_bit_queue[64];
int recieved_bit_length;
int recieved_bytes[3];
boolean next_bit_clock = true;
byte msg_queue_pos_current = 0;
byte msg_queue_pos_available = 0;
byte msg_queue_array_id[8];
byte msg_queue_array_value[8];
byte test_msg = 00000000;


/*
  Time
*/
long timeout = 100000000; // max duration for a transfer phase
long phase_start = 0;  // start time of the current transfer phase
long current_time = 0;
long last_switch = 0; // l'instant du dernier changement de tension (pour l'envoi)
long switch_duration = 1000; //10


/*
  Tensions
*/
int pin_switch = 7;
int pin_V_m = 7; // Cross mesure: ligne de transmission commune
int V_nb = 5;
//int V_i[5] = {0, 200, 286, 423, 812}; //paliers V0, V1, ...
int V_i[5] = {0, 305, 572, 806, 900}; //paliers V0, V1, ... Via mesurer.arduino
//int V_i[5] = {0, 260, 471, 661, 812}; //paliers V0, V1, ...
int dV = 60;
int pins_tension[4] = {12, 11, 10, 9}; // V1, V2, V3, V4
int V_last = 0; //niveau tension la derniere fois qu'il y a eu une mesure (pour la réception)
float tension;
int amp;
int Vm = 0;
int Vm_last = 0;


/*
  Levels
*/
int loop_level = 0;  //le niveau que l'on incrémente dans la boucle
int self_level = 0;  //le niveau que le module propose à la zone commune
int Vm_level = 0;    //le niveau mesuré dans la zone de transmission commune
  
/*
  Mesures
*/
int all_sensors[128];


void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  int i = 0;
  while(i < V_nb)
  {
    pinMode(pins_tension[i], OUTPUT);
    digitalWrite(pins_tension[i], LOW);
    i++;
  }
  pinMode(pin_switch, INPUT);
  
  //status = waiting
  V_set(1);
  T_status = 0;
  
  i = 0;
  while(i < 3)
  {
    delay(1000);
    Serial.print(">");
    i++;
  }
  Serial.println(" Ready Slave");
}

void loop() {
  /*
    Loop Init
  */
  //delay(100);
  
  float ar = analogRead(pin_V_m);
  int V = float(ar);
  //log_number("Vm = ", V);
  Vm_level = tensionToLevel(pin_V_m);
  
  // XXX mettre if "mesure comprehensible" = level && stable
  
  /*
    Sensors
  */
  if(T_status == 0)
  {
    getSensors();
    if(digitalRead(pin_switch) && !is_master)
    {
      addToSend(12, 230);
    }
  }
  
  /*
    Bond
  */
  //log_number("Vm_level ", Vm_level);
  T_connection_status();  //change le statut en fonction de ce qui est mesuré/voulu
  
  
  /*
    Processing
  */
  T_send();        // si en mode envoi et qch à envoyer, envoie 1 bit
  T_listen();      // si en mode écoute, ...
  T_send_init();  // si possible et si nécessaire, prépare la liste des bools à envoyer
}

void getSensors(){
  int i = 0;
  while(i < 16)
  {
    all_sensors[i] = tensionToLevel(pin_V_m);
    i++;
  }
  //Serial.println("get mesures ok");
  /*int x = random(0, 127);
  Serial.print("all_sensors[");
  Serial.print(x, 10);
  Serial.print("] = ");
  Serial.println(all_sensors[x],10);*/
}

void addToSend(int id, int value){
  //ajoute une info à envoyer à la liste
  //on ajoute que si il reste de la place parmi les 8 valeurs en queue
  if((msg_queue_pos_current - 1) %8 != msg_queue_pos_available)
  {
    
    if(testing_sending_ON)
      return;
      
    Serial.println("can add something");
    msg_queue_array_id[msg_queue_pos_available] = id;
    //msg_queue_array_value[msg_queue_pos_available] = value;
    msg_queue_array_value[msg_queue_pos_available] = test_msg++;
    if(test_msg >= 128)
      test_msg = 0;
    msg_queue_pos_available = (msg_queue_pos_available+1)%8;
    
    testing_Has_to_send();
  }
}

void T_connection_status(){
  //si je dois écouter l'autre, alors ...
  //si j'ai qch à envoyer, ...
  //si j'ai fini
  if(T_status < 2 && ((is_master && Vm_level == 2) || (!is_master && Vm_level == 3)))
  {
    Serial.println("!R");
    T_status = 2;
    next_bit_clock = true; // on commence avec un switch
    V_set(4);
  }else if(T_status == 2 && self_level == 4)
  {
    Serial.println("ok");
    V_set(0);
  }else if(T_status == 1 && Vm_level == 4 && self_level != 4)  //Vm = 4, mais tant que pas interlocuteur, reste 1
  {
    //on peut passer en mode envoi
    Serial.println("!E");
    T_status = 3;
    next_bit_clock = true; // on commence avec un switch
  }
}

void T_send_init(){
  // S'il y a un message à envoyer dans la liste
  // shift du prochain message pour le traduire en liste de bits à envoyer
  
  //boolean send_bit_queue[64];
  if(T_status == 0 && msg_queue_pos_current != msg_queue_pos_available)
  {
    Serial.println("init!");
    send_bit_pos = 0;
    send_bit_length = 0;
    byte data = msg_queue_array_value[msg_queue_pos_current];
    byte mask = 1;
    int i = 0;
    while(i < 3)
    {
      for (mask = 10000000; mask>0; mask >>= 1) { //iterate through bit mask
        if (data & mask){ // if bitwise AND resolves to true
          send_bit_queue[send_bit_length] = true;
          Serial.print("1");
        }
        else{ //if bitwise and resolves to false
          send_bit_queue[send_bit_length] = false;
          Serial.print("0");
        }
        send_bit_length++;
      }
      i++;
    }
    
    msg_queue_pos_current = (msg_queue_pos_current+1)%8;
    T_status = 1;
    Serial.println("  : bit queue ready");
    if(is_master)
    {
      V_set(3);
    }else
    {
      V_set(2);
    }
  }
}

void T_send(){
  //envoit 1 bit
  while(T_status == 3)
  {
    //int phase_start = 0;  // start time of the current transfer phase
    //int current_time = 0;
    //int last_switch = 0;
    if(get_Vm())  //Vm mesurée et si Vm stable...
    {
      if(false)
      {
        //timeout reset
      }else if(Vm_level == 4)
      {
        if(self_level == 4)
        {
          T_status = 0;
          setDefaultTension();
          testing_Has_sent();
          Serial.println("  envoi termine");
        }else
        {
          Serial.print(">");
        }
      }else if(next_bit_clock)
      {
        //Serial.print("-");
        V_set(3);
        next_bit_clock = false;
      }else
      {
        if(send_bit_pos < send_bit_length)
        {
          if(send_bit_queue[send_bit_pos])
          {
            V_set(2);
            //Serial.print("1");
          }
          else
          {
            V_set(1);
            //Serial.print("0");
          }
          send_bit_pos++;
          next_bit_clock = true;
        }else
        {
          Serial.print("  closure  ");
          V_set(4);
          //delay(10000);
        }
      }
    }
  }
}

void T_listen(){
  
  Vm_level = tensionToLevel(pin_V_m); //comme je triche a la main (manque de diode)
  if(T_status == 2 && self_level != 4)
  {
    //Serial.println("listening");
    //log_number("Vm_level =  ", Vm_level);
    if(Vm_level == 3)
    {
      next_bit_clock = false;
      Serial.print("-");
    }else if(Vm_level == 4 && self_level != 4 && recieved_bit_length != 0)
    {
      T_status = 0;
      setDefaultTension();
      Serial.println("  listening over");
      log_number("recieved bits ", recieved_bit_length);
      log_number("byte 1 ", recieved_bytes[0]);
      log_number("byte 2 ", recieved_bytes[1]);
    }else if(Vm_level == 2 || Vm_level == 1)
    {
      int more = round(pow(2, 7 - (recieved_bit_length) % 8));
      int pos = round(recieved_bit_length/8);
      if(more == 128)
      {
        recieved_bytes[pos] = 0;
      }
      if(Vm_level == 2 && !next_bit_clock)
      {
        recieved_bytes[pos] = recieved_bytes[pos] + more;
        Serial.print(" 1 ");
        Serial.print(more);
      }else if(!next_bit_clock)
      {
        Serial.print(" 0 ");
        Serial.print(more);
      }
      recieved_bit_length++;
      next_bit_clock = true;
    }else
    {
      /*Serial.println("else");
      float ar = analogRead(pin_V_m);
      int V = float(ar);
      log_number("Vm special = ", V);*/
    }
  }
}



/*
  Tension functions
*/

void V_set(int level)
{
  //log_number("set level = ", level);
  current_time = micros();
  while(current_time - last_switch < switch_duration)
  {
    current_time = micros();
  }
  last_switch = current_time;
  if(level < V_nb && level != self_level)
  {
    //log_number("level", level);
    //log_number("self_level", self_level);
    if(level > 0)
    {
      //log_number("Pin high", pins_tension[level-1]);
      digitalWrite(pins_tension[level-1], HIGH);
    }
    if(self_level > 0)
    {
      //log_number("Pin low", pins_tension[self_level-1]);
      digitalWrite(pins_tension[self_level-1], LOW);
    }
    self_level = level;
  }
  Vm_level = tensionToLevel(pin_V_m);
}

int tensionToLevel(int pin)
{
  float ar = analogRead(pin_V_m);
  int V = float(ar);
  int i = 0;
  while(i < 5)
  {
    if(V_i[i] - dV < V && V < V_i[i] + dV)
      return i;
    i++;
  }
  if(V > V_i[4])
    return 4;
  
  return 0;
}

void setDefaultTension()
{
  if(is_master)
  {
    V_set(0);
  }
  else
  {
    V_set(1);
  }
}

void log_number(String msg, int value){
    Serial.print(msg);
    Serial.println(value,10);
}

boolean get_Vm()
{
  //mesure vm et renvoit true si la mesure est exploitable
  float ar = analogRead(pin_V_m);
  Vm = float(ar);
  Vm_level = tensionToLevel(pin_V_m);
  
  if(abs(Vm-Vm_last) <= 20 && (Vm_level > 0 || Vm < V_i[1]))
  {
    return true;
  }else
  {
    Vm_last = Vm;
    return false;
  }
}




/*
  Testing
*/

//boolean testing_sending_ON = false;

void testing_Has_to_send(){
  digitalWrite(led, HIGH);
  testing_sending_ON = true;
}

void testing_Has_sent(){
  digitalWrite(led, LOW);
  testing_sending_ON = false;
}
