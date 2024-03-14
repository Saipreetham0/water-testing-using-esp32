/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL3efdfLfAs"
#define BLYNK_TEMPLATE_NAME "ESP32 Blynk"
#define BLYNK_AUTH_TOKEN "zqREgx13hgQUIdhPk52KzDXkuAlUUXaD"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "KSP";
char pass[] = "9550421866";

int sensorValue;
int value;
long Stime = 0;
long Ltime = 0;
int count = 0;

int BodyTemp;

const int trigPin = 5;
const int echoPin = 18;

// define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

int turbidity = map(sensorValue, 0, 750, 100, 0);

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

BlynkTimer timer;

#define SensorPin 34        // pH meter Analog output to Arduino Analog Input
unsigned long int avgValue; // Store the average value of the sensor feedback
float b;
int buf[10], temp;

int f;     // for float value to string converstion
float val; // also works with double.
char buff2[10];
String valueString = "";
String Value = "";

float phValue = (float)avgValue * 3.3 / 1024 / 6; // convert the analog into millivolt

//

// turbidty

int turbidtySensorPin = 35;

void updateLabels(int phValue, int turbidity, int bodyTemp)
{
  // Conditions for pH
  String phLabel = "";
  String phColor = "";
  if (phValue >= 6.5 && phValue <= 8.5)
  {
    phLabel = "Drinking Water pH: 6.5-8.5";
    phColor = "#2ecc71"; // Green color for drinking water
  }
  else if (phValue >= 5 && phValue <= 7)
  {
    phLabel = "Pharmaceutical and Medicinal pH: 5-7";
    phColor = "#3498db"; // Blue color for pharmaceutical and medicinal
  }
  else if (phValue >= 7 && phValue <= 9)
  {
    phLabel = "Industry (Paper Manufacturing) pH: 7-9";
    phColor = "#e74c3c"; // Red color for industry
  }

  // Conditions for turbidity
  String turbidityLabel = "";
  String turbidityColor = "";
  if (turbidity <= 5)
  {
    turbidityLabel = "Turbidity: <= 5 NTU";
    turbidityColor = "#2ecc71"; // Green color for drinking water
  }
  else if (turbidity <= 1)
  {
    turbidityLabel = "Turbidity: <= 1 NTU";
    turbidityColor = "#3498db"; // Blue color for pharmaceutical and medicinal
  }
  else if (turbidity < 5)
  {
    turbidityLabel = "Turbidity: < 5 NTU";
    turbidityColor = "#e74c3c"; // Red color for industry
  }

  // Conditions for temperature
  String tempLabel = "";
  String tempColor = "";
  if ((bodyTemp >= 10 && bodyTemp <= 15.6) || (bodyTemp >= 50 && bodyTemp <= 60))
  {
    tempLabel = "Temperature: 10-15.6°C or 50-60°F";
    tempColor = "#2ecc71"; // Green color for drinking water
  }
  else if (bodyTemp >= 20 && bodyTemp <= 25)
  {
    tempLabel = "Temperature: 20-25°C";
    tempColor = "#3498db"; // Blue color for pharmaceutical and medicinal
  }
  else if (bodyTemp >= 10 && bodyTemp <= 35)
  {
    tempLabel = "Temperature: 10-35°C";
    tempColor = "#e74c3c"; // Red color for industry
  }

  // Update label V4
  String combinedLabel = phLabel + "\n" + turbidityLabel + "\n" + tempLabel;
  String combinedColor = phColor + "\n" + turbidityColor + "\n" + tempColor;

  Blynk.setProperty(V4, "label", combinedLabel);
  Blynk.setProperty(V4, "labelColor", combinedColor);
}

void turbidtyValue()
{
  int sensorValue = analogRead(turbidtySensorPin);
  Serial.println(sensorValue);

  Blynk.virtualWrite(V2, turbidity);
  Serial.print("turbidity");
  Serial.print(turbidity);
  delay(100);
}

void PH_Value()
{
  for (int i = 0; i < 10; i++) // Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) // sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++) // take the average value of 6 center sample
    avgValue += buf[i];
  // float phValue = (float)avgValue * 3.3 / 1024 / 6; // convert the analog into millivolt
  phValue = 3.5 * phValue; // convert the millivolt into pH value

  Value = dtostrf(phValue, 4, 2, buff2); // 4 is mininum width, 6 is precision
  Serial.print(Value);
  valueString = "";
  delay(1000);
  // Blynk.virtualWrite(V0, 7.06);
  Blynk.virtualWrite(V0, phValue);
  Serial.print("phValue");
  Serial.print(phValue);
}

void TempSensor()
{
  // Send the command to get temperatures
  sensors.requestTemperatures();
  // print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print((char)176); // shows degrees character
  Serial.print("C  |  ");
  BodyTemp = sensors.getTempCByIndex(0);
  Blynk.virtualWrite(V1, BodyTemp);
  Serial.print(BodyTemp);
  delay(500);
}

void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second

  // timer.setInterval(1000L, TempSensor);
  timer.setInterval(1000L, TempSensor);
  timer.setInterval(1000L, turbidtyValue);
  timer.setInterval(1000L, PH_Value);
}

void loop()
{
  Blynk.run();
  timer.run();

  updateLabels(phValue, turbidity, BodyTemp);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;

  Blynk.virtualWrite(V1, distanceCm); // Use V1 as a display widget in your Blynk app

  // Convert to inches
  // distanceInch = distanceCm * CM_TO_INCH;

  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  // Serial.print("Distance (inch): ");
  // Serial.println(distanceInch);

  delay(1000);
}