// -*- C -*-

/* Simple DRAM tester
 * Andre Miller http://www.andremiller.net/
 * Based on:  http://insentricity.com/a.cl/252  Chris Osborn <fozztexx@fozztexx.com>
 *            https://github.com/FozzTexx/DRAM-Tester
 */

// Tested on Arduino MEGA 2560 China Clone with only 12MHz

#define DIN_P             53
#define DIN             (1 << 0)    // Bit 0 on Port B
#define DOUT_P            52
#define DOUT            (1 << 1)    // Bit 1 on Port B
#define CAS_P             51
#define CAS             (1 << 2)    // Bit 2 on Port B
#define RAS_P             50
#define RAS             (1 << 3)    // Bit 3 on Port B

// Write enable is on Pin 49 which is Bit 0 of Port L
#define WE_P              49
#define WE              (1 << 0)

#define ADDR_BITS       9  // 9 for 256, 10 is max address bits

void fillSame(int val);
void fillAlternating(int start);

void setDIN(int val)
{
  // Takes 150ns to toggle the state
  if (val) PORTB |= DIN;
  else PORTB &= ~DIN;
}

void setCAS(int val)
{
  // Takes 150ns to toggle the state
  if (val) PORTB |= CAS;
  else PORTB &= ~CAS;
}

void setRAS(int val)
{
  // Takes 150ns to toggle the state
  if (val) PORTB |= RAS;
  else PORTB &= ~RAS;
}

void setWE(int val)
{
  // Takes 150ns to toggle the state
  if (val) PORTL |= WE;
  else PORTL &= ~WE;
}

void setup()
{
  int mask;
  Serial.begin(115200);
  Serial.println("SETUP");
  Serial.print("Number of address bits: ");
  Serial.println(ADDR_BITS);

  
  pinMode(DIN_P, OUTPUT);
  pinMode(DOUT_P, INPUT);

  pinMode(CAS_P, OUTPUT);
  pinMode(RAS_P, OUTPUT);
  pinMode(WE_P, OUTPUT);

  /* 10 is max address bits, even if chip is smaller */
  mask = (1 << 10) - 1; 
  DDRA = mask & 0x3f;
  mask >>= 6;
  DDRC = mask & 0x0f;
  
  setCAS(HIGH);
  setRAS(HIGH);
  
  setWE(HIGH);
}

void loop()
{
  static int i=1;
  Serial.print("START ITERATION: ");
  Serial.println(i);

  fillAlternating(1);
  fillAlternating(0);
  fillSame(0);
  fillSame(1);
  fillRandom(10);
  fillRandom(200);

  Serial.print("END ITERATION: ");
  Serial.println(i);
  i+=1;
  
}

static inline int setAddress(int row, int col, int wrt)
{
  int val = 0;

  PORTA = row & 0x3f;
  PORTC = (PORTC & 0xf0) | (row >> 6) & 0x0f;
  setRAS(LOW);

  if (wrt)
  {
    setWE(LOW);
  }

  PORTA = col & 0x3f;
  PORTC = (PORTC & 0xf0) | (col >> 6) & 0x0f;
  setCAS(LOW);

  if (wrt)
  {
    setWE(HIGH);
  }
  else
  {
    val = digitalRead(DOUT_P);
  }
 
  setCAS(HIGH);
  setRAS(HIGH);

  return val;
}

void fail(int row, int col, int val)
{

  Serial.print("*** FAIL row ");
  Serial.print(row);
  Serial.print(" col ");
  Serial.print(col);
  Serial.print(" was expecting ");
  Serial.print(val);
  Serial.print(" got ");
  Serial.println(!val);

  while (1)
    ;
}

void fillSame(int val)
{
  int row, col;

  unsigned long writeStartMillis;
  unsigned long writeEndMillis;
  unsigned long readStartMillis;
  unsigned long readEndMillis;

  Serial.print("  Setting all bits set to: ");
  Serial.println(val);
  setDIN(val);

  Serial.println("    Write");
  writeStartMillis = millis();
  for (col = 0; col < (1 << ADDR_BITS); col++)
    for (row = 0; row < (1 << ADDR_BITS); row++)
      setAddress(row, col, 1);
  writeEndMillis = millis();

  /* Reverse DIN in case DOUT is floating */
  setDIN(!val);

  
  Serial.println("    Read");
  readStartMillis = millis();
  for (col = 0; col < (1 << ADDR_BITS); col++)
    for (row = 0; row < (1 << ADDR_BITS); row++)
      if (setAddress(row, col, 0) != val)
        fail(row, col, val);
  readEndMillis = millis();

  Serial.print("    Pass ");
  Serial.print("Write: ");
  Serial.print(writeEndMillis - writeStartMillis);
  Serial.print("ms Read: ");
  Serial.print(readEndMillis - readStartMillis);
  Serial.println("ms");
  return;
}

void fillAlternating(int start)
{
  int row, col, i;

  unsigned long writeStartMillis;
  unsigned long writeEndMillis;
  unsigned long readStartMillis;
  unsigned long readEndMillis;

  Serial.print("  Alternating bits starting with: ");
  Serial.println(start);

  Serial.println("    Write");
  i = start;
  writeStartMillis = millis();
  for (col = 0; col < (1 << ADDR_BITS); col++) {
    for (row = 0; row < (1 << ADDR_BITS); row++) {
      setDIN(i);
      i = !i;
      setAddress(row, col, 1);
    }
  }
  writeEndMillis = millis();

  Serial.println("    Read");
  readStartMillis = millis();
  for (col = 0; col < (1 << ADDR_BITS); col++) {
    for (row = 0; row < (1 << ADDR_BITS); row++) { 
      if (setAddress(row, col, 0) != i)
        fail(row, col, i);

      i = !i;
    }
  }
  readEndMillis = millis();
  
  Serial.print("    Pass ");
  Serial.print("Write: ");
  Serial.print(writeEndMillis - writeStartMillis);
  Serial.print("ms Read: ");
  Serial.print(readEndMillis - readStartMillis);
  Serial.println("ms");
  return;
}

void fillRandom(int seed)
{
  int row, col, i;

  unsigned long writeStartMillis;
  unsigned long writeEndMillis;
  unsigned long readStartMillis;
  unsigned long readEndMillis;

  randomSeed(seed);

  Serial.print("  Random bit values with seed: ");
  Serial.println(seed);

  Serial.println("    Write");
  writeStartMillis = millis();
  for (col = 0; col < (1 << ADDR_BITS); col++) {
    for (row = 0; row < (1 << ADDR_BITS); row++) {
      i = random(0,2);
      //i = 1;
      //Serial.println(i);
      setDIN(i);
      setAddress(row, col, 1);
    }
  }
  writeEndMillis = millis();

  randomSeed(seed);

  Serial.println("    Read");
  readStartMillis = millis();
  for (col = 0; col < (1 << ADDR_BITS); col++) {
    for (row = 0; row < (1 << ADDR_BITS); row++) { 
      i = random(0,2);
      //i=1;
      //Serial.println(i);
      if (setAddress(row, col, 0) != i)
        fail(row, col, i);
    }
  }
  readEndMillis = millis();
  
  Serial.print("    Pass ");
  Serial.print("Write: ");
  Serial.print(writeEndMillis - writeStartMillis);
  Serial.print("ms Read: ");
  Serial.print(readEndMillis - readStartMillis);
  Serial.println("ms");
  return;
}
