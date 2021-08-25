#include <ADS126X.h>
#include <Modbus.h>

ADS126X adc;

ModbusRTUServerClass ModbusRTUServer;
int chip_select = 11; // Arduino pin connected to CS on ADS126X

const int numCoils = 10;
const int numDiscreteInputs = 10;
const int numHoldingRegisters = 10;
const int numInputRegisters = 10;

void setup() {
//  Serial.begin(115200);

  PORTA = 0b00001100;                         // Switch on Pullup-Resistors
  PORTB = 0b00111001;                         // to get idle high on unused ports
  PORTC = 0b11111111;
  PORTD = 0b11001111;
  
  DDRA =  0b11110000;
  DDRB =  0b10111010;
  DDRC =  0b00000000;
  DDRD =  0b00110010;

  PORTA |= 0x20;  // Yellow led on

  if (!ModbusRTUServer.begin(&RS485_0, 1, 19200, SERIAL_8N2)) {
    while (1);
  }
  RS485_0.receive();

  // configure coils at address 0x00
  ModbusRTUServer.configureCoils(0x00, numCoils);

  // configure discrete inputs at address 0x00
  ModbusRTUServer.configureDiscreteInputs(0x00, numDiscreteInputs);

  // configure holding registers at address 0x00
  ModbusRTUServer.configureHoldingRegisters(0x00, numHoldingRegisters);

  // configure input registers at address 0x00
  ModbusRTUServer.configureInputRegisters(0x00, numInputRegisters);

  // LED outputs
  DDRA |= 0xF0;


  // CS for EEPROM is output
  DDRB |= (1 << 4);
  PORTB |= (1 << 4);

  // Un-reset the ADC
  DDRB |= (1 << 1);
  PORTB |= (1 << 1);
  //pinMode(1, OUTPUT);
  //digitalWrite(1, HIGH);

  // Input for ADC ready
  pinMode(2, INPUT);

  adc.begin(chip_select);                     // setup with chip select pin
  adc.setIDAC1Pin(8);                         // Route excitation current source 1 to sensor bridge
  adc.setIDAC1Mag(ADS126X_IDAC_MAG_500);      // Set excitation current (min 100 for valid Vref)
  adc.setGain(ADS126X_GAIN_8);
  //adc.setRate(ADS126X_RATE_2_5);
  adc.setRate(ADS126X_RATE_100);
  adc.setReference(ADS126X_REF_NEG_AIN1, ADS126X_REF_POS_AIN0);
  //adc.setReference(ADS126X_REF_NEG_INT, ADS126X_REF_POS_INT);
  adc.enableInternalReference();
  adc.setDelay(ADS126X_DELAY_2_2);

  delay(3000);  // Wait for PSU to settle
  adc.setContinuousMode();
  adc.startADC1();
  adc.calibrateSelfOffsetADC1();
  PORTA |= 0x20;  // Yellow led on
  while (digitalRead(2));  // wait for adc calibration ready
  PORTA &= ~0x20; // Yellow led off
  //delay(8000);  // Wait for calibration
  digitalWrite(chip_select, true);  // Set CS back high again

  adc.setPulseMode();
  adc.setFilter(ADS126X_SINC4);
  adc.startADC1(); // start conversion on ADC1
}

void loop() {
  /*
    // Sample chip temperature
    long raw = adc.readADC1(ADS126X_TEMP,ADS126X_TEMP);
    //Serial.println(adc.lastADC1Status());
    // Internal ref is 2.5 V
    double voltage = (double) raw / 0x80000000 * 2.5;
    //Serial.println(voltage, 8);
    double chipTemperature = (voltage - 0.1224) / 420e-6 + 25.0;
    Serial.print(chipTemperature, 2);
    Serial.print(" °C on Chip");

    delay(3000); // wait 1 second
    PORTA ^= 0x30;  // Toggle some LEDs
  */

  /*
    // Sample rtd temperature
    long raw = adc.readADC1(ADS126X_AIN2, ADS126X_AIN3); // Bridge differential measurement
    //long raw = adc.readADC1(ADS126X_AIN4, ADS126X_AIN5); // Single ended rtd measurement
    //double resistance = (double) raw / 0x80000000 * 5e3 * 2.0;  // Single ended measurement, 5kOhm reference with bridge dividing current in half
    double resistance = (double) raw / 0x80000000 * 5e3 * 2.0 + 100.0;  // Bridge measurement, 5kOhm reference with bridge dividing current in half
    const double alpha = 0.00391;
    double rtdTemperature = (resistance / 100.0 - 1.0) / alpha;
    //Serial.print(resistance, 6);
    //Serial.println(" Ohm RTD");
    Serial.print(rtdTemperature, 4);
    Serial.println(" °C RTD");
  */
  double rtdTemperature = 0.0;
  for (int i = 0; i < 100; i++)
  {
    PORTA |= 0x10;  // Green led on
    while (digitalRead(2))  // wait for adc ready
    {
    }
    PORTA &= ~0x10; // Green led off

    // 3rd order polynomial approximation for PGA = 8
    long raw = adc.readADC1(ADS126X_AIN2, ADS126X_AIN3); // Bridge differential measurement
    double x = (double) raw / 0x80000000; // ADC counts / fullscale
    rtdTemperature += 3.6409923e5 * pow(x, 3) + 4.5934421e4 * pow(x, 2) + 6.3967439e3 * x;
    //Serial.print(rtdTemperature, 4);
    //Serial.println(" °C RTD");


    adc.startADC1(); // start conversion on ADC1

  }
/*
  if (adc.lastADC1LowReferenceAlarm())
    Serial.println("Low ref alarm!");
  else if (adc.lastADC1PGAOutputLowAlarm())
    Serial.println("PGA out low alarm!");
  else if (adc.lastADC1PGAOutputHighAlarm())
    Serial.println("PGA out high alarm!");
  else if (adc.lastADC1PGADifferentialOutputAlarm())
    Serial.println("PGA diff out alarm!");
  else
  {
    Serial.print(rtdTemperature/100.0, 9);
    Serial.println(" °C RTD");
  }
*/
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();

  
/*  // map the coil values to the discrete input values
  for (int i = 0; i < numCoils; i++) {
    int coilValue = ModbusRTUServer.coilRead(i);

    ModbusRTUServer.discreteInputWrite(i, coilValue);
  }

  // map the holiding register values to the input register values
  for (int i = 0; i < numHoldingRegisters; i++) {
    long holdingRegisterValue = ModbusRTUServer.holdingRegisterRead(i);

    ModbusRTUServer.inputRegisterWrite(i, holdingRegisterValue);
  }*/

  ModbusRTUServer.inputRegisterWrite(0x00, (uint16_t) (rtdTemperature * 1000.0));
}
