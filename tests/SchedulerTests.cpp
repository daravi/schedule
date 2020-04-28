#include "SchedulerTests.h"

#include <chrono>
#include <thread>
#include <unordered_map>
#include <vector>

#include <glog/logging.h>

#include "Scheduler.h"

using namespace std::chrono_literals;

namespace sched::tests
{
void SchedulerFixture::SetUp()
{
	m_scheduler = std::make_unique<Scheduler>();
}

void SchedulerFixture::TearDown()
{
	m_scheduler = nullptr;		
}

TEST_F(SchedulerFixture, Construction)
{
	Scheduler scheduler{};
}

TEST_F(SchedulerFixture, RegisterCall)
{
	int expectedArgument{ 42 };
	int actualArgument{ 0 };
	auto expectedExecutionTime = std::chrono::steady_clock::now() + 153ms;
	std::chrono::steady_clock::time_point actualExecutionTime;
	bool called{ false };
	
	auto testCall = [&](int argument){
		called = true;
		actualExecutionTime = std::chrono::steady_clock::now();
		actualArgument = argument;
		std::this_thread::sleep_for(25ms);
	};
	
	m_scheduler->registerCall(expectedExecutionTime, testCall, expectedArgument);
	
	std::this_thread::sleep_until(expectedExecutionTime + 5ms);
	
	ASSERT_TRUE(called);
	EXPECT_EQ(actualArgument, expectedArgument);
	auto executionTimeError = actualExecutionTime > expectedExecutionTime ? actualExecutionTime - expectedExecutionTime : expectedExecutionTime - actualExecutionTime;
	EXPECT_NEAR(executionTimeError / 1ms, 0, 10);
}

TEST_F(SchedulerFixture, RegisterMultipleCalls)
{
	int callCount{ 10 };
	std::vector<int> expectedArguments;
	std::unordered_map<int, int> actualArguments;
	std::vector<std::chrono::steady_clock::time_point> expectedTimes;
	std::unordered_map<int, std::chrono::steady_clock::time_point> actualTimes;
	
	auto testCall = [&](int callIndex, int argument){
		actualTimes[callIndex] = std::chrono::steady_clock::now();
		actualArguments[callIndex] = argument;
		std::this_thread::sleep_for(25ms);
	};
	
	int argument{ 42 };
	auto callTime{std::chrono::steady_clock::now() + 50ms};
	for (size_t i = 0; i < callCount; i++)
	{
		expectedTimes.emplace_back(callTime);
		expectedArguments.emplace_back(argument);
		m_scheduler->registerCall(callTime, testCall, i, argument);
		argument += 11;
		callTime += 23ms;
	}
	
	std::this_thread::sleep_until(expectedTimes.back() + 5ms);
	
	for (size_t i = 0; i < callCount; i++)
	{
		EXPECT_EQ(expectedArguments[i], actualArguments[i]);
		
		auto callTimeError = actualTimes[i] > expectedTimes[i] ? actualTimes[i] - expectedTimes[i] : expectedTimes[i] - actualTimes[i];
		EXPECT_NEAR(callTimeError / 1ms, 0, 10);
	}
}

TEST_F(SchedulerFixture, RegisterRepeatedCall_StartNow)
{
	int expectedCallCount{ 5 };
	int expectedArgument{ 42 };
	std::chrono::duration interval{ 42ms };
	
	std::unordered_map<int, int> actualArguments;
	std::vector<std::chrono::steady_clock::time_point> expectedTimes;
	std::unordered_map<int, std::chrono::steady_clock::time_point> actualTimes;
	
	auto expectedStartTime = std::chrono::steady_clock::now() + 20ms;
	for (size_t i = 0; i < expectedCallCount; i++)
	{
		expectedTimes.emplace_back() = expectedStartTime + i * interval;
	}
	
	int callCount{ 0 };
	auto testCall = [&](int argument){
		actualTimes[callCount] = std::chrono::steady_clock::now();
		actualArguments[callCount++] = argument;
		std::this_thread::sleep_for(interval / 2);
	};
	
	std::this_thread::sleep_until(expectedStartTime);
	m_scheduler->registerRepeatedCall(interval, testCall, expectedArgument);
	std::this_thread::sleep_until(expectedTimes.back() + 10ms);
	
	for (size_t i = 0; i < expectedCallCount; i++)
	{
		EXPECT_EQ(actualArguments[i], expectedArgument);
		
		auto callTimeError = actualTimes[i] > expectedTimes[i] ? actualTimes[i] - expectedTimes[i] : expectedTimes[i] - actualTimes[i];
		EXPECT_NEAR(callTimeError / 1ms, 0, 10);
	}
}

TEST_F(SchedulerFixture, RegisterRepeatedCall_StartLater)
{
	int expectedCallCount{ 5 };
	int expectedArgument{ 42 };
	std::chrono::duration interval{ 42ms };
	
	std::unordered_map<int, int> actualArguments;
	std::vector<std::chrono::steady_clock::time_point> expectedTimes;
	std::unordered_map<int, std::chrono::steady_clock::time_point> actualTimes;
	
	auto expectedStartTime = std::chrono::steady_clock::now() + 20ms;
	for (size_t i = 0; i < expectedCallCount; i++)
	{
		expectedTimes.emplace_back() = expectedStartTime + i * interval;
	}
	
	int callCount{ 0 };
	auto testCall = [&](int argument){
		actualTimes[callCount] = std::chrono::steady_clock::now();
		actualArguments[callCount++] = argument;
		std::this_thread::sleep_for(interval / 2);
	};
	
	m_scheduler->registerRepeatedCall(expectedStartTime, interval, testCall, expectedArgument);
	std::this_thread::sleep_until(expectedTimes.back() + 10ms);
	
	for (size_t i = 0; i < expectedCallCount; i++)
	{
		EXPECT_EQ(actualArguments[i], expectedArgument);
		
		auto callTimeError = actualTimes[i] > expectedTimes[i] ? actualTimes[i] - expectedTimes[i] : expectedTimes[i] - actualTimes[i];
		EXPECT_NEAR(callTimeError / 1ms, 0, 10);
	}
}
}