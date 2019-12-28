# Arduino DeepSleepScheduler Library #
https://github.com/PRosenb/DeepSleepScheduler

DeepSleepScheduler is a lightweight, cooperative task scheduler library with configurable sleep and task supervision.  

## Features ##
- Easy to use
- Configurable task supervision (using hardware watchdog on AVR)
- Schedule in interrupt
- Small footprint
- Supports multiple CPU architectures with the same API
  - AVR based Arduino boards like Arduino Uno, Mega, Nano etc.
  - ESP32
  - ESP8266 (no sleep support)
- Configurable sleep with `SLEEP_MODE_PWR_DOWN` or `SLEEP_MODE_IDLE` while no task is running (on AVR)

## Installation ##
- The library can be installed directly in the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software) as follows:
  - Menu Sketch->Include Library->Manage Libraries...
  - On top right in "Filter your search..." type: DeepSleepScheduler
  - The DeepSleepScheduler library will show
  - Click on it and then click "Install"
  - For more details see manual [Installing Additional Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries#toc3)
- If you do not use the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software):
  - [Download the latest version](https://github.com/PRosenb/DeepSleepScheduler/releases/latest)
  - Uncompress the downloaded file
  - This will result in a folder containing all the files for the library. The folder name includes the version: **DeepSleepScheduler-x.y.z**
  - Rename the folder to **DeepSleepScheduler**
  - Copy the renamed folder to your **libraries** folder
  - From time to time, check on https://github.com/PRosenb/DeepSleepScheduler if updates become available

## Getting Started ##
Simple blink:
```c++
#include <DeepSleepScheduler.h>

#ifdef ESP32
#include <esp_sleep.h>
#endif

bool ledOn = true;

void toggleLed() {
  if (ledOn) {
    ledOn = false;
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    ledOn = true;
    digitalWrite(LED_BUILTIN, LOW);
  }
  scheduler.scheduleDelayed(toggleLed, 1000);
}

void setup() {
#ifdef ESP32
  // ESP_PD_DOMAIN_RTC_PERIPH needs to be kept on
  // in order for the LED to stay on during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif

  pinMode(LED_BUILTIN, OUTPUT);
  scheduler.schedule(toggleLed);
}

void loop() {
  scheduler.execute();
}
```
Simple blink with Runnable:
```c++
#include <DeepSleepScheduler.h>

#ifdef ESP32
#include <esp_sleep.h>
#endif

class BlinkRunnable: public Runnable {
  private:
    bool ledOn = true;
    const byte ledPin;
    const int delay;
  public:
    BlinkRunnable(byte ledPin, int delay) : ledPin(ledPin), delay(delay) {
      pinMode(ledPin, OUTPUT);
    }
    virtual void run() {
      if (ledOn) {
        ledOn = false;
        digitalWrite(ledPin, HIGH);
      } else {
        ledOn = true;
        digitalWrite(ledPin, LOW);
      }
      scheduler.scheduleDelayed(this, delay);
    }
};

void setup() {
#ifdef ESP32
  // ESP_PD_DOMAIN_RTC_PERIPH needs to be kept on
  // in order for the LED to stay on during sleep
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif

  BlinkRunnable *blinkRunnable = new BlinkRunnable(LED_BUILTIN, 1000);
  scheduler.schedule(blinkRunnable);
}

void loop() {
  scheduler.execute();
}
```

## Examples ##
The following example sketches are included in the **DeepSleepScheduler** library.  
You can also see them in the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software) in menu File->Examples->DeepSleepScheduler.
### General ###
- [**Blink**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/Blink/Blink.ino): On other simple LED blink example  
- [**BlinkRunnable**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/BlinkRunnable/BlinkRunnable.ino): A simple LED blink example using Runnable  
- [**ScheduleRepeated**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/ScheduleRepeated/ScheduleRepeated.ino): Shows how to execute a repeated task. The library does not support it intrinsic to save memory.
- [**ScheduleFromInterrupt**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/ScheduleFromInterrupt/ScheduleFromInterrupt.ino): Shows how you can schedule a callback on the main thread from an interrupt  
- [**ShowSleep**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/ShowSleep/ShowSleep.ino): Shows with the LED, when the CPU is in sleep or awake  
- [**Supervision**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/Supervision/Supervision.ino): Shows how to activate the task supervision in order to restart the CPU when a task takes too much time  
- [**SupervisionWithCallback**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/SupervisionWithCallback/SupervisionWithCallback.ino): Shows how to activate the task supervision and get a callback when a task takes too much time  
- [**SerialWithDeepSleepDelay**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/SerialWithDeepSleepDelay/SerialWithDeepSleepDelay.ino): Shows how to use `SLEEP_DELAY` to allow serial write to finish before entering sleep
- [**PwmSleep**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/PwmSleep/PwmSleep.ino): Shows how to use analogWrite() and still use low power mode.  
### AVR Specific ###
- [**AdjustSleepTimeCorrections**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/AdjustSleepTimeCorrections/AdjustSleepTimeCorrections.ino): Shows how to adjust the sleep time corrections to your specific CPU
### ESP32 Specific ###
- [**SchedulerWithOtherTaskPriority**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/SchedulerWithOtherTaskPriority/SchedulerWithOtherTaskPriority.ino): Shows how to set an other FreeRTOS task priority for tasks scheduled by DeepSleepScheduler

