# Arduino DeepSleepScheduler Library #
https://github.com/PRosenb/DeepSleepScheduler

DeepSleepScheduler is a lightweight, cooperative task scheduler library with configurable sleep and task supervision.  

## Features ##
- Easy to use
- Configurable sleep with `SLEEP_MODE_PWR_DOWN` or `SLEEP_MODE_IDLE` while no task is running
- Configurable task supervision using hardware watchdog
- Schedule in interrupt
- Small footprint

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
```c++
#include <DeepSleepScheduler.h>
#define LED_PIN 13

void ledOn() {
  digitalWrite(LED_PIN, HIGH);
  scheduler.scheduleDelayed(ledOff, 1000);
}

void ledOff() {
  digitalWrite(LED_PIN, LOW);
  scheduler.scheduleDelayed(ledOn, 1000);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  scheduler.schedule(ledOff);
}

void loop() {
  scheduler.execute();
}
```

## Examples ##
The following example sketches are included in the **DeepSleepScheduler** library.  
You can also see them in the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software) in menu File->Examples->DeepSleepScheduler.
- [**Blink**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/Blink/Blink.ino): The simple LED blink example above  
- [**ScheduleFromInterrupt**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/ScheduleFromInterrupt/ScheduleFromInterrupt.ino): Shows how you can schedule a callback on the main thread from an interrupt  
- [**ShowSleep**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/ShowSleep/ShowSleep.ino): Shows with the LED, when the CPU is in sleep or awake  
- [**Supervision**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/Supervision/Supervision.ino): Shows how to activate the task supervision in order to restart the CPU when a task takes too much time  
- [**PrintMillis**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/PrintMillis/PrintMillis.ino): Prints millis() and scheduler.getMillis() to show the difference
- [**SerialWithNoDeepSleepLock**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/SerialWithNoDeepSleepLock/SerialWithNoDeepSleepLock.ino): Shows how to use the methods `acquireNoDeepSleepLock()` and `releaseNoDeepSleepLock()` to allow serial write to finish before entering deep sleep
- [**AdjustSleepTimeCorrections**](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/AdjustSleepTimeCorrections/AdjustSleepTimeCorrections.ino): Shows how to adjust the sleep time corrections to your specific CPU

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
       Schedule the callback after delayMillis milliseconds.
       @param callback: the method to be called on the main thread
       @param delayMillis: the time to wait in milliseconds until the callback shall be made
    */
    void scheduleDelayed(void (*callback)(), unsigned long delayMillis);

    /**
       Schedule the callback uptimeMillis milliseconds after the device was started.
       @param callback: the method to be called on the main thread
       @param uptimeMillis: the time in milliseconds since the device was started
                            to schedule the callback.
    */
    void scheduleAt(void (*callback)(), unsigned long uptimeMillis);

    /**
       Schedule the callback method as next task even if other tasks are in the queue already.
       @param callback: the method to be called on the main thread
    */
    void scheduleAtFrontOfQueue(void (*callback)());

    /**
       Cancel all schedules that were scheduled for this callback.
       @param callback: method of which all schedules shall be removed
    */
    void removeCallbacks(void (*callback)());

    /**
       Acquire a lock to prevent the CPU from entering deep sleep.
       acquireNoDeepSleepLock() supports up to 255 locks.
       You need to call releaseNoDeepSleepLock() the same amount of times
       as removeCallbacks() to allow the CPU to enter deep sleep again.
    */
    void acquireNoDeepSleepLock();

    /**
       Release the lock acquired by acquireNoDeepSleepLock(). Please make sure you
       call releaseNoDeepSleepLock() the same amount of times as acquireNoDeepSleepLock(),
       otherwise the CPU is not allowed to enter deep sleep.
    */
    void releaseNoDeepSleepLock();

    /**
       return: true if the CPU is currently allowed to enter deep sleep, false otherwise.
    */
    bool doesDeepSleep();

    /**
       Configure the supervision of future tasks. Can be deactivated with NO_SUPERVISION.
       Default: TIMEOUT_8S
       @param taskTimeout: The task timeout to be used
    */
    void setTaskTimeout(TaskTimeout taskTimeout);

    /**
       return: The milliseconds since startup of the device where the sleep time was added
    */
    unsigned long getMillis();

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

All following options are to be set before the include where **no** `LIBCALL_DEEP_SLEEP_SCHEDULER` is defined:
- `#define AWAKE_INDICATION_PIN`: Show on a LED if the CPU is active or in sleep mode.  
HIGH = active, LOW = sleeping
- `#define SLEEP_TIME_XXX_CORRECTION`: Adjust the sleep time correction for the time when the CPU is in `SLEEP_MODE_PWR_DOWN` and waking up. See [Implementation Notes](#implementation-notes) and example [AdjustSleepTimeCorrections](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/AdjustSleepTimeCorrections/AdjustSleepTimeCorrections.ino).
- `#define QUEUE_OVERFLOW_PROTECTION`: Prevents, that the same callback is scheduled multiple times what may happen with repeated interrupts. When `QUEUE_OVERFLOW_PROTECTION` is set, the new insert callback is ignored, if an other one is found **before** the new one should be insert (due to optimisation).

## Implementation Notes ##
- The watchdog timer is used to wake the CPU up from `SLEEP_MODE_PWR_DOWN` and for task supervision. It can therefore not be used for other means.
- It is possible to schedule callbacks in interrupts. The run time of the `scheduleXX()` methods is relatively short but it blocks execution of other interrupts. If you have very time critical interrupts, they may still be blocked for too long.  
- No matter how callbacks were scheduled, they are always run on the main thread. The scheduler can therefore be used as a convenient way to pass control from an interrupt to the main thread.
- Definition and code are in the header file. It is done like this to allow the user to configure the library by using `#define`. You can still include the header file in multiple files of a project by using `#define LIBCALL_DEEP_SLEEP_SCHEDULER`. See [Define Options](#define-options).
- When the CPU enters `SLEEP_MODE_PWR_DOWN`, the watchdog timer is used to wake it up again. The accuracy of the watchdog timer is not very well though. Further, the wake up time depends on the CPU type you are using. If you have certain time constraints, it may happen, that the schedule times are not precise enough.  
One possibility is to adapt the sleep time corrections by setting the defines `SLEEP_TIME_XXX_CORRECTION` (see [Define Options](#define-options) and example [AdjustSleepTimeCorrections](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/AdjustSleepTimeCorrections/AdjustSleepTimeCorrections.ino)).  
An other option is to disable deep sleep (`SLEEP_MODE_PWR_DOWN`) while scheduling with tight time constraints. To do so, use the methods `acquireNoDeepSleepLock()` and `releaseNoDeepSleepLock()` (see [Methods](#methods) and example [SerialWithNoDeepSleepLock](https://github.com/PRosenb/DeepSleepScheduler/blob/master/examples/SerialWithNoDeepSleepLock/SerialWithNoDeepSleepLock.ino)). Please report values back to me if you do time measuring, thanks.

## Contributions ##
Enhancements and improvements are welcome.

## License ##
```
Arduino DeepSleepScheduler Library
Copyright (c) 2016 Peter Rosenberg (https://github.com/PRosenb).

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
