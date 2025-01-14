#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "OSLikeStuff/task_scheduler.cpp"
#include "cstdint"
#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace {

void sleep_50ns() {
	mock().actualCall("sleep_50ns");
	uint32_t now = getTimerValue(0);
	while (getTimerValue(0) < now + 0.00005 * DELUGE_CLOCKS_PER) {
		;
	}
}
void sleep_10ns() {
	mock().actualCall("sleep_10ns");
	uint32_t now = getTimerValue(0);

	while (getTimerValue(0) < now + 0.00001 * DELUGE_CLOCKS_PER) {
		;
	}
}
void sleep_2ms() {
	mock().actualCall("sleep_2ms");
	uint32_t now = getTimerValue(0);
	while (getTimerValue(0) < now + 0.002 * DELUGE_CLOCKS_PER) {
		;
	}
}

TEST_GROUP(Scheduler) {
	TaskManager testTaskManager;

	void setup() {
		testTaskManager = TaskManager();
	}
};

TEST(Scheduler, schedule) {
	mock().clear();
	// will be called one less time due to the time the sleep takes not being zero
	mock().expectNCalls(0.01 / 0.001 - 1, "sleep_50ns");
	testTaskManager.addRepeatingTask(sleep_50ns, 0, 0.001, 0.001, 0.001);
	// run the scheduler for just under 10ms, calling the function to sleep 50ns every 1ms
	testTaskManager.start(0.0095);
	std::cout << "ending tests at " << getTimerValueSeconds(0) << std::endl;
	mock().checkExpectations();
};

TEST(Scheduler, scheduleMultiple) {
	mock().clear();
	mock().expectNCalls(0.01 / 0.001 - 1, "sleep_50ns");
	mock().expectNCalls(0.01 / 0.001 - 1, "sleep_10ns");

	// every 1ms sleep for 50ns and 10ns
	testTaskManager.addRepeatingTask(sleep_50ns, 10, 0.001, 0.001, 0.001);
	testTaskManager.addRepeatingTask(sleep_10ns, 0, 0.001, 0.001, 0.001);
	// run the scheduler for 10ms
	testTaskManager.start(0.0095);
	std::cout << "ending tests at " << getTimerValueSeconds(0) << std::endl;
	mock().checkExpectations();
};
TEST(Scheduler, overSchedule) {
	mock().clear();

	// will take one call to get duration, second call at its maximum time between calls
	mock().expectNCalls(2, "sleep_2ms");
	// will be missing 4ms from the two sleeps
	mock().expectNCalls(0.006 / 0.001, "sleep_50ns");
	mock().expectNCalls(0.006 / 0.001, "sleep_10ns");

	// every 1ms sleep for 50ns and 10ns
	auto fiftynshandle = testTaskManager.addRepeatingTask(sleep_50ns, 10, 0.001, 0.001, 0.001);
	auto tennshandle = testTaskManager.addRepeatingTask(sleep_10ns, 0, 0.001, 0.001, 0.001);
	auto twomsHandle = testTaskManager.addRepeatingTask(sleep_2ms, 100, 0.001, 0.002, 0.005);
	// run the scheduler for 10ms
	testTaskManager.start(0.0098);
	std::cout << "ending tests at " << getTimerValueSeconds(0) << std::endl;

	mock().checkExpectations();
};

} // namespace
