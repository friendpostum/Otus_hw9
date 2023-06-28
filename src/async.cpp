#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <thread>
#include <condition_variable>


struct block_t {
    std::string t_stamp;
    std::string cmd;
};

struct conn_t {
    size_t qty_cmd{}, cnt_cmd{}, cnt_brace{};
    block_t block_cmd;
    bool empty(){
        return block_cmd.t_stamp.empty();
    }
};

using t_id = size_t;

struct Bulk {
    ~Bulk() {
        if (conn_pool.empty()) {
            while (!done);
            stop = true;
            cv_log.notify_one();
            cv_file.notify_all();
        }

        log.join();
        file1.join();
        file2.join();
    }

    void connect(const t_id &id) {
            conn_pool.insert({id, {id, {}}});
    }

    void receive(const char *buff, size_t buff_size, const t_id &id) {
        std::string line(buff, buff_size);
        if (input(std::move(line), id)) {
            done = false;
            {
                std::unique_lock<std::mutex> lk(m_log);
                tasks_log.push_back(std::move(conn_pool[id].block_cmd));
            }
            cv_log.notify_one();
        }
    }

    void disconnect(const t_id &id) {
            conn_pool.extract(id);
    }

private:
    bool input(std::string&& line, const t_id &id) {
        if (line == "{")
            return conn_pool[id].cnt_brace++ == 0 && (conn_pool[id].cnt_cmd = 0, !conn_pool[id].empty());

        if (line == "}")
            return --conn_pool[id].cnt_brace == 0;

        if (conn_pool[id].cnt_cmd++ == 0) {
            conn_pool[id].block_cmd = block_new(line);
        } else conn_pool[id].block_cmd.cmd += ", " + line;

        if (conn_pool[id].cnt_brace == 0) {
            return conn_pool[id].cnt_cmd == conn_pool[id].qty_cmd && (conn_pool[id].cnt_cmd = 0, true);
        }
        return false;
    }

    block_t block_new(const std::string &line) {
        auto t = std::chrono::system_clock::now().time_since_epoch();
        auto utc = std::chrono::duration_cast<std::chrono::microseconds>(t).count();
        return {std::to_string(utc)+ "_" , "bulk: " + line};
    }

    void to_log_q(size_t id) {
        while (!stop) {
            std::unique_lock<std::mutex> lk(m_log);
            cv_log.wait(lk, [this]() { return !tasks_log.empty() || stop; });

            if (!stop) {
                while (!tasks_log.empty()) {
                    auto block = tasks_log.front();
                    std::cout << block.cmd << '\n';
                    {
                        std::unique_lock<std::mutex> lk(m_file);
                        tasks_file.push(std::move(block));
                    }
                    tasks_log.pop_front();
                }
                lk.unlock();
                cv_file.notify_all();
            }
        }
    }

    void to_file_q(size_t id) {
        while (!stop) {
            std::unique_lock<std::mutex> lk(m_file);
            cv_file.wait(lk, [this]() { return !tasks_file.empty() || stop; });

            if (!tasks_file.empty()){
                auto b = tasks_file.front();
                tasks_file.pop();
                lk.unlock();
                done = tasks_log.empty() && tasks_file.empty();
                std::ofstream file(b.t_stamp + std::to_string(id) + ".log");
                file << b.cmd;
                file.close();
            }
        }
    }

    std::thread log{&Bulk::to_log_q, this, 1};
    std::thread file1{&Bulk::to_file_q, this, 2};
    std::thread file2{&Bulk::to_file_q, this, 3};

    bool stop{false};
    bool done{true};
    std::map<t_id, conn_t> conn_pool;
    std::deque<block_t> tasks_log;
    std::queue<block_t> tasks_file;
    std::mutex m_log;
    std::mutex m_file;
    std::condition_variable cv_log;
    std::condition_variable cv_file;
};

Bulk bulk;

t_id connect(size_t N) {
    bulk.connect(N);
    return N;
}

void receive(const char *buff, size_t buff_size, const t_id &id) {
    bulk.receive(buff, buff_size, id);
}

void disconnect(const t_id &id) {
    bulk.disconnect(id);
}

