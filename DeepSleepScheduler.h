/*
    Copyright 2016-2016 Peter Rosenberg

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
  This file is part of the DeepSleepScheduler library for Arduino.
  Definition and code are in the header file in order to allow the user to configure the library by using defines.

  The following options are available:
  - #define LIBCALL_DEEP_SLEEP_SCHEDULER: This h file can only be included once within a project as it also contains the implementation.
    To use it in multiple files, define LIBCALL_DEEP_SLEEP_SCHEDULER before all include statements except one.
  All following options are to be set before the include where no LIBCALL_DEEP_SLEEP_SCHEDULER is defined.
  - #define SLEEP_MODE: Specifies the sleep mode entered when doing deep sleep. Default is SLEEP_MODE_PWR_DOWN.
  - #define SLEEP_DELAY: Prevent the CPU from entering sleep for the specified amount of milli seconds after finishing the previous task.
  - #define SUPERVISION_CALLBACK: Allows to specify a callback Runnable to be called when a task runs too long. When
    the callback returns, the CPU is restarted after 15 ms by the watchdog. The callback method is called directly
    from the watchdog interrupt. This means that e.g. delay() does not work.
  - #define SUPERVISION_CALLBACK_TIMEOUT: Specify the timeout of the callback until the watchdog resets the CPU. Defaults to WDTO_1S.
  - #define AWAKE_INDICATION_PIN: Show on a LED if the CPU is active or in sleep mode. HIGH = active, LOW = sleeping.
  - #define SLEEP_TIME_XXX_CORRECTION: When the CPU wakes up from SLEEP_MODE_PWR_DOWN, it needs some cycles to get active. This is also dependent on
    the used CPU type. Using the constants SLEEP_TIME_15MS_CORRECTION to SLEEP_TIME_8S_CORRECTION you can define more exact values for your
    CPU. Please report values back to me if you do some measuring, thanks.
*/

#ifndef DEEP_SLEEP_SCHEDULER_H
#define DEEP_SLEEP_SCHEDULER_H

// -------------------------------------------------------------------------------------------------
// Definition (usually in H file)
// -------------------------------------------------------------------------------------------------
#include <Arduino.h>

#define BUFFER_TIME 2
#define NOT_USED 255

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

/**
   Extend from Runnable in order to have the run() method run by the scheduler.
*/
class Runnable {
  public:
    virtual void run() = 0;
};

class Scheduler {
  public:
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
       return: The milliseconds since startup of the device where the sleep time was added.
               This value does not consider the time when the CPU is in infinite deep sleep
               while nothing is in the queue.
    */
    virtual unsigned long getMillis() const = 0;

#ifdef SUPERVISION_CALLBACK
#ifdef ESP8266
#error "SUPERVISION_CALLBACK not supported for ESP8266"
#endif
    /**
       Sets the runnable to be called when the task supervision detects a task that runs too long.
       The run() method will be called from the watchdog interrupt what means, that
       e.g. the method delay() does not work. When run() returns, the CPU will be restarted after 15ms.
       See description of SUPERVISION_CALLBACK and SUPERVISION_CALLBACK_TIMEOUT.
       @param runnable: instance of Runnable where the run() method is called
    */
    void setSupervisionCallback(Runnable *runnable) {
      supervisionCallbackRunnable = runnable;
    }
#endif

    /**
       This method needs to be called from your loop() method and does not return.
    */
    void execute();

    /**
       Constructor of the scheduler. Do not all this method as there is only one instance of Scheduler supported.
    */
    Scheduler();

  private:
    class Task {
      public:
        Task(const unsigned long scheduledUptimeMillis)
          : scheduledUptimeMillis(scheduledUptimeMillis), next(NULL) {
        }
        virtual ~Task() {}
        virtual void execute() = 0;
        // dynamic_cast is not supported by default as it compiles with -fno-rtti
        // Therefore, we use this method to detect which Task type it is.
        virtual bool isCallbackTask() = 0;
        const unsigned long scheduledUptimeMillis;
        Task *next;
    };
    class CallbackTask: public Task {
      public:
        CallbackTask(void (*callback)(), const unsigned long scheduledUptimeMillis)
          : Task(scheduledUptimeMillis), callback(callback) {
        }
        virtual void execute() {
          callback();
        }
        virtual bool isCallbackTask() {
          return true;
        }
        void (* const callback)();
    };
    class RunnableTask: public Task {
      public:
        RunnableTask(Runnable *runnable, const unsigned long scheduledUptimeMillis)
          : Task(scheduledUptimeMillis), runnable(runnable) {
        }
        virtual void execute() {
          runnable->run();
        }
        virtual bool isCallbackTask() {
          return false;
        }
        Runnable * const runnable;
    };

