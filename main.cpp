#include "mbed.h"

//#include "platform/mbed_thread.h"
//#define BLINKING_RATE_MS 

#include "FXOS8700CQ.h"

#define DATA_RECORD_TIME_MS 1000

Serial pc(USBTX, USBRX); // Primary output to demonstrate library

// Pin names for FRDM-K64F
// FXOS8700CQ fxos(PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // SDA, SCL, (addr << 1)
 FXOS8700CQ fxos(PTD9, PTD8, FXOS8700CQ_SLAVE_ADDR1);

DigitalOut green(LED_GREEN); // waiting light
DigitalOut blue(LED_BLUE); // collection-in-progress light
DigitalOut red(LED_RED); // completed/error ligt

Timer t; // Microsecond timer, 32 bit int, maximum count of ~30 minutes
// InterruptIn fxos_int1(PTC6); // unused, common with SW2 on FRDM-K64F
InterruptIn fxos_int1(PTC17);

InterruptIn fxos_int2(PTC13); // should just be the Data-Ready interrupt

// InterruptIn start_sw(PTA4); // switch SW3
InterruptIn start_sw(PTA10); // switch SW3

// Interrupt status flags and data
bool fxos_int1_triggered = false;
bool fxos_int2_triggered = false;
uint32_t us_ellapsed = 0;
uint32_t previous_us_ellapsed = 0;

bool start_sw_triggered = false;

void trigger_fxos_int1(void)
{
    fxos_int1_triggered = true;
}

void trigger_fxos_int2(void)
{
    fxos_int2_triggered = true;
    us_ellapsed = t.read_us();
}

void trigger_start_sw(void)
{
    start_sw_triggered = true;
}

void print_accel(){
 if ( us_ellapsed == previous_us_ellapsed) return;
  pc.printf("%d %d %d %d\r\n", us_ellapsed, fxos.getAccelX(), fxos.getAccelY(), fxos.getAccelZ());
  previous_us_ellapsed = us_ellapsed;
}
void print_reading()
{
    pc.printf("%d A X:%5d,Y:%5d,Z:%5d   M X:%5d,Y:%5d,Z:%5d\r\n",
              us_ellapsed,
              fxos.getAccelX(), fxos.getAccelY(), fxos.getAccelZ(),
              fxos.getMagnetX(), fxos.getMagnetY(), fxos.getMagnetZ());
}

#define BAUDRATE 115200
//#define BAUDRATE 9600

int main(void)
{
    // Setup
    t.reset();
    
    //pc.baud(115200); // Print quickly! 200Hz x line of output data!
    //pc.baud(9600); // Print quickly! 200Hz x line of output data!
    pc.baud(BAUDRATE);
    // Lights off (FRDM-K64F has active-low LEDs)
    green.write(1);
    red.write(1);
    blue.write(1);
/*
    int i=0;
    int j=1;
    int k=0;

    while (true) {
     
     printf ("\n %d %d %d" , i,j,k);
     thread_sleep_for(BLINKING_RATE_MS);
     i++; j++; k++;
     if (i==0) i=1;
     if (j==0) j=1;
     if (k==0) k=1;
     j++;
     k++;
    }
*/
    // Diagnostic printing of the FXOS WHOAMI register value

    while (1 >2) {
         printf("\r\n\nFXOS8700Q Who Am I= %X\r\n", fxos.get_whoami());
         uint8_t scale = fxos.get_accel_scale();
         // FXOS8700CQ_XYZ_DATA_CFG_FS2(1)    ... return 4
         printf ("\n scale=%hhu", scale);
    }
    // Iterrupt for active-low interrupt line from FXOS
    // Configured with only one interrupt on INT2 signaling Data-Ready
    fxos_int2.fall(&trigger_fxos_int2);
    fxos.enable();

    // Interrupt for SW3 button-down state
    start_sw.mode(PullUp); // Since the FRDM-K64F doesn't have its SW2/SW3 pull-ups populated
    start_sw.fall(&trigger_start_sw);

    green.write(0); // ready-green on

    // Example data printing
/* 
   fxos.get_data();
    //print_reading();
    print_accel();

    pc.printf("Waiting for data collection trigger on SW3\r\n");

    while(true) {
        if(start_sw_triggered) {
            break;
        }
        wait_ms(50); // just to slow the loop, fast enough for UX
    }

    green.write(1); // ready-green off
    blue.write(0); // working-blue on
   uint8_t get_accel_scale() returns  2, 4, or 8, depending on part configuration; 0 on error
   pc.printf("Started data collection. Accelerometer at max %dg.\r\n",
   fxos.get_accel_scale());
    //uint8_t get_data(); returns 0 on success
    fxos.get_data(); // clear interrupt from device
    fxos_int2_triggered = false; // un-trigger
*/
    t.start(); // start timer and enter collection loop
    //while (t.read_ms() <= DATA_RECORD_TIME_MS) {
    while (1) {
        fxos.get_data();
        print_accel();
/*
        if(fxos_int2_triggered) {
            fxos_int2_triggered = false; // un-trigger
            fxos.get_data();
            print_reading(); // outpouring of data !!
        }

        // Continuous polling of interrupt status is not efficient, but...
        wait_us(500); // 1/10th the period of the 200Hz sample rate
*/
    }

    blue.write(1); // working-blue off
    red.write(0); // complete-red on

    pc.printf("Done. Reset to repeat.\r\n");

    while(true) {
        pc.putc('.'); // idle dots
        wait(1.0);
    }
}
