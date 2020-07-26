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
uint32_t us_elapsed = 0;
uint32_t previous_us_elapsed = 0;

 uint32_t us_new_interval = 0; // 2 sec

int x=0;
int y=0;
int z=0;

int x_arr[1024]={0};
int y_arr[1024]={0};
int z_arr[1024]={0};

int  size=0;


bool start_sw_triggered = false;

void trigger_fxos_int1(void)
{
    fxos_int1_triggered = true;
}

void trigger_fxos_int2(void)
{
    fxos_int2_triggered = true;
    us_elapsed = t.read_us();
}

void trigger_start_sw(void)
{
    start_sw_triggered = true;
}

float get_mean(int* arr, int size){
    float sum=0.0;
    for (int i=0; i<size; i++){
        sum += arr[i];
    }
    return sum / float(size);
}

float get_std(int* arr, int size, float mean){
   float varsum=0.0;
   for (int i=0; i<size; i++){
       varsum += pow((arr[i] - mean), 2);
   }
   return sqrt(varsum/size);
}

float linear_model(){
/*
Sample 2 seconds of raw data
Calculate, ideally by using CMSIS-DSP
x_s = standardDeviation(x)
y_s = standardDeviation(y)
z_s = standardDeviation(z)
xy_s = x_s/y_s
yz_s = y_s/z_s
xz_s = x_s/z_s
x_m = mean(x)
y_m = mean(y)
z_m = mean(z)
order features
features = ['x_s','y_s','z_s', 'xy_s', 'yz_s', 'xz_s', 'x_m','y_m','z_m']
Coefficients
[0.00084098]
[[ 3.20564208e-01 -2.11376098e-01 9.24422019e-01 6.76794213e-04
 -1.05621872e-02 -4.43306975e-04 5.12216590e-01 -2.13622099e-01
 -4.05966192e-02]]
*/
 
 float x_m = get_mean(x_arr, size);
 float y_m = get_mean(y_arr, size);
 float z_m = get_mean(z_arr, size);

 float x_s = get_std(x_arr, size, x_m);
 float y_s = get_std(y_arr, size, y_m);
 float z_s = get_std(z_arr, size, z_m);
 
 float xy_s = x_s/y_s;
 float yz_s = y_s/z_s;
 float xz_s = x_s/z_s;

 static float c[] = {
   3.20564208e-01,    // 0
  -2.11376098e-01,
   9.24422019e-01,    // 2
   6.76794213e-04,
  -1.05621872e-02,    // 4
  -4.43306975e-04,
   5.12216590e-01,    // 6
  -2.13622099e-01,
  -4.05966192e-02     // 8
 };

 return 0.00084098 +
        c[0] * x_s +
        c[1] * y_s +
        c[2] * z_s +
        c[3] * xy_s +
        c[4] * yz_s +
        c[5] * xz_s +
        c[6] * x_m +
        c[7] * y_m +
        c[8] * z_m
        ;
}

void print_accel(){
 if ( us_elapsed == previous_us_elapsed) return;
  if ( (us_elapsed - us_new_interval) > 2000000  ) { //2 SECONDS
     us_new_interval =  us_elapsed;
     linear_model();
     size=0;
  }

  x=fxos.getAccelX();
  y=fxos.getAccelY();
  z=fxos.getAccelZ();
  x_arr[size]=x;
  y_arr[size]=y;
  z_arr[size]=z;
  size++;

  pc.printf("%lu %d %d %d\r\n", us_elapsed, x, y, z);
  previous_us_elapsed = us_elapsed;
}
void print_reading()
{
    pc.printf("%lu A X:%5d,Y:%5d,Z:%5d   M X:%5d,Y:%5d,Z:%5d\r\n",
              us_elapsed,
              fxos.getAccelX(), fxos.getAccelY(), fxos.getAccelZ(),
              fxos.getMagnetX(), fxos.getMagnetY(), fxos.getMagnetZ());
}

#define BAUDRATE 115200

int main(void)
{
    // Setup
    t.reset();

    pc.baud(BAUDRATE);
    // Lights off
    green.write(1);
    red.write(1);
    blue.write(1);

 
    // Iterrupt for active-low interrupt line from FXOS
    // Configured with only one interrupt on INT2 signaling Data-Ready
    fxos_int2.fall(&trigger_fxos_int2);
    fxos.enable();

    // Interrupt for SW3 button-down state
    start_sw.mode(PullUp); // Since the FRDM-K64F doesn't have its SW2/SW3 pull-ups populated
    start_sw.fall(&trigger_start_sw);

    green.write(0); // ready-green on

    // Example data printing

    t.start(); // start timer and enter collection loop

    while (1) {
        fxos.get_data();
        print_accel();
    }
}
