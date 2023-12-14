#include "mbed.h"
#include "LSM6DS3.h"
#include "DHT22.h"

typedef struct
{
    float temp, humidity;
    float x, y, z;
} message_t;

DHT22 DHT(PA_10);
LSM6DS3 ACC(I2C_SDA, I2C_SCL, LSM6DS3_AG_I2C_ADDR(0));
MemoryPool<message_t, 18> pool;
Queue<message_t, 18> queue;
Thread thread;

void send_Values(void)
{
    while (true)
    {
        if (!queue.full())
        {
            ACC.readAccel();
            DHT.sample();
            message_t *message = pool.alloc();
            message->humidity = DHT.getHumidity();
            message->temp = DHT.getTemperature();
            message->x = ACC.ax;
            message->y = ACC.ay;
            message->z = ACC.az;
            queue.put(message);
            ThisThread::sleep_for(1s);
        }
        else
        {
            printf("Ошибка! Очередь заполнена.\n\r");
            break;
        }
    }
}
int main(void)
{

    ACC.begin();
    thread.start(callback(send_Values));
    while (true)
    {
        osEvent evt = queue.get();
        if (evt.status == osEventMessage)
        {
            message_t *message = (message_t *)evt.value.p;
            printf("Температура: %.2f\n\r ", message->temp);
            printf("Влажность: %.2f\n\r", message->humidity);
            printf("x: %.2f\n\r", message->x);
            printf("y: %.2f\n\r", message->y);
            printf("z: %.2f\n\r", message->z);
            pool.free(message);
        }
    }
}