// for I2c with RTC
volatile uint8_t *twi_TWBR = (uint8_t*)0xB8;
volatile uint8_t *twi_TWCR = (uint8_t*)0xBC;
volatile uint8_t *twi_TWSR = (uint8_t*)0xB9;
volatile uint8_t *twi_TWDR = (uint8_t*)0xBB;

// CODE FOR RTC using I2C
void start() {
  *twi_TWCR = 0b10100100;
  while (!(*twi_TWCR & (~0b1111111)));
  // if((*twi_TWSR &(~0b11)) == 0x08) Serial.println("Started");
}
void stop() {
  *twi_TWCR = 0b10010100;
  while (!(*twi_TWCR & (~0b1111111)));
}
void notACK() {
  *twi_TWCR = 0b10000100;
  while (!(*twi_TWCR & (~0b1111111)));
}

void connect(uint8_t address, int rw) {
  *twi_TWDR = ((address << 1) + rw);
  *twi_TWCR = 0b10000100;
  while (!(*twi_TWCR & (~0b1111111)));
  // if(((*twi_TWSR &(~0b11)) == 0x18)&&!rw) Serial.println("connect ACk");
  // if(((*twi_TWSR &(~0b11)) == 0x40)&&rw) Serial.println("connect ACk");
}

void writei2c(uint8_t data) {
  *twi_TWDR = data;
  *twi_TWCR = 0b10000100;
  while (!(*twi_TWCR & (~0b1111111)));
  // if((*twi_TWSR &(~0b11)) == 0x28) Serial.println("Data sent ACk");
}
uint8_t readi2c() {
  *twi_TWCR = 0b11000100;
  while (!(*twi_TWCR & (~0b1111111)));
  return *twi_TWDR;
}
void rtc_update(uint8_t address, char *s) {
  uint8_t temp;
  *twi_TWBR = 0b1111111;
  *twi_TWSR |= 0b11;
  start();
  connect(address, 0);
  writei2c(0x00);
  start();
  connect(address, 1);
  temp = readi2c();
  s[17] = ((temp << 1) >> 5) + '0';
  s[18] = (temp & 0b00001111) + '0'; s[19] = ' '; s[20] = '='; s[21] = ' ';
  temp = readi2c();
  s[14] = ((temp << 1) >> 5) + '0';
  s[15] = (temp & 0b00001111) + '0'; s[16] = ':';
  temp = readi2c();
  s[11] = (((temp >> 4) & 0b11) + '0');
  s[12] = (temp & 0b00001111) + '0'; s[13] = ':';
  temp = readi2c();
  // now.day = temp;
  temp = readi2c();
  s[8] = ((temp) >> 4) + '0';
  s[9] = (temp & 0b00001111) + '0'; s[10] = ' ';
  temp = readi2c();
  s[5] = ((temp) >> 4) + '0';
  s[6] = (temp & 0b00001111) + '0'; s[7] = '.';
  temp = readi2c();
  s[0] = '2'; s[1] = '0'; s[2] = (((temp) >> 4)) + '0'; s[3] = ((temp & 0b00001111)) + '0'; s[4] = '.';
  notACK();
  stop();
}
