#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <vector>
#include "atomic_queue/atomic_queue.h"

using namespace atomic_queue;

template <typename T, unsigned SIZE>
class AtomicQueueWithCallback {
public:
    AtomicQueueWithCallback() : queue(), callback_(nullptr), data_available(false) {}

    // 注册回调函数
    void register_callback(std::function<void(T, int)> callback) {
        callback_ = callback;
    }

    // 推送数据到队列
    void push(const T& item) {
        queue.push(item);
        // 设置数据可用标志
        data_available.store(true, std::memory_order_release);
    }

    // 消费者线程
    void pop(int consumer_id) {
        while (true) {
            T value;
            // 尝试从队列中弹出数据
            while (queue.try_pop(value)) {
                if (callback_) {
                    callback_(value, consumer_id);  // 传递消费者编号
                }
            }

            // 如果队列为空，并且数据标志被设置，表示可以继续尝试获取数据
            if (data_available.load(std::memory_order_acquire)) {
                std::this_thread::yield();  // 等待，继续尝试
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 稍微休眠以减少忙等
            }
        }
    }

private:
    AtomicQueue<T, SIZE> queue;          // 队列
    std::function<void(T, int)> callback_; // 回调函数（增加了消费者ID）
    std::atomic<bool> data_available;    // 原子标志，表示队列是否有数据
};

int main() {
    AtomicQueueWithCallback<int, 1024> queue;

    // 注册一个回调函数，队列中有数据时调用，增加消费者ID
    queue.register_callback([](int value, int consumer_id) {
        std::cout << "Consumer " << consumer_id << " received: " << value << std::endl;
    });

    // 启动多个消费者线程
    const int num_consumers = 3;
    std::vector<std::thread> consumers;
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, i]() {
            queue.pop(i);  // 将消费者编号传递给消费者线程
        });
    }

    // 启动生产者线程
    std::thread producer([&queue]() {
        for (int i = 1; i <= 10; ++i) {
            queue.push(i);
            std::cout << "Put: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟生产过程
        }
    });

    producer.join();
    for (auto& consumer : consumers) {
        consumer.join();  // 等待所有消费者线程结束
    }

    return 0;
}
