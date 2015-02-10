int led = 13;
/*
  Status
*/
boolean is_master = true;  // comportement master ou slave
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
//byte recieved_bytes[3];
int recieved_bytes[3];
boolean next_bit_clock = true;
byte msg_queue_pos_current = 0;
byte msg_queue_pos_available = 0;
byte msg_queue_array_id[8];
byte msg_queue_array_value[8];


/*
  Time
*/
long timeout = 100000000; // max duration for a transfer phase
long phase_start = 0;  // start time of the current transfer phase
long current_time = 0;
long last_switch = 0; // l'instant du dernier changement de tension (pour l'envoi)
long switch_duration = 1000;
long last_mesure = 0;

/*
  Multiplexion
*/
int pins_multi[3] = {5, 6, 7};

/*
  Tensions
*/
int pin_switch = 7;
int pin_V_m = 7; // Cross mesure: ligne de transmission commune
int V_nb = 5;
//int V_i[5] = {0, 172, 227, 332, 600}; //paliers V0, V1, ...
//int V_i[5] = {0, 260, 471, 661, 812}; //paliers V0, V1, ...
int V_i[5] = {0, 290, 540, 770, 940}; //paliers V0, V1, ... Via mesurer.arduino
int dV = 50;
int pins_tension[4] = {11, 10, 9, 8}; // V1, V2, V3, V4
int V_last = 0; //niveau tension la derniere fois qu'il y a eu une mesure (pour la réception)
float tension;
int amp;


/*
  Levels
*/
int loop_level = 0;  //le niveau que l'on incrémente dans la boucle
int self_level = 0;  //le niveau que le module propose à la zone commune
int Vm_level = 0;    //le niveau mesuré dans la zone de transmission commune
int Vm = 0; // la tension au loop courant
int Vm_last = 0; //la tension de la loop precedente

/*
  Mesures
*/
int all_sensors[128];


void setup() {
  //Serial.begin(9600);
  Serial.begin(31250);
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
  T_status = 0;
  
  i = 0;
  while(i < 3)
  {
    pinMode(pins_multi[i], OUTPUT);
    digitalWrite(pins_multi[i], LOW);
    i++;
  }
  
  i = 0;
  while(i < 3)
  {
    delay(1000);
    //Serial.print(">");
    i++;
  }
  //Serial.println(" Ready Master");
}

void loop() {
  /*
    Loop Init
  */
  //delay(100);
    
  float ar = analogRead(pin_V_m);
  Vm = float(ar);
  Vm_level = tensionToLevel(pin_V_m);
  if(false && micros() - last_mesure > 1000000)
  {
    log_number("Vm = ", Vm);
    log_number("Vm_level = ", Vm_level);
    last_mesure = micros();
  }
  
  if(abs(Vm-Vm_last) <= 40 && (Vm_level > 0 || Vm < V_i[1]))
  {
    
    /*
      Sensors
    */
    getSensors();
    /*if(digitalRead(pin_switch) && !is_master)
    {
      addToSend(12, 230);
    }*/
    
    /*
      Bond
    */
    //Vm_level = tensionToLevel(pin_V_m);
    //log_number("Vm_level ", Vm_level);
    T_connection_status();  //change le statut en fonction de ce qui est mesuré/voulu
    
    
    /*
      Processing
    */
      T_send();        // si en mode envoi et qch à envoyer, envoie 1 bit
      T_listen();      // si en mode écoute, ...
      T_send_init();  // si possible et si nécessaire, prépare la liste des bools à envoyer
  }else
    Vm_last = Vm;
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
  //Serial.println("add to send");
  //on ajoute que si il reste de la place parmi les 8 valeurs en queue
  if((msg_queue_pos_current - 1) %8 != msg_queue_pos_available)
  {
    if(testing_sending_ON)
      return;
      
    //Serial.println("can add something");
    msg_queue_array_id[msg_queue_pos_available] = id;
    msg_queue_array_value[msg_queue_pos_available] = value;
    msg_queue_pos_available = (msg_queue_pos_available+1)%8;
    
    testing_Has_to_send();
  }
}

void T_connection_status(){
  //si je dois écouter l'autre, alors ...
  //si j'ai qch à envoyer, ...
  //si j'ai fini
  //log_number("T_status ", T_status);
  if(T_status < 2 && ((is_master && Vm_level == 2) || (!is_master && Vm_level == 3)))
  {
    //Serial.println("!R"); //passage en mode reception
    T_status = 2;
    recieved_bit_length = 0;
    next_bit_clock = true;
    V_set(4);
  }else if(T_status == 1 && Vm_level == 4 && self_level != 4)  //Vm = 4, mais tant que pas interlocuteur, reste 1
  {
    //on peut passer en mode envoi
    //Serial.println("!E"); //passage en mode envoi
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
    //Serial.println("init!");
    send_bit_pos = 0;
    send_bit_length = 0;
    byte data = msg_queue_array_value[msg_queue_pos_current];
    byte mask = 1;
    
    for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
      if (data & mask){ // if bitwise AND resolves to true
        send_bit_queue[send_bit_length] = true;
        //Serial.print("1");
      }
      else{ //if bitwise and resolves to false
        send_bit_queue[send_bit_length] = false;
        //Serial.print("0");
      }
      send_bit_length++;
    }
    
    msg_queue_pos_current = (msg_queue_pos_current+1)%8;
    T_status = 1;
    if(is_master)
    {
      V_set(3);
    }else
    {
      V_set(2);
    }
    //Serial.println("  : bit queue ready");
  }
}

