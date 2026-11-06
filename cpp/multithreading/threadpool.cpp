#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <cstddef>
#include <string>
#include <iostream>

// Mutex to ensure that prints don't happyen simultaneously
std::mutex print_mutex;

class Threadpool {
public:
    explicit Threadpool(std::size_t n) {
        for (std::size_t i = 0; i < n; i++) {
            m_workers.emplace_back([this] { WorkerLoop(); });
        }
    }

    ~Threadpool() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopping = true;
        }

        // Notify the workers via the condition variable
        // Let them know that they're gonna DIE
        m_cv.notify_all();
        for (auto& worker : m_workers) {
            worker.join();
        }
    }

    template <class F>
    auto Submit(F f) -> std::future<decltype(f())> {
        // Return type
        using R = decltype(f());

        // A task is basically a mailbox: the worker puts the result in it when it's done
        // A packaged_task is f bundled together with that mailbox. When a worker calls it, it:
        //  1. Calls f()
        //  2. Puts the return value in the mailbox, instead of returning it
        // To retrieve the value, call task->get_future()
        // Note: packaged_task is move only, but the queue stores a std::function which is copied
        //       shared_ptr solves this by holding a pointer to the task
        //       m_tasks is pushing back a lambda that holds a copy of the pointer to the task
        auto task = std::make_shared<std::packaged_task<R()>>(std::move(f));
        std::future<R> result = task->get_future();

        // Lock the mutex for pushing the tasks to the queue
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.push([task] { (*task)(); });
        }

        m_cv.notify_one(); // Wake one idle worker to complete the task
        return result;
    }

private:
    void WorkerLoop() {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(m_mutex);

                m_cv.wait(lock, [this] { return m_stopping || !m_tasks.empty(); });

                if (m_stopping && m_tasks.empty()) return;

                job = std::move(m_tasks.front());
                m_tasks.pop();
            }

            // Run the job outside of the task so that other workers can grab the tasks
            job();
        }
    }

    // Protect the queue
    std::mutex m_mutex;

    // Allow idle worker threads to sleep without consuming CPU cycles until new
    // tasks are added to the workers queue
    std::condition_variable m_cv;

    // Hold pending tasks
    std::queue<std::function<void()>> m_tasks;

    // Workers need to be destructed first
    std::vector<std::thread> m_workers;

    bool m_stopping = false;
};

struct Image {
    int width, height;
    std::string path;
};

std::vector<std::string> img_paths = {
    "images/img1.png",
    "images/img2.png",
    "images/img3.png",
    "images/img4.png",
    "images/img5.png",
};

Image LoadAndResizeImage(const std::string& path, int width, int height) {
{
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << path << " | Loading...\n";
}

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    Image img = { width, height, path };
    return img;
}

int main() {
    Threadpool pool(2);

    std::vector<std::future<Image>> pending;
    for (const auto& path : img_paths) {
        pending.push_back(pool.Submit([path] {
            return LoadAndResizeImage(path, 100, 100);
        }));
    }

    for (auto& f : pending) {
        Image img = f.get();

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            // This is blocked until the img is ready, via f.get()
            std::cout << img.path << " | " << img.width << " by " << img.height << "\n";
        }
    }
}