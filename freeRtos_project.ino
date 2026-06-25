#include<Arduino.h>
#include <DHT.h>         

DHT dht(2,DHT11);
const int LED_PIN=4;
QueueHandle_t sensorQueue;


typedef struct
{
  float temperature;
  float humidity;
} SensorData;



void ledTask(void *pvParameters)
{
  pinMode(LED_PIN, OUTPUT);

  while (1)
  {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));

    digitalWrite(LED_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}



void sensorTask(void *pvParameters)
{
  SensorData data;

  while (1)
  {
    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();

    
    if (!isnan(data.temperature) && !isnan(data.humidity))
    {
      if (xQueueSend(sensorQueue, &data, pdMS_TO_TICKS(100)) == pdPASS)
      {
        Serial.println("Data Sent");
      }
    }
    else
    {
      Serial.println("DHT Read Failed");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}



void serialTask(void *pvParameters)
{
  SensorData receivedData;

  while (1)
  {
    if (xQueueReceive(sensorQueue, &receivedData, portMAX_DELAY) == pdTRUE)
    {
      Serial.println("-------------------------");

      Serial.print("Temperature : ");
      Serial.print(receivedData.temperature);
      Serial.println(" °C");

      Serial.print("Humidity : ");
      Serial.print(receivedData.humidity);
      Serial.println(" %");

      Serial.println("-------------------------");
    }
  }
}


void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("Setup Started");

  dht.begin();

  sensorQueue = xQueueCreate(5, sizeof(SensorData));

  if (sensorQueue == NULL)
  {
    Serial.println("Queue Creation Failed!");
    while (1);
  }

  Serial.println("Queue Created");

  xTaskCreate(
      ledTask,
      "LED Task",
      2048,
      NULL,
      1,
      NULL);

  xTaskCreate(
      sensorTask,
      "Sensor Task",
      4096,
      NULL,
      1,
      NULL);

  xTaskCreate(
      serialTask,
      "Serial Task",
      4096,
      NULL,
      1,
      NULL);
}

void loop()
{
}