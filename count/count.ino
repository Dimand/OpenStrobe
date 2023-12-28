//Count
// Author: Sam Legge
// Based on code from: Nick Gammon
// Date: 2023-12-28

// Input: Pin D5 -> counter 1

// these are checked for in the main program
volatile unsigned long timerCounts;
volatile boolean counterReady;

// internal to counting routine
unsigned long overflowCount = 0;
unsigned long overflowCount2 = 0;
unsigned int timerTicks;
unsigned int timerPeriod;

unsigned long previous_1 = 0;
unsigned long previous_2 = 0;
bool first_loop = 0;

ISR (TIMER1_OVF_vect)
  {
  ++overflowCount;               // count number of Counter1 overflows  
  }  // end of TIMER1_OVF_vect


//******************************************************************
//  Timer2 Interrupt Service is invoked by hardware Timer 2 every 1 ms = 1000 Hz
//  16Mhz / 128 / 125 = 1000 Hz

ISR (TIMER2_OVF_vect) 
  {
    ++overflowCount2;
  }  // end of TIMER2_COMPA_vect




  
//******************************************************************
void setup () 
  {
  Serial.begin(115200);       
  Serial.println("Frequency Counter");

  //start counter 1 and here then just watch it.
  //counter 1 counts the strobe wheel pulses
  //counter 2 counts time.

  //counter 2 counts at 16mhz with 16 bits
  //2^32/16e6 = 268.4 seconds
  //so add another uint16 to count overflows

  // reset Timer 1 and Timer 2
  TCCR1A = 0;             
  TCCR1B = 0;              
  TCCR2A = 0;
  TCCR2B = 0;

  // Timer 1 - counts events on pin D5
  TIMSK1 = bit (TOIE1);   // interrupt on Timer 1 overflow

  // Timer 2 - gives us our 1 ms counting interval
  // 16 MHz clock (62.5 ns per tick) - prescaled by 128
  //  counter increments every 8 µs. 
  // So we count 125 of them, giving exactly 1000 µs (1 ms)
  //TCCR2A = bit (WGM21) ;   // CTC mode
  //OCR2A  = 124;            // count up to 125  (zero relative!!!!)

  // Timer 2 - interrupt on match (ie. every 1 ms)
   TIMSK2 = bit (TOIE2);   // enable Timer2 Interrupt on overflow

  TCNT1 = 0;      // Both counters to zero
  TCNT2 = 0;     

  // Reset prescalers
  GTCCR = bit (PSRASY);        // reset prescaler now
  // start Timer 2
  TCCR2B =  bit (CS20) | bit (CS22) ;  // prescaler of 128
  // start Timer 1
  // External clock source on T1 pin (D5). Clock on rising edge.
  TCCR1B =  bit (CS10) | bit (CS11) | bit (CS12);
  } // end of setup



  
//******************************************************************
void loop () 
  {
    // grab counter value before it changes any more
    unsigned int timer1CounterValue;
    unsigned int timer2CounterValue;
    timer1CounterValue = TCNT1;  // see datasheet, page 117 (accessing 16-bit registers)
    timer2CounterValue = TCNT2;
 
    // calculate total count
    unsigned long Counts1 = (overflowCount << 16) + timer1CounterValue;  // each overflow is 2^16 | 65536 more  
    unsigned long Counts2 = (overflowCount2 << 8) + timer2CounterValue;  // each overflow is 2^8 more

    if (first_loop == 0) {
    previous_1 = Counts1;
    previous_2 = Counts2;
    first_loop = 1;
    delay(500);
    return;
    }

    float time_delay = (Counts2-previous_2)/125.0*1e-3; //in s
    float fequency =  (Counts1-previous_1)/time_delay;
    //that works. now add a simple circular buffer to store this over a resonable time.





    //now write a feedback loop to stabilise the frequency.
    

    Serial.print (fequency);
    Serial.println (" fequency");

    Serial.print ( time_delay);
    Serial.println (" time_delay");
    Serial.print ( Counts1);
    Serial.println (" Timer1");
    //Serial.print ( TCNT1);
    //Serial.println (" TCNT1");
    Serial.print ( Counts2);
    Serial.println (" Timer2");
    Serial.println ("");
    // let serial stuff finish
    delay(500);


    previous_1 = Counts1;
    previous_2 = Counts2;
  }   // end of loop
