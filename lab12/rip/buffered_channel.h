#ifndef RIP_BUFFERED_CHANNEL_H
#define RIP_BUFFERED_CHANNEL_H

#include <utility>
#include <optional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <cassert>

template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : size(size) {}

    void Send(const T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        if (closed) {
            return;
            //throw std::runtime_error("Sending to closed channel not allowed");
        } else {
            if (queue.size() == size) {
                ++waiting_senders;
                cv_senders.wait(lock, [this]() { return ((queue.size() < size)); });
                --waiting_senders;
            }
            queue.push(value);
            if (waiting_readers > 0) {
                cv_readers.notify_one();
            } else if ((waiting_senders > 0) && (closed || (queue.size() < size))) {
                cv_senders.notify_one();
            }
        }
    }

    std::optional<T> Recv() {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.empty() && (!closed || waiting_senders > 0)) {
            ++waiting_readers;
            cv_readers.wait(lock, [this]() { return (!queue.empty() || (closed && waiting_senders == 0)); });
            --waiting_readers;
        }
        std::optional<T> recv = std::nullopt;
        if (!queue.empty()) {
            recv = std::optional<T>(queue.front());
            queue.pop();
        }
        if (waiting_senders > 0) {
            cv_senders.notify_one();
        } else if((waiting_readers > 0) && (closed || !queue.empty())) {
            cv_readers.notify_one();
        }
        return recv;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        if ((waiting_readers > 0) && (!queue.empty() || waiting_senders == 0)) {
            cv_readers.notify_one();
        } else if (waiting_senders > 0) {
            cv_senders.notify_one();
        }
    }

private:
    bool closed = false;
    std::mutex mutex;
    int size;
    int waiting_senders = 0;
    int waiting_readers = 0;
    std::condition_variable cv_senders;
    std::condition_variable cv_readers;
    std::queue<T> queue;
};


#endif //RIP_BUFFERED_CHANNEL_H
