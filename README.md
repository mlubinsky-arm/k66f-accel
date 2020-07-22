# FRDM-K66F accelerometer

Output format: every record has 4 integer values, separated by space :

 1) Time from the start in (microseconds)
 2) X accelaration
 3) Y accelaration
 4) Z acceleration

Compiled with baudrate: 115200 

### Sensitivity
```
Sensitivity of an accelerometer defines at what rate the sensor converts mechanical energy into an electric signal (the output); 
and this will define the acceleration measurement range of the accelerometer.  
Sensitivity is usually expressed as mV/g (millivolts or per g) or pC/g (picocoulombs per g), 
where g is the acceleration due to gravity or 9.81 m/s2; 
but digital output accelerometers will specify this as LSB/g (least significant bit per g).  

The sensitivity that you desire depends on the level of the signal you wish to measure.  
If you are interested in small vibrations then a higher sensitivity will be desirable to provide a cleaner signal (higher signal to noise ratio).  
When trying to measure higher amplitudes for shock events you will need a lower sensitivity.

Sensitivity describes the conversion between vibration and voltage at a reference frequency, such as 160 Hz. 
Sensitivity is specified in mV per G. 
If typical accelerometer sensitivity is 100 mV/G and you measure a 10 G signal, you expect a 1000 mV or 1 V output.

```

### FXOS8700CQ
```
Accelerometer sensitivity is 0.244 mg/LSB in 2 g mode, 0.488 mg/LSB in 4 g mode, and 0.976 mg/LSB in 8 g mode. 
A small, full scale range means that the sensitivity of the sensor is greater, resulting in a more precise reading out of the accelerometer.

FXOS8700CQ has dynamically selectable acceleration full-scale ranges of ±2 g/±4 g/±8 g 
and a fixed magnetic measurement range of ±1200 μT. 

The accelerometer sensitivity changes with the full scale range that you select in this parameter.

FXOS8700CQ has 14-bit accelerometer and 16-bit magnetometer resolution

Output data rates (ODR) from 1.563 Hz to 800 Hz are selectable by the user for each sensor. 
Interleaved magnetic and acceleration data is available at ODR rates of up to 400 Hz. 
```


## Links

<https://ozzmaker.com/accelerometer-to-g/>

<https://www.ni.com/en-us/innovations/white-papers/06/measuring-vibration-with-accelerometers.html>

<https://www.mathworks.com/help/supportpkg/freescalefrdmk64fboard/ref/fxos87006axessensor.html>

<https://stackoverflow.com/questions/19161872/meaning-of-lsb-unit-and-unit-lsb>

<https://www.digikey.com/catalog/en/partgroup/fxos8700cq/38337>

<https://www.nxp.com/docs/en/data-sheet/FXOS8700CQ.pdf?&pspll=1>

<https://os.mbed.com/users/janjongboom/code/fxos8700cq_example//file/cd2525cbbe4e/main.cpp/>

<https://community.nxp.com/docs/DOC-104587>

<https://community.nxp.com/docs/DOC-101425>

<https://community.nxp.com/docs/DOC-101385>  - K64

<https://os.mbed.com/platforms/FRDM-K66F/>

<https://os.mbed.com/forum/platform-228-FRDM-K66F-community/topic/28586/>
