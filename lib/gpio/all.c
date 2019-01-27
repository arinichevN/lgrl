
void pinWrite(int pin, int value) {
    ;
}

 int pinRead(int pin) {
    int regval;
    srand(pin);
    regval = rand();
    regval &= 1;
    return regval;
}

 void pinLow(int pin) {
   ;
}

 void pinHigh(int pin) {
   ;
}

 void pinModeIn(int pin) {
   ;
}

 void pinModeOut(int pin) {
   ;
}

 void pinPUD(int pin, int pud) {
 ;
}
 
int checkPin(int pin) {
    return 1;
}

int gpioSetup() {
    return 1;
}

void gpioFree() {
    ;
}


