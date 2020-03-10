#include "../blockingQueue.h"
#include "../readerwriterqueue.h"
#include <thread>
#include <chrono>
#include "../ATimer.h"

#define LOG_PER_SECOND 10000000

int main(int argc, char **argv)
{
    BlockingQueue<int> *bq = new BlockingQueue<int>();
    moodycamel::BlockingReaderWriterQueue<int> *rwq = new moodycamel::BlockingReaderWriterQueue<int>(100);

    std::thread *bq_t1 = new std::thread([bq]() {
        Timer t1;
        t1.start();
        size_t record = 0;
        while (true)
        {
            bq->push(1);
            record++;
            if (record > LOG_PER_SECOND)
            {
                t1.stop();
                std::cout << "[BQ]: Write Performances: " << record * 1.0 / t1.seconds() << " wps" << std::endl;
                t1.start();
                record = 0;
            }
        }
    });
    std::thread *bq_t2 = new std::thread([bq]() {
        int data = 0;
        Timer t1;
        t1.start();
        size_t record = 0;
        while (true)
        {
            data = bq->pop();
            record++;
            if (record > LOG_PER_SECOND)
            {
                t1.stop();
                std::cout << "[BQ]: Read Performances: " << record * 1.0 / t1.seconds() << " wps" << std::endl;
                t1.start();
                record = 0;
            }
        }
    });
    //wait_dequeue
    std::thread *rw_t1 = new std::thread([rwq]() {
        Timer t1;
        t1.start();
        size_t record = 0;
        while (true)
        {
            rwq->enqueue(1);
            record++;
            if (record > LOG_PER_SECOND)
            {
                t1.stop();
                std::cout << "[WQ]: Write Performances: " << record * 1.0 / t1.seconds() << " wps" << std::endl;
                t1.start();
                record = 0;
            }
        }
    });
    std::thread *rw_t2 = new std::thread([rwq]() {
        int data = 0;
        Timer t1;
        t1.start();
        size_t record = 0;
        while (true)
        {
            rwq->wait_dequeue(data);
            record++;
            if (record > LOG_PER_SECOND)
            {
                t1.stop();
                std::cout << "[WQ]: Read Performances: " << record * 1.0 / t1.seconds() << " wps" << std::endl;
                t1.start();
                record = 0;
            }
        }
    });
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}