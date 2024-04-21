#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

int f(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (x == 10) return 0;
    if (x <= 0) while (true) {}

    return x;
}

int g(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    if(x == 9) return 0;
    if (x <= 0) return 0;

    return x * 2;
}

void wait_for_threads(int& result_f, int& result_g) {
    std::unique_lock<std::mutex> lock(mtx);
    if (!cv.wait_for(lock, std::chrono::seconds(10), [&]() { return ready; })) {
        std::cout << "Timeout occurred. Proceeding without results from threads." << std::endl;
    } else {
        std::cout << "Results received from threads." << std::endl;
    }
}


void thread_f(int x, int& result_f) {
    result_f = f(x);
    {
        std::lock_guard<std::mutex> lock(mtx);
        if(result_f == 0) ready = true;
    }
    cv.notify_one();
}

void thread_g(int x, int& result_g) {
    result_g = g(x);
    {
        std::lock_guard<std::mutex> lock(mtx);
        if(result_g == 0) ready = true;
    }
    cv.notify_one();
}

int main() {
    int x = 1;

    int result_f, result_g;

    std::thread t1(thread_f, x, std::ref(result_f));
    std::thread t2(thread_g, x, std::ref(result_g));

    wait_for_threads(result_f, result_g);

    bool final_result = result_f && result_g;
    std::cout << "Final result of f(x) && g(x): " << final_result << std::endl;
    std::cout << "Final result for f(x): " << result_f << std::endl;
    std::cout << "Final result for g(x): " << result_g << std::endl;

    t1.join();
    t2.join();

    return 0;
}