## Reference ##
### Methods ###
```c++
/**
  Schedule the callback method as soon as possible but after other tasks
  that are to be scheduled immediately and are in the queue already.
  @param callback: the method to be called on the main thread
*/
void schedule(void (*callback)());
/**
  Schedule the Runnable as soon as possible but after other tasks
  that are to be scheduled immediately and are in the queue already.
  @param runnable: the Runnable on which the run() method will be called on the main thread
*/
void schedule(Runnable *runnable);

/**
  Schedule the callback method as soon as possible and remove all other
  tasks with the same callback. This is useful if you call it
  from an interrupt and want one execution only even if the interrupt triggers
  multiple times.
  @param callback: the method to be called on the main thread
*/
void scheduleOnce(void (*callback)());
/**
  Schedule the Runnable as soon as possible and remove all other
  tasks with the same Runnable. This is useful if you call it
  from an interrupt and want one execution only even if the interrupt triggers
  multiple times.
  @param runnable: the Runnable on which the run() method will be called on the main thread
*/
void scheduleOnce(Runnable *runnable);

/**
  Schedule the callback after delayMillis milliseconds.
  @param callback: the method to be called on the main thread
  @param delayMillis: the time to wait in milliseconds until the callback shall be made
*/
void scheduleDelayed(void (*callback)(), unsigned long delayMillis);
/**
  Schedule the callback after delayMillis milliseconds.
  @param runnable: the Runnable on which the run() method will be called on the main thread
  @param delayMillis: the time to wait in milliseconds until the callback shall be made
*/
void scheduleDelayed(Runnable *runnable, unsigned long delayMillis);

/**
  Schedule the callback uptimeMillis milliseconds after the device was started.
  Please be aware that uptimeMillis is stopped when no task is pending. In this case,
  the CPU may only wake up on an external interrupt.
  @param callback: the method to be called on the main thread
  @param uptimeMillis: the time in milliseconds since the device was started
                       to schedule the callback.
*/
void scheduleAt(void (*callback)(), unsigned long uptimeMillis);
/**
  Schedule the callback uptimeMillis milliseconds after the device was started.
  Please be aware that uptimeMillis is stopped when no task is pending. In this case,
  the CPU may only wake up on an external interrupt.
  @param runnable: the Runnable on which the run() method will be called on the main thread
  @param uptimeMillis: the time in milliseconds since the device was started
                       to schedule the callback.
*/
void scheduleAt(Runnable *runnable, unsigned long uptimeMillis);

/**
  Schedule the callback method as next task even if other tasks are in the queue already.
  @param callback: the method to be called on the main thread
*/
void scheduleAtFrontOfQueue(void (*callback)());
/**
  Schedule the callback method as next task even if other tasks are in the queue already.
  @param runnable: the Runnable on which the run() method will be called on the main thread
*/
void scheduleAtFrontOfQueue(Runnable *runnable);

/**
  Check if this callback is scheduled at least once already.
  This method can be called in an interrupt but bear in mind, that it loops through
  the run queue until it finds it or reaches the end.
  @param callback: callback to check
*/
bool isScheduled(void (*callback)()) const;

/**
  Check if this runnable is scheduled at least once already.
  This method can be called in an interrupt but bear in mind, that it loops through
  the run queue until it finds it or reaches the end.
  @param runnable: Runnable to check
*/
bool isScheduled(Runnable *runnable) const;

/**
  Returns the scheduled time of the task that is currently running.
  If no task is currently running, 0 is returned.
*/
unsigned long getScheduleTimeOfCurrentTask() const;

/**
  Cancel all schedules that were scheduled for this callback.
  @param callback: method of which all schedules shall be removed
*/
void removeCallbacks(void (*callback)());
/**
  Cancel all schedules that were scheduled for this runnable.
  @param runnable: instance of Runnable of which all schedules shall be removed
*/
void removeCallbacks(Runnable *runnable);

/**
  Acquire a lock to prevent the CPU from entering sleep.
  acquireNoSleepLock() supports up to 255 locks.
  You need to call releaseNoSleepLock() the same amount of times
  to allow the CPU to enter sleep again.
*/
void acquireNoSleepLock();

/**
  Release the lock acquired by acquireNoSleepLock(). Please make sure you
  call releaseNoSleepLock() the same amount of times as acquireNoSleepLock(),
  otherwise the CPU is not allowed to enter sleep.
*/
void releaseNoSleepLock();

/**
  return: true if the CPU is currently allowed to enter sleep, false otherwise.
*/
bool doesSleep() const;

/**
  Configure the supervision of future tasks. Can be deactivated with NO_SUPERVISION.
  Default: TIMEOUT_8S
  @param taskTimeout: The task timeout to be used
*/
void setTaskTimeout(TaskTimeout taskTimeout);

/**
   Resets the task watchdog. After this call returns, the currently running
   Task can run up to the configured TaskTimeout set by setTaskTimeout().
*/
void taskWdtReset();

/**
  return: The milliseconds since startup of the device where the sleep time was added.
          This value does not consider the time when the CPU is in infinite deep sleep
          while nothing is in the queue.
*/
unsigned long getMillis() const;

/**
  Sets the runnable to be called when the task supervision detects a task that runs too long.
  The run() method will be called from the watchdog interrupt what means, that
  e.g. the method delay() does not work.
  On AVR, when run() returns, the CPU will be restarted after 15ms.
  On ESP32, the interrupt service routine as a whole has a time limit and calls
  abort() when returning from this method.
  See description of SUPERVISION_CALLBACK and SUPERVISION_CALLBACK_TIMEOUT.
  @param runnable: instance of Runnable where the run() method is called
*/
void setSupervisionCallback(Runnable *runnable);

/**
  This method needs to be called from your loop() method and does not return.
*/
void execute();
```

