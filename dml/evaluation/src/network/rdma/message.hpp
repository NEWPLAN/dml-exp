
#include <iostream>
#include <cstdlib>
#include <cstring>

#include <time.h>
#include <unistd.h>

class Message
{
public:
    Message() { std::cout << "Building Message" << std::endl; }
    ~Message() { std::cout << "Destroy Message" << std::endl; }

public:
    void encode(uint64_t *key, uint32_t id)
    {
        uint64_t code = 1;
        if (id < 64)
        {
            key[0] += code << id;
        }
        else if (64 <= id && id < 128)
        {
            key[1] += code << (id - 64);
        }
        else if (128 <= id && id < 192)
        {
            key[2] += code << (id - 128);
        }
        else if (192 <= id && id < 256)
        {
            key[3] += code << (id - 192);
        }
        else
        {
            printf("Error of unknown key with id: %u\n", id);
            exit(0);
        }
    }

    void decode(uint32_t *id, uint64_t *key,
                uint32_t base_pos, int32_t num)
    {
        uint32_t start = base_pos % 256;
        while (num > 0)
        {
            int processed = 0;
            uint64_t bit_mask = 1;
            if (start < 64)
            {
                if (key[0] & bit_mask << start)
                {
                    processed = 1;
                }
            }
            else if (64 <= start && start < 128)
            {
                if (key[1] & bit_mask << (start - 64))
                {
                    processed = 1;
                }
            }
            else if (128 <= start && start < 192)
            {
                if (key[2] & bit_mask << (start - 128))
                {
                    processed = 1;
                }
            }
            else if (192 <= start && start < 256)
            {
                if (key[3] & bit_mask << (start - 192))
                {
                    processed = 1;
                }
            }
            else
            {
                printf("Error of unknown key with id: %u\n", start);
                exit(0);
            }
            if (processed)
            {
                *id = start;
                id++;
            }
            num--;
            start = (start + 1) % 256;
        }
    }

    int test()
    {
        uint64_t key[4];

        int decode_data[256];
        int encode_data[256];
        for (int try_time = 0; try_time < 10000000; try_time++)
        {
#ifdef DEBUG
            memset(encode_data, 0, sizeof(int) * 256);
            int start_pos = rand() % 256;
            int num = 1 + rand() % 256;
#else
            int start_pos = 64;
            int num = 128;
#endif
            int encode_num = 0;
            int start = start_pos;
            for (int index = 0; index < 256; index++)
            {

#ifdef DEBUG
                if (start + num - encode_num + index < start_pos + 256)
                {
                    if (rand() % 99 < 39)
                    {
                        continue;
                    }
                }
#endif
                encode_data[encode_num++] = (start + index) % 256;
            }

            //memset(key, 0, sizeof(key));
            key[0] = key[1] = key[2] = key[3] = 0;
            for (int index = 0; index < num; index++)
            {
                encode(key, encode_data[index]);
            }
            //memset(decode_data, 0, sizeof(int) * 256);
            decode((uint32_t *)decode_data, key,
                   encode_data[0], num);

            if (strncmp((char *)encode_data,
                        (char *)decode_data,
                        sizeof(int) * num) != 0)
            {

                printf("\n\nEncode pos: %d, num: %d: ", encode_data[0], num);
                printf("KEY: %lx %lx %lx %lx\n",
                       key[0], key[1], key[2], key[3]);
                printf("Encode data: ");
                for (int index = 0; index < num; index++)
                    printf("%4d", encode_data[index]);
                printf("\nDecode data: ");
                for (int index = 0; index < num; index++)
                    printf("%4d", decode_data[index]);
                sleep(1);
            }
            else
            {
                //printf("matched\n");
            }
        }
        return 0;
    }
};

// int main(int argc, char *argv[])
// {
//     Message *msg = new Message();
//     msg->test();
//     delete msg;
// }