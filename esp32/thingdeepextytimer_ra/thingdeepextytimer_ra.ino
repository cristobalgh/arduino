/*
Deep Sleep despierta con apretar boton o por timer interno

NOTE:
======
Only RTC IO can be used as a source for external wake
source. They are pins: 0,2,4,12-15,25-27,32-39.
*/

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* factor para sleep time en s) */
#define tt 50 //ms de parpadeo

RTC_DATA_ATTR int bootCount = 0;

void blink(int i){
  int k = 0;
  for (k = 0; k<=i; k++){
    digitalWrite(5, HIGH);
    delay(tt);
    digitalWrite(5, LOW);
    delay(tt);
    }
  }

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void setup(){
  Serial.begin(115200);
  randomSeed(analogRead(0));
  delay(100); //Take some time to open up the Serial Monitor

  pinMode(5, OUTPUT); //led de placa

  //Increment boot number and print it every reboot
  ++bootCount;
  int blinks = random(1,5)*log(bootCount);
  Serial.print("Seran: ");
  Serial.print(blinks);
  Serial.println(" blinks.");
  blink(blinks);
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up for an external trigger.
  There are two types for ESP32, ext0 and ext1 .
  ext0 uses RTC_IO to wakeup thus requires RTC peripherals
  to be on while ext1 uses RTC Controller so doesnt need
  peripherals to be powered on.
  Note that using internal pullups/pulldowns also requires
  RTC peripherals to be turned on.
  */
  int sleept = random(1,5)*TIME_TO_SLEEP*log(bootCount);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0,0); //1 = High, 0 = Low
  esp_sleep_enable_timer_wakeup(sleept * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(sleept) +
  " Seconds");

  //If you were to use ext1, you would use it like
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);

  //Go to sleep now
  Serial.println("Going to sleep now");
  delay(100);//para alcanzar a escribir
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop(){
  //This is not going to be called
}
