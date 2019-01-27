

int max31855_init(int sclk, int cs, int miso){
  pinModeOut(cs);
  pinHigh(cs);
  
    pinModeOut(sclk); 
    pinModeIn(miso);
}


int max31855_read(double *result, int sclk, int cs, int miso) {

  int32_t v;

  v = spiread32(sclk, cs, miso);

  if (v & 0x7) {
    return 0; 
  }

  if (v & 0x80000000) {
    // Negative value, drop the lower 18 bits and explicitly extend sign bits.
    v = 0xFFFFC000 | ((v >> 18) & 0x00003FFFF);
  }
  else {
    // Positive value, just drop the lower 18 bits.
    v >>= 18;
  }
 
  double centigrade = v;
  centigrade *= 0.25;
  *result= centigrade;
  return 1;
}

static uint32_t spiread32(int sclk, int cs, int miso) { 
  int i;
  uint32_t d = 0;

  pinLow(cs);
  delay(1);

  
    pinLow(sclk);
    delay(1);

    for (i=31; i>=0; i--) {
      pinLow(sclk);
      delay(1);
      d <<= 1;
      if (pinRead(miso)) {
	d |= 1;
      }
      
      pinHigh(sclk);
      delay(1);
    }

  pinHigh(cs);
  return d;
}
