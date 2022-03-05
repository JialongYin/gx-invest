#include "common.h"


sem_t *empty_ab = sem_open("/empty_ab", O_CREAT, 0644, 1);
sem_t *full_ab = sem_open("/full_ab", O_CREAT, 0644, 0);
sem_t *mutex_ab = sem_open("/mutex_ab", O_CREAT, 0644, 1);

sem_t *empty_ba = sem_open("/empty_ba", O_CREAT, 0644, 1);
sem_t *full_ba = sem_open("/full_ba", O_CREAT, 0644, 0);
sem_t *mutex_ba = sem_open("/mutex_ba", O_CREAT, 0644, 1);


void send(const Message *message)
{
    // static int fifo = 0;
    // if (fifo == 0)
    // {
    //     const char *filename = "bob_to_alice";
    //     if (access(filename, F_OK)) { // return 0 if file exists, -1 if no exists
    //         mkfifo(filename, 0666);
    //     }
    //     fifo = open(filename, O_WRONLY);
    //     assert(fifo != 0);
    // }
    // assert(write(fifo, message, message->size) == message->size);


    static Message *str;
    if (str == NULL) {
        key_t key = ftok("bob_to_alice",65);
        int shmid = shmget(key,MESSAGE_SIZES[4],0666|IPC_CREAT);
        str = (Message*) shmat(shmid,(void*)0,0);
        assert(str != (void *)-1);
    }
    sem_wait(empty_ba);
    sem_wait(mutex_ba);
    deepCopy(str, message);
    sem_post(mutex_ba);
    sem_post(full_ba);
}

const Message *recv()
{
    // static int fifo = 0;
    // if (fifo == 0)
    // {
    //     const char *filename = "alice_to_bob";
    //     if (access(filename, F_OK)) {
    //         mkfifo(filename, 0666);
    //     }
    //
    //     fifo = open(filename, O_RDONLY);
    //     assert(fifo != 0);
    // }
    // static Message *m = (Message *)malloc(MESSAGE_SIZES[4]);
    // assert(read(fifo, m, sizeof(Message)) == sizeof(Message));
    // assert(read(fifo, m->payload, m->payload_size()) == m->payload_size());
    // return m;


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
    deepCopy(m, str);
    sem_post(mutex_ab);
    sem_post(empty_ab);
    return m;
}

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