    /**
      controls if sleep is done, 0 does sleep
    */
    byte noSleepLocksCount;

    void insertTask(Task *task);

  protected:
    enum SleepMode {
      NO_SLEEP,
      IDLE,
      SLEEP
    };

#ifdef SUPERVISION_CALLBACK
    static Runnable *supervisionCallbackRunnable;
#endif

    /**
       currently set task timeout
    */
    TaskTimeout taskTimeout;
    /**
       first element in the run queue
    */
    Task *first;
    /*
       the task currently running or null if none running
    */
    Task *current;
#ifdef SLEEP_DELAY
    /**
       The time in millis since start up when the last task finished.
       Used to delay deep sleep.
    */
    unsigned long lastTaskFinishedMillis;
#endif

    virtual void taskWdtEnable(const uint8_t value) = 0;
    virtual void taskWdtReset() = 0;
    virtual void taskWdtDisable() = 0;
    virtual void sleepIfRequired() = 0;
    // only used by AVR as the watchdog timer is used
    virtual bool isWakeupByOtherInterrupt() {
      return false;
    }

    // default does nothing, only used for AVR
    virtual void wdtEnableInterrupt() {}
};

#ifndef LIBCALL_DEEP_SLEEP_SCHEDULER
// -------------------------------------------------------------------------------------------------
// Implementation (usuallly in CPP file)
// -------------------------------------------------------------------------------------------------

#ifdef SUPERVISION_CALLBACK
Runnable *Scheduler::supervisionCallbackRunnable;
#endif

Scheduler::Scheduler() {
#ifdef AWAKE_INDICATION_PIN
  pinMode(AWAKE_INDICATION_PIN, OUTPUT);
#endif
  taskTimeout = TIMEOUT_8S;

  first = NULL;
  current = NULL;
  noSleepLocksCount = 0;
}

void Scheduler::schedule(void (*callback)()) {
  Task *newTask = new CallbackTask(callback, getMillis());
  insertTask(newTask);
}

void Scheduler::schedule(Runnable *runnable) {
  Task *newTask = new RunnableTask(runnable, getMillis());
  insertTask(newTask);
}

void Scheduler::scheduleDelayed(void (*callback)(), unsigned long delayMillis) {
  Task *newTask = new CallbackTask(callback, getMillis() + delayMillis);
  insertTask(newTask);
}

void Scheduler::scheduleDelayed(Runnable *runnable, unsigned long delayMillis) {
  Task *newTask = new RunnableTask(runnable, getMillis() + delayMillis);
  insertTask(newTask);
}

void Scheduler::scheduleAt(void (*callback)(), unsigned long uptimeMillis) {
  Task *newTask = new CallbackTask(callback, uptimeMillis);
  insertTask(newTask);
}

void Scheduler::scheduleAt(Runnable *runnable, unsigned long uptimeMillis) {
  Task *newTask = new RunnableTask(runnable, uptimeMillis);
  insertTask(newTask);
}

void Scheduler::scheduleAtFrontOfQueue(void (*callback)()) {
  Task *newTask = new CallbackTask(callback, getMillis());
  noInterrupts();
  newTask->next = first;
  first = newTask;
  interrupts();
}

void Scheduler::scheduleAtFrontOfQueue(Runnable *runnable) {
  Task *newTask = new RunnableTask(runnable, getMillis());
  noInterrupts();
  newTask->next = first;
  first = newTask;
  interrupts();
}

bool Scheduler::isScheduled(void (*callback)()) const {
  bool scheduled = false;
  noInterrupts();
  Task *currentTask = first;
  while (currentTask != NULL) {
    if (currentTask->isCallbackTask() && ((CallbackTask*)currentTask)->callback == callback) {
      scheduled = true;
      break;
    }
    currentTask = currentTask->next;
  }
  interrupts();
  return scheduled;
}

bool Scheduler::isScheduled(Runnable *runnable) const {
  bool scheduled = false;
  noInterrupts();
  Task *currentTask = first;
  while (currentTask != NULL) {
    if (!currentTask->isCallbackTask() && ((RunnableTask*)currentTask)->runnable == runnable) {
      scheduled = true;
      break;
    }
    currentTask = currentTask->next;
  }
  interrupts();
  return scheduled;
}

unsigned long Scheduler::getScheduleTimeOfCurrentTask() const {
  noInterrupts();
  if (current != NULL) {
    return current->scheduledUptimeMillis;
  }
  interrupts();
  return 0;
}

