#include <Wire.h>
#include <Adafruit_MLX90614.h>

// Create an instance of the Adafruit_MLX90614 class
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud
  
  // Initialize the I2C communication and the MLX90614 sensor
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX90614 sensor. Check wiring.");
    while (1); // Stop execution in case of failure
  }

  Serial.println("MLX90614 sensor initialized successfully!");
}

void loop() {
  // Read object temperature in Celsius
  float objectTemp = mlx.readObjectTempC();
  
  // Read ambient temperature in Celsius (optional)
  float ambientTemp = mlx.readAmbientTempC();
  
  // Print the temperatures
  Serial.print("Object Temperature: ");
  Serial.print(((objectTemp*9)/5)+32+10);
  Serial.println(" °F");


  // Wait for a second before taking the next reading
  delay(1000);
}
