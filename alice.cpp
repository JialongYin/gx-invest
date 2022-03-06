#include "common.h"

/* --------------------------------------不得修改两条分割线之间的内容-------------------------------------- */

/*
示例的测试数据（最终测试时会更换），对于每一组测试数据
第一个数字表示要求发送消息的时间，保证严格递增
第二个数字表示所发送消息的长度，保证出现在MESSAGE_SIZES中
*/
std::deque<std::pair<time_t, int>> generate()
{
    std::deque<std::pair<time_t, int>> c = {
        // 间隔1s发送
        {1 * SECOND_TO_NANO, MESSAGE_SIZES[0]},
        {2 * SECOND_TO_NANO, MESSAGE_SIZES[1]},
        {3 * SECOND_TO_NANO, MESSAGE_SIZES[2]},
        {4 * SECOND_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO, MESSAGE_SIZES[4]},
        // 间隔100mss发送
        {5 * SECOND_TO_NANO + 100 * MILLI_TO_NANO, MESSAGE_SIZES[4]},
        {5 * SECOND_TO_NANO + 200 * MILLI_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO + 300 * MILLI_TO_NANO, MESSAGE_SIZES[2]},
        {5 * SECOND_TO_NANO + 400 * MILLI_TO_NANO, MESSAGE_SIZES[1]},
        {5 * SECOND_TO_NANO + 500 * MILLI_TO_NANO, MESSAGE_SIZES[0]},
        // 间隔10ms发送
        {5 * SECOND_TO_NANO + 510 * MILLI_TO_NANO, MESSAGE_SIZES[0]},
        {5 * SECOND_TO_NANO + 520 * MILLI_TO_NANO, MESSAGE_SIZES[1]},
        {5 * SECOND_TO_NANO + 530 * MILLI_TO_NANO, MESSAGE_SIZES[2]},
        {5 * SECOND_TO_NANO + 540 * MILLI_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO + 550 * MILLI_TO_NANO, MESSAGE_SIZES[4]},
        // 间隔1ms发送
        {5 * SECOND_TO_NANO + 551 * MILLI_TO_NANO, MESSAGE_SIZES[4]},
        {5 * SECOND_TO_NANO + 552 * MILLI_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO + 553 * MILLI_TO_NANO, MESSAGE_SIZES[2]},
        {5 * SECOND_TO_NANO + 554 * MILLI_TO_NANO, MESSAGE_SIZES[1]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO, MESSAGE_SIZES[0]},
        // 间隔100us发送
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 100 * MICRO_TO_NANO, MESSAGE_SIZES[0]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 200 * MICRO_TO_NANO, MESSAGE_SIZES[1]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 300 * MICRO_TO_NANO, MESSAGE_SIZES[2]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 400 * MICRO_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 500 * MICRO_TO_NANO, MESSAGE_SIZES[4]},
        // 间隔10us发送
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 510 * MICRO_TO_NANO, MESSAGE_SIZES[4]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 520 * MICRO_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 530 * MICRO_TO_NANO, MESSAGE_SIZES[2]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 540 * MICRO_TO_NANO, MESSAGE_SIZES[1]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 550 * MICRO_TO_NANO, MESSAGE_SIZES[0]},
        // 间隔1us发送
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 551 * MICRO_TO_NANO, MESSAGE_SIZES[0]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 552 * MICRO_TO_NANO, MESSAGE_SIZES[1]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 553 * MICRO_TO_NANO, MESSAGE_SIZES[2]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 554 * MICRO_TO_NANO, MESSAGE_SIZES[3]},
        {5 * SECOND_TO_NANO + 555 * MILLI_TO_NANO + 555 * MICRO_TO_NANO, MESSAGE_SIZES[4]},
    };
    int init_size = c.size();
    const int repeat = 10; // 重复若干次
    for (int i = 1; i <= repeat; ++i)
        for (int j = 0; j < init_size; ++j)
            c.push_back(std::make_pair(c[j].first + 6 * SECOND_TO_NANO * i, c[j].second));
    // 检查时间递增
    for (int i = 1; i < c.size(); ++i)
        assert(c[i].first > c[i - 1].first);
    return c;
}

auto test_cases = generate();
size_t test_case_count = test_cases.size();
std::vector<time_t> delays;

