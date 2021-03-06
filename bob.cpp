#include "common.h"

/*Non-blocking FIFO(in progress)*/
// static ssize_t bytew, byter;
// void send(const Message *message)
// {
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "bob_to_alice";
//         if (access(filename, F_OK)) { // return 0 if file exists, -1 if no exists
//             mkfifo(filename, 0666);
//         }
//         fifo = open(filename, O_WRONLY | O_NONBLOCK);
//
//         assert(fifo != 0);
//     }
//
//     // static ssize_t bytew;
//     if ((bytew = write(fifo, message, message->size)) == -1) {
//         std::cout << "bob after send: " << bytew << std::endl;
//         return;
//     }
//     std::cout << "bob after send: " << bytew << std::endl;
//     assert(bytew == message->size);
// }
// const Message *recv()
// {
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "alice_to_bob";
//         if (access(filename, F_OK)) {
//             mkfifo(filename, 0666);
//         }
//         fifo = open(filename, O_RDONLY | O_NONBLOCK);
//
//         assert(fifo != 0);
//     }
//
//     static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
//     if ((byter = read(fifo, m, m->size)) == 0) {
//         // std::cout << "bob after recv: " << byter << std::endl;
//         return NULL;
//     }
//     std::cout << "bob after recv: " << byter << std::endl;
//     assert(byter == m->size);
//     return m;
// }
/*Non-blocking FIFO(in progress)*/
// int main()
// {
//     Message *m2 = (Message *)malloc(MESSAGE_SIZES[4]);
//     // int i = 1000;
//     while (true)
//     {
//         const Message *m1 = recv();
//         // std::cout << "bob after recv: " << byter << std::endl;
//         if (m1 != NULL) {
//             std::cout << "bob after recv: " << byter << std::endl;
//             assert(m1->checksum == crc32(m1));
//             memcpy(m2, m1, m1->size); // 拷贝m1至m2
//             m2->payload[0]++;         // 第一个字符加一
//             m2->checksum = crc32(m2); // 更新校验和
//             send(m2);
//         }
//     }
//
//     return 0;
// }


/*Blocking FIFO(Original)*/
// void send(const Message *message)
// {
//     static int fifo = 0;
//     if (fifo == 0)
//     {
//         const char *filename = "bob_to_alice";
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
//         const char *filename = "alice_to_bob";
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
sem_t *full_ab = sem_open("/full_ab", O_CREAT, 0644, 0);
sem_t *empty_ab = sem_open("/empty_ab", O_CREAT, 0644, 1);
sem_t *mutex_ab = sem_open("/mutex_ab", O_CREAT, 0644, 1);
sem_t *full_ba = sem_open("/full_ba", O_CREAT, 0644, 0);
sem_t *empty_ba = sem_open("/empty_ba", O_CREAT, 0644, 1);
sem_t *mutex_ba = sem_open("/mutex_ba", O_CREAT, 0644, 1);

void deepCopy(Message *str, const Message *message) {
    str->t = message->t;
    str->size = message->size;
    str->checksum = message->checksum;
    int *ps = (int *)(str->payload);
    int *pm = (int *)(message->payload);
    for (auto i = message->payload_size() / 4; i; --i) {
        *ps++ = *pm++;
    }
}
void send(const Message *message)
{
    static Message *str;
    if (str == NULL) {
        key_t key = ftok("bob_to_alice",65);
        int shmid = shmget(key,MESSAGE_SIZES[4],0666|IPC_CREAT);
        str = (Message*) shmat(shmid,(void*)0,0);
        assert(str != (void *)-1);
    }
    sem_wait(empty_ba);
    sem_wait(mutex_ba);
    /*User-defined deepCopy*/
    // deepCopy(str, message);
    /*library memory copy memcpy*/
    memcpy (str, message, message->size);
    sem_post(mutex_ba);
    sem_post(full_ba);
}
const Message *recv()
{
    static Message *str;
    if (str == NULL) {
        key_t key = ftok("alice_to_bob",65);
        int shmid = shmget(key,MESSAGE_SIZES[4],0666|IPC_CREAT);
        str = (Message*) shmat(shmid,(void*)0,0);
        assert(str != (void *)-1);
    }
    static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
    sem_wait(full_ab);
    sem_wait(mutex_ab);
    /*User-defined deepCopy*/
    // deepCopy(m, str);
    /*library memory copy memcpy*/
    memcpy (m, str, str->size);
    sem_post(mutex_ab);
    sem_post(empty_ab);
    return m;
}

/*Blocking FIFO(Original)*/ /*Shared Memory*/
int main()
{
    Message *m2 = (Message *)malloc(MESSAGE_SIZES[4]);
    while (true)
    {
        const Message *m1 = recv();
        assert(m1->checksum == crc32(m1));
        memcpy(m2, m1, m1->size); // 拷贝m1至m2
        m2->payload[0]++;         // 第一个字符加一
        m2->checksum = crc32(m2); // 更新校验和
        send(m2);

    }

    return 0;
}
