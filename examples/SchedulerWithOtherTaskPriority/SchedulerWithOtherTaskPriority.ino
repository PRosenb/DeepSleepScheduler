#define SLEEP_DELAY 100
#include <DeepSleepScheduler.h>

void setup() {
  Serial.begin(115200);

  Serial.print("setup() FreeRTOS priority: ");
  Serial.print(uxTaskPriorityGet(NULL));
  Serial.print(" of ");
  Serial.println(configMAX_PRIORITIES);

  scheduler.schedule(task1);
}

void task1() {
  Serial.print("task1 FreeRTOS priority: ");
  Serial.println(uxTaskPriorityGet(NULL));
}

void loop() {
  // set the FreeRTOS task priority the scheduler will use for the tasks is schedules.
  vTaskPrioritySet(NULL, 6);
  scheduler.execute();
}

