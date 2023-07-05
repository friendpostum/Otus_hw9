#include <condition_variable>
#include <queue>
#include <memory>

template<typename T>
struct safe_queue {
    safe_queue() = default;

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        ext_ctrl = false;
        cv.notify_one();
    }

    void ret_ctrl(){
        ext_ctrl = true;
        cv.notify_one();
    }

    bool wait_and_pop(T &value) {
        std::unique_lock<std::mutex> lk(mut);
        cv.wait(lk, [this] { return !data_queue.empty() || ext_ctrl; });
        if (ext_ctrl) {
            return false;
        } else {
            value = std::move(data_queue.front());
            data_queue.pop();
            return true;
        }
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        cv.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front() )));
        data_queue.pop();
        return res;
    }

    bool try_pop(T &value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;

        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();

        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

private:
    mutable std::mutex mut;
    bool ext_ctrl = false;
    std::queue<T> data_queue;
    std::condition_variable cv;
};