### Enumerations ###
```c++
enum TaskTimeout {
  TIMEOUT_15Ms,
  TIMEOUT_30MS,
  TIMEOUT_60MS,
  TIMEOUT_120MS,
  TIMEOUT_250MS,
  TIMEOUT_500MS,
  TIMEOUT_1S,
  TIMEOUT_2S,
  TIMEOUT_4S,
  TIMEOUT_8S,
  NO_SUPERVISION
};
```

### Define Options ###
- `#define LIBCALL_DEEP_SLEEP_SCHEDULER`: The header file contains definition and implementation. For that reason, it can be included once only in a project. To use it in multiple files, define `LIBCALL_DEEP_SLEEP_SCHEDULER` before all include statements except one.

All following options are to be set **before** the include where **no** `LIBCALL_DEEP_SLEEP_SCHEDULER` is defined.

#### General options ####
- `#define SLEEP_DELAY`: Prevent the CPU from entering sleep for the specified amount of milliseconds after finishing the previous task.
- `#define SUPERVISION_CALLBACK`: Allows to specify a callback `Runnable` to be called when a task runs too long. When
    the callback returns, the CPU is restarted after 15 ms by the watchdog. The callback method is called directly
    from the watchdog interrupt. This means that e.g. `delay()` does not work.
- `#define SUPERVISION_CALLBACK_TIMEOUT`: Specify the timeout of the callback until the watchdog resets the CPU. Defaults to `WDTO_1S`.
- `#define AWAKE_INDICATION_PIN`: Show on a LED if the CPU is active or in sleep mode.  
HIGH = active, LOW = sleeping

