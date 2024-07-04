//For ADC (for temperature)
volatile uint8_t *adc_ADMUX = 0x7C, *adc_ADCSRB = 0x7B, *adc_ADCSRA = 0x7A, *adcL = 0x78, *adcH = 0x79;// FOr Temperture reading (ADC)
float getTemp();
uint16_t aread();
//CODE FOR TEMPERATURE using ADC
float getTemp() {
  uint16_t value = aread();
  float celsius = 1 / (log(1 / (1023. / value - 1)) / 3950 + 1.0 / 298.15) - 273.15;
  return celsius;
}
uint16_t aread() {
  uint16_t value;
  *adc_ADMUX = 0b01100000;
  *adc_ADCSRB = 0;
  *adc_ADCSRA = 0b11000000;
  while (!(*adc_ADCSRA & 0b00010000));
  value = *adcH;
  value = value << 2;
  value |= (*adcL >> 6);
  return value;
}
