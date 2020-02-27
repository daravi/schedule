#include <iostream>

#include "Scheduler.h"

using namespace std::chrono_literals;

void foo(int x)
{
    std::cout << "Received x: " << x << std::endl;
}

int main(int argc, char const *argv[])
{
    sched::Scheduler s{};
    s.registerCall(std::chrono::system_clock::now() + 5000ms, &foo, 42);
    s.registerCall(std::chrono::system_clock::now() + 4000ms, &foo, 32);
    s.registerCall(std::chrono::system_clock::now() + 3000ms, &foo, 22);
    s.registerCall(std::chrono::system_clock::now() + 2000ms, &foo, 12);
    s.registerCall(std::chrono::system_clock::now() + 1000ms, &foo, 2);
    std::this_thread::sleep_for(6000ms);
    // s.call();
    return 0;
}
