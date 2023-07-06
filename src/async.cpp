#include <iostream>
#include <fstream>
#include <map>
#include <thread>

#include "ts_queue.h"

struct block_t {
    std::string t_stamp;
    std::string cmd;
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
        while (!finished);
        q_log.wake_and_done();
        q_file.wake_and_done();

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
            finished = false;
            q_log.push(std::move(conn_pool[id].block_cmd));
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
        while (true) {
            if (q_log.wait_and_pop(block)) {
                std::cout << block.cmd << '\n';
                q_file.push(block);
            } else break;
        }
    }

    void to_file_q(size_t id) {
        block_t block;
        while (true) {
            if (q_file.wait_and_pop(block)) {
                std::ofstream file(block.t_stamp + std::to_string(id) + ".log");
                file << block.cmd;
                file.close();
                finished = conn_pool.empty() && q_log.empty() && q_file.empty();
            } else break;
        }
    }

    bool finished{false};
    std::map<t_id, conn_t> conn_pool;
    ts_queue<block_t> q_log{};
    ts_queue<block_t> q_file{};
    std::thread log{&Bulk::to_log_q, this};
    std::thread file1{&Bulk::to_file_q, this, 2};
    std::thread file2{&Bulk::to_file_q, this, 3};
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

