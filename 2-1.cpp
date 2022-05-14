#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <vector>

// 定义互斥锁(条件变量需要和互斥锁一起使用)
std::mutex mtx;
// 定义条件变量(用来做线程间的同步通信)
std::condition_variable cv;
// 定义vector容器，作为生产者和消费者共享的容器
std::vector<int> vec;

// 生产者线程函数
void producer()
{
    // 生产者每生产一个，就通知消费者消费一个
    for (int i = 1; i <= 10; ++i)
    {
        // 获取mtx互斥锁资源
        std::unique_lock<std::mutex> lock(mtx);

        // 如果容器不为空，代表还有产品未消费，等待消费者线程消费完，再生产
        while (!vec.empty())
        {
            // 判断容器不为空，进入等待条件变量的状态，释放mtx锁，
            // 让消费者线程抢到锁能够去消费产品
            cv.wait(lock);
        }
        vec.push_back(i); // 表示生产者生产的产品序号i
        std::cout << "producer生产产品:" << i << std::endl;

        /*
        生产者线程生产完产品，通知等待在cv条件变量上的消费者线程，
        可以开始消费产品了，然后释放锁mtx
        */
        cv.notify_all();

        // 生产一个产品，睡眠100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
// 消费者线程函数
void consumer()
{
    // 消费者每消费一个，就通知生产者生产一个
    for (int i = 1; i <= 10; ++i)
    {
        // 获取mtx互斥锁资源
        std::unique_lock<std::mutex> lock(mtx);

        // 如果容器为空，代表还有没有产品可消费，等待生产者生产，再消费
        while (vec.empty())
        {
            // 判断容器为空，进入等待条件变量的状态，释放mtx锁，
            // 让生产者线程抢到锁能够去生产产品
            cv.wait(lock);
        }
        int data = vec.back(); // 表示消费者消费的产品序号i
        vec.pop_back();
        std::cout << "consumer消费产品:" << data << std::endl;

        /*
        消费者消费完产品，通知等待在cv条件变量上的生产者线程，
        可以开始生产产品了，然后释放锁mtx
        */
        cv.notify_all();

        // 消费一个产品，睡眠100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
int main()
{
    // 创建生产者和消费者线程
    std::thread t1(producer);
    std::thread t2(consumer);

    // main主线程等待所有子线程执行完
    t1.join();
    t2.join();

    return 0;
}