void T_send(){
  //envoit 1 bit
  if(T_status == 3)
  {
    //int phase_start = 0;  // start time of the current transfer phase
    //int current_time = 0;
    //int last_switch = 0; 
    if(get_Vm())
    {
      if(Vm_level == 4)
      {
        if(self_level == 4)
        {
          T_status = 0;
          setDefaultTension();
          testing_Has_sent();
          //Serial.println("  envoi termine");
        }else
        {
          //Serial.print(">");
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
          //Serial.print("  closure  ");
          V_set(4);
        }
      }
    }
  }
}

void T_listen(){
  
  //Vm_level = tensionToLevel(pin_V_m); //comme je triche a la main (manque de diode)
  int more = 0;
  int pos = 0;
  int bit_level = 0;
  recieved_bytes[0] = 0;
  recieved_bytes[1] = 0;
  recieved_bytes[2] = 0;
  while(T_status == 2)
  {
    //Serial.println("listening");
    //log_number("Vm_level =  ", Vm_level);
    if(get_Vm())
    {
      //log_number("Vm =  ", Vm);
      if(self_level == 4)
      {
        //Serial.println("ok"); //mode reception operationnel
        V_set(0);
      }else if(Vm_level == 3)
      {
        next_bit_clock = false;
        bit_level = 0;
        //Serial.print("-");
      }else if(Vm_level == 4 && self_level != 4 && recieved_bit_length != 0)
      {
        T_status = 0;
        setDefaultTension();
        midi(0x90, 2, 0x45);
        //Serial.println("  listening over");
        log_number("recieved bits ", recieved_bit_length);
        log_number("byte 1 ", recieved_bytes[0]);
        log_number("byte 2 ", recieved_bytes[1]);
        log_number("byte 3 ", recieved_bytes[2]);
      }else if(Vm_level == 2 || Vm_level == 1)
      {
        if(Vm_level == 2 && !next_bit_clock)
        {
          more = round(pow(2, 7 - (recieved_bit_length) % 8));
          pos = round(recieved_bit_length/8);
          if(more == 128)
          {
            recieved_bytes[pos] = 0;
          }
          //Serial.println("level 2");
          recieved_bytes[pos] = recieved_bytes[pos] + more;
          bit_level = 2;
          recieved_bit_length++;
        }else if(!next_bit_clock) //next bit est important car initialisé à l'extérieur pour définir que la séquence commence au prochain niveau 3
        {
          //si c'est la premiere fois qu'on mesure le bit
          bit_level = 1;
          recieved_bit_length++;
          //Serial.println("level 1");
        }else if(Vm_level == 1 && bit_level == 2)
        {
          bit_level = 1;
          recieved_bytes[pos] = recieved_bytes[pos] - more;
          //Serial.println("level from 2 to 1");
          //log_number("level from 2 to 1 ", Vm);
        }
        next_bit_clock = true;
      }else
      {
        //Serial.println("?");
      }
    }
  }
}

int note = 0x00;
int channel = 0xB0;
// a priori les channels 0xBn pour les controls
// les channels 0x[1-9]n pour les instrus
// pour les drums
// 1 module = 1 channel
// pitch l'id de la note mesurée par le module
// velocity la valeur mesurée

void midi(int cmd, int pitch, int velocity)
{
  Serial.write(channel);
  Serial.write(0x45);
  Serial.write(note);
  note ++;
  delay(20);
  if(note >= 0x80)
  {
    note = 0x00;
    if(channel == 0xB0)
      channel = 0xB1;
    else
      channel = 0xB0;
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
  //log_number("V_set level", level);
  if(level < V_nb && level != self_level)
  {
    if(level > 0)
    {
      //log_number("pins_tension[level-1]", pins_tension[level-1]);
      digitalWrite(pins_tension[level-1], HIGH);
    }
    if(self_level > 0)
    {
      //log_number("pins_tension[self_level-1]", pins_tension[self_level-1]);
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
  return;
  Serial.print(msg);
  Serial.println(value,10);
}

boolean get_Vm()
{
  //mesure vm et renvoit true si la mesure est exploitable
  float ar = analogRead(pin_V_m);
  Vm = float(ar);
  Vm_level = tensionToLevel(pin_V_m);
  /*if(T_status == 2)
    log_number("", Vm);*/
  if(abs(Vm-Vm_last) <= 5 && (Vm_level > 0 || Vm < V_i[1]))
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