// 获取下一条现在需要发送的消息并将其从test_cases中移除，当test_cases中没有满足时间要求的消息时返回NULL
const Message *next_message()
{
    // 所有测试用例均完成，打印统计结果并退出
    std::cout << "delay: " << delays.size() << " left: " << test_cases.size() << " target: " << test_case_count << std::endl;
    if (delays.size() == test_case_count)
    {
        std::sort(delays.begin(), delays.end());
        double median = delays[delays.size() / 2];
        double p95 = delays[delays.size() * 95 / 100];
        double p99 = delays[delays.size() * 99 / 100];
        double maximum = delays.back();
        double s = 0;
        for (auto d : delays)
            s += d;
        double mean = s / delays.size();
        double s2 = 0;
        for (auto d : delays)
            s2 += (d - mean) * (d - mean);
        double std = sqrt(s2 / delays.size());
        printf("n=%zu median=%.0fns mean=%.0fns std=%.0fns 95%%=%.0fns 99%%=%.0fns maximum=%.0fns\n", delays.size(), median, mean, std, p95, p99, maximum);
        exit(0);
    }

    // std::cout << "pass here 1" << std::endl;
    // 检查下一条消息是否已经到时间
    if (test_cases.empty())
        return NULL;
    // std::cout << "pass here 2" << std::endl;
    auto c = test_cases.front();

    time_t seconds = now();
    // std::stringstream ss, tt;
    // ss << seconds;
    // tt << c.first;
    // std::string ts = ss.str(), st = tt.str();
    // std::cout << "st: " << st << " ts: " << ts << std::endl;

    if (c.first > seconds)
        return NULL;
    // std::cout << "pass here 3" << std::endl;

    // 在堆上申请Message的临时空间，并在所有next_message的调用中复用
    static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);

    // 构建消息头
    test_cases.pop_front();
    m->t = c.first;
    m->size = c.second;

    // 随机生成消息体
    static unsigned int seed = 1;
    int *p = (int *)(m->payload);
    for (auto i = m->payload_size() / 4; i; --i)
    {
        seed = (seed * 1103515245U + 12345U) & 0x7fffffffU;
        *p++ = (int)seed;
    }

    m->checksum = crc32(m);
    // std::cout << "size: " << m->size << " checksum: " << m->checksum << "payload: " << m->payload << std::endl;
    return m;
}

// 记录延迟
void record(const Message *m)
{
    assert(m->checksum == crc32(m));
    time_t t = now();
    delays.push_back((t - m->t) / 2); // 来回的时间差除2近似地取作单程延迟，注意每轮通信会有四次校验和的计算，这些耗时也必须包含进延迟中
    // printf("%ld %ld %ld\n", m->t, t, (t - m->t) / 2);
    static std::set<time_t> ts;
    assert(ts.find(m->t) == ts.end()); // 每个时间戳对应的消息只应被记录一次
    ts.insert(m->t);
    for (auto c : test_cases)
        assert(c.first != m->t); // 被记录的消息必须是已经被取出的测试数据
}

/* --------------------------------------不得修改两条分割线之间的内容-------------------------------------- */

/*Non-blocking FIFO*/
static ssize_t bytew = 0, byter = 0;
void send(const Message *message)
{
    static int fifo = 0;
    if (fifo == 0)
    {
        const char *filename = "alice_to_bob";
        if (access(filename, F_OK)) { // return 0 if file exists, -1 if no exists
            mkfifo(filename, 0666);
        }
        fifo = open(filename, O_WRONLY | O_NONBLOCK);
        assert(fifo != 0);
    }

    if ((bytew = write(fifo, message, message->size)) == -1) { // return -1 if no reader attached
        // std::cout << "alice send: " << bytew << std::endl;
        return;
    }
    // std::cout << "alice send: " << bytew << std::endl;
    assert(bytew == message->size);
}
const Message *recv()
{
    static int fifo = 0;
    if (fifo == 0)
    {
        const char *filename = "bob_to_alice";
        if (access(filename, F_OK)) {
            mkfifo(filename, 0666);
        }
        fifo = open(filename, O_RDONLY | O_NONBLOCK);
        assert(fifo != 0);
    }

    static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
    if ((byter = read(fifo, m, m->size)) == 0) { // return 0 if no witer attached
        // std::cout << "alice recv: " << byter << std::endl;
        return NULL;
    }
    assert(byter == m->size);
    return m;
}
/*Non-blocking FIFO*/
int main()
{
    const Message *m1 = NULL;
    int i = 10000;
    while (i--)
    {
        // std::cout << "alice before next_message" << std::endl;
        // while (bytew != -1 && m1 == NULL)
        if (bytew != -1 || m1 == NULL)
            m1 = next_message();

        std::cout << "alice before if: " << bytew << std::endl;
        if (m1)
        {
            // std::cout << "alice before send" << std::endl;
            send(m1);
            // std::cout << "alice after send" << std::endl;
            const Message *m2 = recv();
            // std::cout << "alice after recv" << std::endl;
            if (m2 != NULL) {
                record(m2);
            }
        }
        else
        {
            time_t dt = now() - test_cases.front().first;
            timespec req = {dt / SECOND_TO_NANO, dt % SECOND_TO_NANO}, rem;
            nanosleep(&req, &rem); // 等待到下一条消息的发送时间
        }
    }

    return 0;
}


