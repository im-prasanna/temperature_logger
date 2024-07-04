#include "SPI.h"
#include "RTC_I2C.h"
#include "ADC.h"
#include "SD_FAT.h"

volatile uint16_t *TIMER3_OCRA = 0x98; //For timer
volatile uint16_t *TIMER3_TCNT = 0x94;
volatile uint8_t *TIMER3_TCCRA = 0x90;
volatile uint8_t *TIMER3_TCCRB = 0x91;
volatile uint8_t *TIMER3_TIMSK = 0x71;
volatile uint8_t m;

void setup() {
  Serial.begin(9600);

  if(! SD_init() ) {
    SD_CMD8();
    SD_sendCommand_R1(55,0,0);  //commnad 55 tells next command is A command
    SD_sendCommand_R1(41,0x40000000,0);  //ACMD41 with hcs=1(host support high capacity)
    startPartition = startOfFirstPartition();
    startRoot = startOfRoot(startPartition);
    createFile("TEMPLOGG.TXT");
    timerInitiate();
    Serial.println("[+] SETUP DONE");
    Serial.println("ENTER 513 to list files in root directory");
    Serial.println("ENTER FILENUMBER to read the file");

  }
  ListRootDir(startRoot);
}

void loop() {
  uint16_t n;
  if(Serial.available()){
    n=Serial.parseInt();
    if(n==513) ListRootDir(startRoot);
    else if(n!=0 && n!=513){readFileusingEntryNumber(n);};
  }

}
//COde for timer 
void WriteUpdatedData(){
  char updateData[31] = {0};
  rtc_update(0x68, updateData);
  float temp = getTemp();
  ftoa(temp, updateData+22);
  writeFile("TEMPLOGG.TXT", updateData);
  // Serial.println("DONE UPDATION");
}

void timerInitiate(){
  *TIMER3_TIMSK = 2;// Interrupt enable for OCRA
  *TIMER3_TCCRA = 0;
  *TIMER3_TCCRB = 0;
  *TIMER3_OCRA =15625;

  *TIMER3_TCCRB |= 1 << 3;
  *TIMER3_TCNT = 0;
  *TIMER3_TCCRB &= ~(0b010);*TIMER3_TCCRB |= 0b101;  //set 1024 prescaling
  // *TIMER3_TCCRB &= ~(0b111); // stop clock
}

ISR(TIMER3_COMPA_vect){
  m++;
  if(m==10){WriteUpdatedData();m=0;}
    }

