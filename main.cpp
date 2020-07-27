#include "mbed.h"

#include "FXOS8700CQ.h"

Serial pc(USBTX, USBRX);

// Pin names for FRDM-K66F
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

uint32_t us_new_interval = 0; // to track the 2 sec interval

#define MAX_SIZE 1024
int x_arr[MAX_SIZE]={0};
int y_arr[MAX_SIZE]={0};
int z_arr[MAX_SIZE]={0};

int  size=0;  // actual number of elements in arrays x_arr, y_arr, z_arr

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

float linear_model() {

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

  if ( size > 0 && (us_elapsed - us_new_interval) > 2000000  ) { // 2 seconds
     us_new_interval =  us_elapsed;
     float linear = linear_model();
     //Logistic regression
     float  linear_regression = 1.0 / (1.0 + exp(-linear));
     printf ("\n array size=%d logistic_regression=%.2f linear=%.2f ", size, linear_regression, linear);
     size=0; //start populating sensors arrays from beginning

     if (linear_regression > 0.5){
             printf ("\n -----  linear_regression > 0.5 ------");
             green.write(1);
             red.write(0);
             blue.write(1);
     } else {
             green.write(0);
             red.write(1);
             blue.write(1);
     }
  }

  x_arr[size]=fxos.getAccelX();
  y_arr[size]=fxos.getAccelY();
  z_arr[size]=fxos.getAccelZ();

  //pc.printf("%lu x=%d y=%d z=%d size=%d \r\n", us_elapsed, x_arr[size], y_arr[size], z_arr[size], size);
   size++;
  previous_us_elapsed = us_elapsed;
}

void print_reading() //   magnetometer and accelerometer
{
    pc.printf("%lu A X:%5d,Y:%5d,Z:%5d   M X:%5d,Y:%5d,Z:%5d\r\n",
              us_elapsed,
              fxos.getAccelX(), fxos.getAccelY(), fxos.getAccelZ(),
              fxos.getMagnetX(), fxos.getMagnetY(), fxos.getMagnetZ());
}



int main(void)
{
    t.reset();

#define BAUDRATE 115200
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
    t.start(); // start timer and enter collection loop

    while (1) {
        fxos.get_data();
        print_accel();
    }
}
