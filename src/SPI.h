//FOR SPI with SDCARD
volatile uint8_t *spi_SPCR = 0x4C, *spi_SPSR = 0x4D, *spi_SPDR = 0x4E, *spi_DDRB = 0x24, *spi_PORTB = 0x25;

#define SPIF 7  // for SPSR register
#define WCOL 6
#define SPI2X 0

#define SPIE 7  // for SPCR register
#define SPE 6
#define DORD 5
#define MSTR 4
#define CPOL 3
#define CPHA 2
#define SPR1 1
#define SPR0 0

#define SS 0
#define COPI 2
#define CIPO 3
#define SCK 1

#define SShigh() *spi_PORTB |= (1<<SS);
#define SSlow() *spi_PORTB &= ~(1<<SS);
void masterInit(){
  *spi_DDRB |= ((1<<SS)|(1<<COPI)|(1<<SCK)); //set SS,COPI, SCK as output
  *spi_DDRB &= (~(1<<CIPO)); // CIPO as input
  SSlow();
  *spi_SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR1)|(1<<SPR0));
  *spi_SPSR &= ~(1<<SPI2X);
}

uint8_t spitransfer(uint8_t data){
  *spi_SPDR = data;
  while(!(*spi_SPSR & (1<<SPIF)));
  return *spi_SPDR;
}
