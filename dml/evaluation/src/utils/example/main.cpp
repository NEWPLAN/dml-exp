#include "../blockingQueue.h"
#include "../readerwriterqueue.h"
#include <thread>
#include <chrono>
#include "../ATimer.h"
#include "../SPSCQueue.h"
#include "../logging.h"

#define LOG_PER_SECOND 100000000

std::mutex out_mtx;

typedef uint32_t ElemType;

int main(int argc, char **argv)
{
    BlockingQueue<ElemType> *bq = new BlockingQueue<ElemType>();
    moodycamel::BlockingReaderWriterQueue<ElemType> *rwq = new moodycamel::BlockingReaderWriterQueue<ElemType>(1024);
    NEWPLAN::SPSCQueue<ElemType> *spsc_q = new NEWPLAN::SPSCQueue<ElemType>(1024);

    LOG(INFO) << "Logging on INFO";
    LOG(WARNING) << "Warning on LOG";
    //LOG(FATAL) << "Error on exit";
    //LOG(INFO) << "Logging on INFO again";

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
                {
                    std::lock_guard<std::mutex> lg(out_mtx);
                    LOG(INFO) << "[BQ]: Write Performances: " << record * 1.0 / t1.seconds() / 10E6 << " M wps";
                }
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
                {
                    std::lock_guard<std::mutex> lg(out_mtx);
                    LOG(INFO) << "[BQ]: Read Performances: " << record * 1.0 / t1.seconds() / 10E6 << " M rps";
                }
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
                {
                    std::lock_guard<std::mutex> lg(out_mtx);
                    LOG(INFO) << "[WQ]: Write Performances: " << record * 1.0 / t1.seconds() / 10E6
                              << " M wps";
                }
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
                {
                    std::lock_guard<std::mutex> lg(out_mtx);
                    LOG(INFO) << "[WQ]: Read Performances: " << record * 1.0 / t1.seconds() / 10E6
                              << " M rps";
                }
                t1.start();
                record = 0;
            }
        }
    });

    //spsc_q
    std::thread *spq_t1 = new std::thread([spsc_q]() {
        Timer t1;
        t1.start();
        size_t record = 0;
        ElemType data = 0;
        while (true)
        {
            spsc_q->push(data);
            data++;
            record++;
            data %= LOG_PER_SECOND;
            if (record > LOG_PER_SECOND)
            {
                t1.stop();
                {
                    std::lock_guard<std::mutex> lg(out_mtx);
                    LOG(INFO) << "[SPSQ]: Write Performances: " << record * 1.0 / t1.seconds() / 10E6
                              << " M wps";
                }
                t1.start();
                record = 0;
            }
        }
    });
    std::thread *spq_t2 = new std::thread([spsc_q]() {
        Timer t1;
        t1.start();
        size_t record = 0;
        ElemType data = 0;
        ElemType *recv_ptr = nullptr;
        ElemType new_data = 0;
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        while (true)
        {
            do
            {
                recv_ptr = spsc_q->front();
            } while (recv_ptr == nullptr);

            // new_data = spsc_q->blocking_pop();
            // {
            //     if (new_data != data)
            //     {
            //         std::cout << "Error of spsc queue" << std::endl;
            //         exit(-1);
            //     }
            // }

            if (*recv_ptr != data)
            {
                std::cout << "Error of spsc queue" << std::endl;
                exit(-1);
            }
            data = (data + 1) % LOG_PER_SECOND;

            spsc_q->pop();
            record++;
            // LOG_EVERY_N(INFO, 100000) << "LOG EVERY N Records: " << record;
            // LOG_IF(INFO, record > 100000 && record < 100009) << "LOG IF Records: " << record;
            if (record > LOG_PER_SECOND)
            {
                t1.stop();
                {
                    std::lock_guard<std::mutex> lg(out_mtx);

                    LOG(INFO) << "[SPSQ]: Read Performances: " << record * 1.0 / t1.seconds() / 10E6
                              << " M rps";
                }
                t1.start();
                record = 0;
            }
        }
    });
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