void Scheduler::removeCallbacks(void (*callback)()) {
  noInterrupts();
  if (first != NULL) {
    Task *previousTask = NULL;
    Task *currentTask = first;
    while (currentTask != NULL) {
      if (currentTask->isCallbackTask() && ((CallbackTask*)currentTask)->callback == callback) {
        Task *taskToDelete = currentTask;
        if (previousTask == NULL) {
          // remove the first task
          first = taskToDelete->next;
        } else {
          previousTask->next = taskToDelete->next;
        }
        currentTask = taskToDelete->next;
        delete taskToDelete;
      } else {
        previousTask = currentTask;
        currentTask = currentTask->next;
      }
    }
  }
  interrupts();
}

void Scheduler::removeCallbacks(Runnable *runnable) {
  noInterrupts();
  if (first != NULL) {
    Task *previousTask = NULL;
    Task *currentTask = first;
    while (currentTask != NULL) {
      if (!currentTask->isCallbackTask() && ((RunnableTask*)currentTask)->runnable == runnable) {
        Task *taskToDelete = currentTask;
        if (previousTask == NULL) {
          // remove the first task
          first = taskToDelete->next;
        } else {
          previousTask->next = taskToDelete->next;
        }
        currentTask = taskToDelete->next;
        delete taskToDelete;
      } else {
        previousTask = currentTask;
        currentTask = currentTask->next;
      }
    }
  }
  interrupts();
}

void Scheduler::acquireNoSleepLock() {
  noSleepLocksCount++;
}

void Scheduler::releaseNoSleepLock() {
  if (noSleepLocksCount != 0) {
    noSleepLocksCount--;
  }
}

bool Scheduler::doesSleep() const {
  return noSleepLocksCount == 0;
}

void Scheduler::setTaskTimeout(TaskTimeout taskTimeout) {
  noInterrupts();
  this->taskTimeout = taskTimeout;
  interrupts();
}

// Inserts a new task in the ordered lists of tasks.
void Scheduler::insertTask(Task *newTask) {
  noInterrupts();
  if (first == NULL) {
    first = newTask;
  } else {
    if (first->scheduledUptimeMillis > newTask->scheduledUptimeMillis) {
      // insert before first
      newTask->next = first;
      first = newTask;
    } else {
      Task *currentTask = first;
      while (currentTask->next != NULL
             && currentTask->next->scheduledUptimeMillis <= newTask->scheduledUptimeMillis) {
        currentTask = currentTask->next;
      }
      // insert after currentTask
      newTask->next = currentTask->next;
      currentTask->next = newTask;
    }
  }
  interrupts();
}

void Scheduler::execute() {
  noInterrupts();
  if (taskTimeout != NO_SUPERVISION) {
    taskWdtEnable(taskTimeout);
#ifdef SUPERVISION_CALLBACK
    wdtEnableInterrupt();
#endif
  }
  interrupts();
  while (true) {
    while (true) {
      noInterrupts();
      if (first != NULL && first->scheduledUptimeMillis <= getMillis()) {
        current = first;
        first = current->next;
      }
      interrupts();

      if (current != NULL) {
        taskWdtReset();
        current->execute();
        taskWdtReset();
#ifdef SLEEP_DELAY
        // use millis() instead of getMillis() because getMillis() may be manipulated by our WTD interrupt.
        lastTaskFinishedMillis = millis();
#endif
        delete current;
        noInterrupts();
        current = NULL;
        interrupts();
      } else {
        break;
      }
    }

    sleepIfRequired();

    if (!isWakeupByOtherInterrupt()) {
      // woken up due to WDT interrupt in case of AVR
      // always executed for esp
      noInterrupts();
      const TaskTimeout taskTimeoutLocal = taskTimeout;
      interrupts();
      if (taskTimeoutLocal != NO_SUPERVISION) {
        // change back to taskTimeout
        taskWdtReset();
        taskWdtEnable(taskTimeoutLocal);
#ifdef SUPERVISION_CALLBACK
        wdtEnableInterrupt();
#endif
      } else {
        // tasks are not supervised, deactivate WDT
        taskWdtDisable();
      }
    } // else the wd is still running in case of AVR
  }
  // never executed so no need to deactivate the WDT
}

#endif // #ifndef LIBCALL_DEEP_SLEEP_SCHEDULER

#if defined(ESP32) || defined(ESP8266)
#include "DeepSleepScheduler_esp.h"
#else
#include "DeepSleepScheduler_avr.h"
#endif

#endif // #ifndef DEEP_SLEEP_SCHEDULER_H
