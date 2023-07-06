#include "async.cpp"

void testSingleClientThread() {
    auto id3 = connect(3);

    receive("cmd1", 4, id3);
    receive("cmd2", 4, id3);
    receive("{", 1, id3);
    receive("cmd3", 4, id3);
    receive("cmd4", 4, id3);
    receive("}", 1, id3);
    receive("{", 1, id3);
    receive("cmd5", 4, id3);
    receive("cmd6", 4, id3);
    receive("{", 1, id3);
    receive("cmd7", 4, id3);
    receive("cmd8", 4, id3);
    receive("}", 1, id3);
    receive("cmd9", 4, id3);
    receive("}", 1, id3);
    receive("{", 1, id3);
    receive("cmd10", 4, id3);
    receive("cmd11", 4, id3);

    auto id4 = connect(4);
    receive("cmd1", 4, id4);
    receive("cmd2", 4, id4);
    receive("{", 1, id4);
    receive("cmd3", 4, id4);
    receive("cmd4", 4, id4);
    receive("}", 1, id4);
    receive("{", 1, id4);
    receive("cmd5", 4, id4);
    receive("cmd6", 4, id4);
    receive("{", 1, id4);
    receive("cmd7", 4, id4);
    receive("cmd8", 4, id4);
    receive("}", 1, id4);
    receive("cmd9", 4, id4);
    receive("}", 1, id4);
    receive("{", 1, id4);
    receive("cmd10", 4, id4);
    receive("cmd11", 4, id4);

    disconnect(id3);
    disconnect(id4);
}

void testMultipleClientThreads() {
    const std::size_t numberOfCommandsPerThread = 10;

    auto worker = [numberOfCommandsPerThread](std::size_t threadNumber, std::size_t blockSize) {
        auto handle = connect(blockSize);
        for (std::size_t i = 0; i < numberOfCommandsPerThread; ++i) {
            std::string command{"cmd"};
            command += std::to_string(i + 1);
            receive(command.data(), command.size(), handle);
        }
        disconnect(handle);
    };

    std::thread t1(worker, 0, 1);
    std::thread t2(worker, 1, 3);
    std::thread t3(worker, 2, 5);

    t1.join();
    t2.join();
    t3.join();
}


int main() {
    std::cout << "===\n=== Send commands from a single thread ===\n===\n" << std::endl;
    testSingleClientThread();

    std::cout << "\n===\n=== Send commands from multiple threads ===\n===\n" << std::endl;
    testMultipleClientThreads();

    return 0;
}
