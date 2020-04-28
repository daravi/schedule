#pragma once

#include <chrono>
#include <memory>

#include <gtest/gtest.h>

namespace sched
{
	class Scheduler;
}

namespace sched::tests
{
class SchedulerFixture : public ::testing::Test
{
public:
	SchedulerFixture() = default;
	virtual ~SchedulerFixture() = default;

	virtual void SetUp() override;
	virtual void TearDown() override;

protected:
	std::unique_ptr<Scheduler> m_scheduler{ nullptr };
	std::chrono::steady_clock::time_point m_executionTime;
};
}
