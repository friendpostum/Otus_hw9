#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <thread>
#include <condition_variable>

#include "queue.h"
#include <boost/lockfree/queue.hpp>

struct block_t {
    std::string t_stamp;
    std::string cmd;
};

struct block_t1 {
    block_t1() = default;

    block_t1(const block_t1 &rhs) = default;

    block_t1 &operator=(const block_t1 &rhs) = default;

    ~block_t1() = default;

    int t_stamp;
    // char cmd[];
};

struct conn_t {
    size_t qty_cmd{}, cnt_cmd{}, cnt_brace{};
    block_t block_cmd;

    bool empty() {
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
            q_log.push(std::move(conn_pool[id].block_cmd));
            cv_log.notify_one();
        }
    }

    void disconnect(const t_id &id) {
        conn_pool.extract(id);
    }

private:
    bool input(std::string &&line, const t_id &id) {
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
        return {std::to_string(utc) + "_", "bulk: " + line};
    }

    void to_log_q() {
        block_t block;
        while (!stop) {
            std::unique_lock<std::mutex> lk(m_log);
            cv_log.wait(lk, [this, &block]() { return q_log.try_pop(block) || stop; });

            if (!stop) {
                std::cout << block.cmd << '\n';
                lk.unlock();
                q_file.push(block);
            }

            cv_file.notify_one();
        }
    }

    void to_file_q(size_t id) {
        block_t block;
        while (!stop) {
            std::unique_lock<std::mutex> lk(m_file);
            cv_file.wait(lk, [this, &block]() { return q_file.try_pop(block) || stop; });

            // std::cout << "file " << block.t_stamp + std::to_string(id) << "\n";

            if (!stop) {
                std::ofstream file(block.t_stamp + std::to_string(id) + ".log");
                file << block.cmd;
                file.close();
                lk.unlock();
                done = q_log.empty() && q_file.empty();
            }
        }
    }

    std::thread log{&Bulk::to_log_q, this};
    std::thread file1{&Bulk::to_file_q, this, 2};
    std::thread file2{&Bulk::to_file_q, this, 3};

    bool stop{false};
    bool done{true};
    std::map<t_id, conn_t> conn_pool;
    Queue<block_t> q_log;
    Queue<block_t> q_file;
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