/*Blocking FIFO(Original)*/
// void send(const Message *message)
// {
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "alice_to_bob";
//         if (access(filename, F_OK)) { // return 0 if file exists, -1 if no exists
//             mkfifo(filename, 0666);
//         }
//         fifo = open(filename, O_WRONLY);
//         assert(fifo != 0);
//     }
//     assert(write(fifo, message, message->size) == message->size);
// }
// const Message *recv()
// {
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "bob_to_alice";
//         if (access(filename, F_OK)) {
//             mkfifo(filename, 0666);
//         }
//         fifo = open(filename, O_RDONLY);
//         assert(fifo != 0);
//     }
//     static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
//     assert(read(fifo, m, sizeof(Message)) == sizeof(Message));
//     assert(read(fifo, m->payload, m->payload_size()) == m->payload_size());
//     return m;
// }

/*Shared Memory*/
// sem_t *full_ab = sem_open("/full_ab", O_CREAT, 0644, 0);
// sem_t *empty_ab = sem_open("/empty_ab", O_CREAT, 0644, 1);
// sem_t *mutex_ab = sem_open("/mutex_ab", O_CREAT, 0644, 1);
// sem_t *full_ba = sem_open("/full_ba", O_CREAT, 0644, 0);
// sem_t *empty_ba = sem_open("/empty_ba", O_CREAT, 0644, 1);
// sem_t *mutex_ba = sem_open("/mutex_ba", O_CREAT, 0644, 1);
//
// void deepCopy(Message *str, const Message *message) {
//     str->t = message->t;
//     str->size = message->size;
//     str->checksum = message->checksum;
//     int *ps = (int *)(str->payload);
//     int *pm = (int *)(message->payload);
//     for (auto i = message->payload_size() / 4; i; --i) {
//         *ps++ = *pm++;
//     }
// }
// void send(const Message *message)
// {
//     static Message *str;
//     if (str == NULL) {
//         key_t key = ftok("alice_to_bob",65);
//         int shmid = shmget(key,MESSAGE_SIZES[4],0666|IPC_CREAT);
//         str = (Message*) shmat(shmid,(void*)0,0);
//         assert(str != (void *)-1);
//     }
//     sem_wait(empty_ab);
//     sem_wait(mutex_ab);
//     /*User-defined deepCopy*/
//     // deepCopy(str, message);
//     /*library memory copy memcpy*/
//     memcpy (str, message, message->size);
//     sem_post(mutex_ab);
//     sem_post(full_ab);
// }
// const Message *recv()
// {
//     static Message *str;
//     if (str == NULL) {
//         key_t key = ftok("bob_to_alice",65);
//         int shmid = shmget(key,MESSAGE_SIZES[4],0666|IPC_CREAT);
//         str = (Message*) shmat(shmid,(void*)0,0);
//         assert(str != (void *)-1);
//     }
//     static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
//     sem_wait(full_ba);
//     sem_wait(mutex_ba);
//     /*User-defined deepCopy*/
//     // deepCopy(m, str);
//     /*library memory copy memcpy*/
//     memcpy (m, str, str->size);
//     sem_post(mutex_ba);
//     sem_post(empty_ba);
//     return m;
// }

/*Blocking FIFO(Original)*/ /*Shared Memory*/
// int main()
// {
//     while (true)
//     {
//
//         const Message *m1 = next_message();
//         if (m1)
//         {
//             send(m1);
//             const Message *m2 = recv();
//             record(m2);
//         }
//         else
//         {
//             time_t dt = now() - test_cases.front().first;
//             timespec req = {dt / SECOND_TO_NANO, dt % SECOND_TO_NANO}, rem;
//             nanosleep(&req, &rem); // 等待到下一条消息的发送时间
//         }
//     }
//
//     return 0;
// }