#### AVR specific options ####
- `#define SLEEP_MODE`: Specifies the sleep mode entered when doing deep sleep. Default is `SLEEP_MODE_PWR_DOWN`.
- `#define MIN_WAIT_TIME_FOR_SLEEP`: Specify the minimum wait time (until the next task will be executed) to put the CPU in sleep mode. Default is 1 second.
- `#define SLEEP_TIME_XXX_CORRECTION`: Adjust the sleep time correction for the time when the CPU is in `SLEEP_MODE_PWR_DOWN` and waking up. See [Implementation Notes](#implementation-notes) and example [AdjustSleepTimeCorrections](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/AdjustSleepTimeCorrections/AdjustSleepTimeCorrections.ino).

#### ESP32 specific options ###
- `#ESP32_TASK_WDT_TIMER_NUMBER`: Specifies the timer number to be used for task supervision. Default is 3.

#### ESP8266 specific options ####
- `ESP8266_MAX_DELAY_TIME_MS`: The maximum time in milliseconds the CPU will be delayed while no task is scheduled. Default is 7000 due to the watchdog timeout of 8 seconds. Set this value lower if you expect interrupts while no task is running.

## Implementation Notes ##
### General ###
- Definition and code are in the header file. It is done like this to allow the user to configure the library by using `#define`. You can still include the header file in multiple files of a project by using `#define LIBCALL_DEEP_SLEEP_SCHEDULER`. See [Define Options](#define-options).
- It is possible to schedule callbacks in interrupts. The run time of the `scheduleXX()` methods is relatively short but it blocks execution of other interrupts. If you have very time critical interrupts, they may still be blocked for too long.  
- No matter how callbacks were scheduled, they are always run on the thread that runs the scheduler.execute() function. The scheduler can therefore be used as a convenient way to pass control from an interrupt to a regular thread.

### AVR ###
- On AVR the watchdog timer is used to wake the CPU up from `SLEEP_MODE_PWR_DOWN` and for task supervision. It can therefore not be used for other means.
- When the CPU enters `SLEEP_MODE_PWR_DOWN`, the watchdog timer is used to wake it up again. The accuracy of the watchdog timer is not very well though. Further, the wake up time depends on the CPU type you are using. If you have certain time constraints, it may happen, that the schedule times are not precise enough.  
One possibility is to adapt the sleep time corrections by setting the defines `SLEEP_TIME_XXX_CORRECTION` (see [Define Options](#define-options) and example [AdjustSleepTimeCorrections](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/AdjustSleepTimeCorrections/AdjustSleepTimeCorrections.ino)).  
An other option is to disable sleep (`SLEEP_MODE_PWR_DOWN`) while scheduling with tight time constraints. To do so, use the methods `acquireNoSleepLock()` and `releaseNoSleepLock()` (see [Methods](#methods)). Please report values back to me if you do time measuring, thanks.
- While the CPU is in `SLEEP_MODE_PWR_DOWN`, the millis timer is not running. For this reason the current uptime is not known when an external interrupt occurs during this time. Instead of the current uptime, the uptime when the CPU started to sleep is taken when calculating the schedule time of a delayed task. This  means that these tasks are potentially scheduled too early because the uptime is corrected when the sleep time is finished.

### ESP32 ###
- At time of writing, the ESP32 implementation available in the Arduino IDE does not allow access to the hardware watchdog of ESP32. To still allow supervision of the tasks, DeepSleepScheduler employs timer 3 to measure the time and restart the CPU if a task runs too long. See [Define Options](#define-options) on how to change the timer.
- On ESP32 FreeRTOS is used. It allows to run multiple threads in parallel and manages their switching and prioritisation. DeepSleepScheduler (that also runs on memory constrained CPUs) is a cooperative task scheduler that runs all tasks on the thread that calls scheduler.execute(). The advantage of that is, that there is no need to synchronize the tasks against each other. On the other hand, they do not run in parallel. To change the FreeRTOS priority of all tasks run by DeepSleepScheduler, set it before scheduler.execute() is called. See [SchedulerWithOtherTaskPriority](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/SchedulerWithOtherTaskPriority/SchedulerWithOtherTaskPriority.ino) for details.

## Contributions ##
Enhancements and improvements are welcome.

## License ##
```
Arduino DeepSleepScheduler Library
Copyright (c) 2018 Peter Rosenberg (https://github.com/PRosenb).

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
