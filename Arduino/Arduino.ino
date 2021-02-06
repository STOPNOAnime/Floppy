#include <TimerOne.h>

static const int FLOPPY_NUMBER = 2;
static const int MAX_POS = 158; //80 steps *2 - 2

struct floppy_struct{
  uint8_t pos;
  uint16_t period;
  uint16_t tick;
  bool dir_val;
  bool step_val;
};

floppy_struct floppy_array[FLOPPY_NUMBER];

void do_step(int what_floppy) {
  //Check if not hitting edge
  if(floppy_array[what_floppy].pos >= MAX_POS) {
    floppy_array[what_floppy].dir_val = HIGH;
    digitalWrite((what_floppy*2)+3,HIGH);
  }
  else if(floppy_array[what_floppy].pos == 0) {
    floppy_array[what_floppy].dir_val = LOW;
    digitalWrite((what_floppy*2)+3,LOW);
  }

  //Do step
  floppy_array[what_floppy].step_val = !floppy_array[what_floppy].step_val;
  digitalWrite((what_floppy*2)+2,floppy_array[what_floppy].step_val);

  //Change postion count
  if(floppy_array[what_floppy].dir_val == LOW) {
    floppy_array[what_floppy].pos++;
  }
  else {
    floppy_array[what_floppy].pos--;
  }
}

void reset_floppy() {
  for(int i=0;i<FLOPPY_NUMBER;i++){
    floppy_array[i].pos = 0;
    floppy_array[i].period = 0;
    floppy_array[i].tick = 0;
    floppy_array[i].dir_val = LOW;
    floppy_array[i].step_val = LOW;
  }

  for(int i=0;i<80;i++){
    for(int j=0;j<FLOPPY_NUMBER;j++){
      digitalWrite((j*2)+3,HIGH);
      digitalWrite((j*2)+2,HIGH);
      digitalWrite((j*2)+2,LOW);
    }
    delay(5);
  }

  for(int i=0;i<FLOPPY_NUMBER;i++){
    digitalWrite((i*2)+3,LOW);
  }
}

void on_tick() {
  for(int i=0;i<FLOPPY_NUMBER;i++){
    if(floppy_array[i].period > 0){
      floppy_array[i].tick++;
      if(floppy_array[i].tick >= floppy_array[i].period) {
        do_step(i);
        floppy_array[i].tick = 0;
      }
    }
  }
}

void connect(uint8_t byte) {
  if(byte == 0x0F){
    reset_floppy();
    Serial.write(FLOPPY_NUMBER);
  }
  else{ //Error
    Serial.write(0);
  }
}

void set_note(uint8_t byte0){
  while (!Serial.available()){}

  uint8_t byte1 = Serial.read();

  uint8_t val = byte0 & 0x0F;
  floppy_array[val].period = 0;
  floppy_array[val].tick = 0;

  if(byte1<72) { //Dont play higher frequencies bcs they dont work
    float temp;

    temp = byte1 - 69;
    temp = temp / 12;
    temp = pow(2,temp);
    temp = 440*temp;
    temp = 20000/temp;

    floppy_array[val].period = temp;
  }
}

void end_note(uint8_t byte0){
  uint8_t val = byte0 & 0x0F;
  floppy_array[val].period = 0;
  floppy_array[val].tick = 0;
}

void setup() {
  //step = even pin numbers
  //direction = uneven pin numbers
  for(int i=2;i<18;i++){
    pinMode(i, OUTPUT);
  }

  reset_floppy();
  
  Serial.begin(115200);

  Timer1.initialize(50);
  Timer1.attachInterrupt(on_tick);
}

void loop() {
  if (Serial.available()){
    uint8_t byte = Serial.read();
    
    switch(byte>>4) {
      case 0: connect(byte);    break;
      case 1: reset_floppy();   break;
      case 2: end_note(byte);   break;
      case 3: set_note(byte);   break;  
    }      
  }
}
