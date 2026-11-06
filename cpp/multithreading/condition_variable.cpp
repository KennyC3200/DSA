#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <string>

std::condition_variable cv;
std::mutex mutex;

int i = 0;

void Wait(const std::string& str) {
    std::unique_lock<std::mutex> lock(mutex);
    std::cout << str << " | Waiting...\n";
    cv.wait(lock, [] { return i == 1; });
    std::cout << str << " | Finished\n";
}

void Signal(const std::string& str) {
    // Notify the condition variables
    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << str << " | Notifying...\n";
    }
    cv.notify_all();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    {
        std::lock_guard<std::mutex> lock(mutex);
        i = 1;
        std::cout << str << " | Notifying again...\n";
    }
    cv.notify_all();
}

int main() {
    std::thread t1(Wait, "t1"), t2(Wait, "t2"), t3(Wait, "t3"), t4(Signal, "t4");
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}