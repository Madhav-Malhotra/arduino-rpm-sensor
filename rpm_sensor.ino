// constants
#define TRANSMITTER 9



// global variables
// for tracking timestamps of receiver signal changes
unsigned long signals[4] = {0, 0, 0, 0};
unsigned long last_signal = 0;
bool last_receive_high = false;

// for checking ambiant light levels
unsigned short avg_laser = 0;
unsigned short avg_laser_off = 0;
unsigned short threshold = 0;



void setup() {
  // Initialising pins and communication
  Serial.begin(9600);
  Serial.println("Started communications");
  pinMode(TRANSMITTER, OUTPUT);


  // Get normal receiver levels without laser. 
  for (unsigned char i = 0; i < 250; i++) {
    avg_laser_off += analogRead(A0);
    avg_laser_off /= 2;
    delay(10);
  }


  // Get normal receiver levels with laser.
  digitalWrite(TRANSMITTER, HIGH);

  for (unsigned char i = 0; i < 250; i++) {
    avg_laser += analogRead(A0);
    avg_laser /= 2;
    delay(10);
  }


  // Show test results
  Serial.println(avg_laser);
  Serial.println(avg_laser_off);
  threshold = (avg_laser_off + avg_laser) / 2;
}



void loop() {
  // read signal
  bool const receive_high = (analogRead(A0) > threshold);

  // if new receive_high, record time
  if (receive_high && !last_receive_high) {
    last_signal = millis();
    track_time(last_signal);
  } 

  last_receive_high = receive_high;
}



// @brief                         - Records time at which a gear piece goes past sensor
// @param new_time                - A new time to record where sensor detection ocurred
// @return                        - void, but calls log_speed() eventually
// @warn                          - relies on global variable: signals
void track_time(unsigned long new_time) {

  // prepare to write into signals array
  unsigned char const max_records = sizeof(signals) / sizeof(signals[0]);

  for (unsigned char i = 0; i < max_records; i++) {
    // write into first empty record
    if (signals[i] == 0) {
      signals[i] = new_time;
      break;
    }
  }

  // If array full, the wheel has finished rotating one turn. 
  // Output a rpm based on rotation speed. 
  if (signals[max_records - 1] != 0) {
    log_speed();
  }

}



// @brief                         - Logs the speed [rpm] of the turning wheel
// @param                         - void
// @return                        - void, but displays to serial monitor
// @warn                          - relies on global variable: signals
void log_speed() {

  // Get time taken for one turn
  unsigned char const max_records = sizeof(signals) / sizeof(signals[0]);
  unsigned long turn_duration = 0;  // [ms per turn]

  for (unsigned char i = 0; i < max_records - 1; i++) {
    turn_duration += (signals[i+1] - signals[i]);
  }

  
  // Convert time per turn to rpm
  // the conversion from ms per revolution to revolutions per minute
  // would be more clear as (1 / turn_duration) * 60000
  // but this would also cause more floating point inaccuracy
  double rpm = 60000 / turn_duration;
  Serial.println(rpm);


  // reset signals
  for (unsigned char i = 0; i < max_records; i++) {
    signals[i] = 0;
  }
}