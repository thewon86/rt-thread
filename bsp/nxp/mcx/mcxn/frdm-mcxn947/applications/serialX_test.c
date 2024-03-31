#include <rtthread.h>
#include <board.h>
#include <rthw.h>
#include "rtdevice.h"

#ifdef RT_USING_SERIAL_X
#define RT_SERIAL_CONFIG_USER              \
{                                          \
    BAUD_RATE_115200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_FIFO_BUFSZ, /* Buffer size */  \
    0                                      \
}
#elif defined(RT_USING_SERIAL_V2)
#define RT_SERIAL_CONFIG_USER              \
{                                          \
    BAUD_RATE_115200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    BSP_UART1_RX_BUFSIZE, /* rxBuf size */   \
    BSP_UART1_RX_BUFSIZE, /* txBuf size */   \
    0                                      \
}
#else
#define RT_SERIAL_CONFIG_USER              \
{                                          \
    BAUD_RATE_115200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_RB_BUFSZ,                    \
    0                                      \
}
#endif

#define SERIAL_DEV_NAME   "uart4"

//#define SERIAL_CLOSEOPEN_TEST
#define SERIAL_POLL_TEST
//#define SERIAL_FLUSH_TEST
//#define SERIAL_NONBLOCK_WRITE_TEST
//#define SERIAL_NONBLOCK_READ_TEST
//#define SERIAL_BLOCK_WRITE_TEST
//#define SERIAL_BLOCK_READ_TEST
#define SERIAL_RW_TEST

static rt_thread_t uart_tid = RT_NULL;

#if defined(SERIAL_CLOSEOPEN_TEST)
static rt_bool_t close_open(rt_device_t uart_dev)
{
    int i;

    rt_kprintf("CLOSE & REOPEN\n");
    rt_thread_mdelay(1000);
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    rt_device_write(uart_dev, 0, "INT mode opened\n", rt_strlen("INT mode opened\n"));
    rt_thread_mdelay(1000);
    for (i = 0; i < 100000; i++) {
        rt_device_close(uart_dev);
        if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
        {
            rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
            return RT_FALSE;
        }
    }
    rt_thread_mdelay(1000);
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    rt_kprintf("REOPEN successfull\n");

    return RT_TRUE;
}
#endif // SERIAL_CLOSEOPEN_TEST

#if defined(SERIAL_POLL_TEST)
static rt_bool_t pool_write(rt_device_t uart_dev, rt_uint8_t *recvbuf)
{
    rt_kprintf("\n\npoll WRITE\n");
    rt_thread_mdelay(3000);
    rt_device_write(uart_dev, 0, recvbuf, 1024);
    rt_device_write(uart_dev, 0, recvbuf, 1024);
    rt_device_write(uart_dev, 0, recvbuf, 1024);
    rt_device_write(uart_dev, 0, recvbuf, 1024);
    rt_device_write(uart_dev, 0, recvbuf, 1024);
    rt_kprintf("\npoll END\n");

    return RT_TRUE;
}
#endif // SERIAL_POLL_TEST

#if defined(SERIAL_FLUSH_TEST)
static rt_bool_t flush_write(rt_device_t uart_dev, rt_uint8_t *recvbuf)
{
    int i;

    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    rt_device_write(uart_dev, 0, "\n\nNO FLUSH, some data may be lost\n", rt_strlen("\n\nNO FLUSH, some data may be lost\n"));
    rt_thread_mdelay(1000);
    for (i = 0; i < 10; i++) {
    rt_device_write(uart_dev, 0, recvbuf, 1024);
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    }
    rt_device_write(uart_dev, 0, "\n\nFLUSH, no data lost\n", rt_strlen("\n\nflush, no data lost\n"));
    rt_thread_mdelay(1000);
    for (i = 0; i < 10; i++) {
    rt_device_write(uart_dev, 0, recvbuf, 1024);
#ifdef RT_USING_SERIAL_X
    rt_device_flush(uart_dev);
#endif
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    }
    return RT_TRUE;
}
#endif // SERIAL_FLUSH_TEST

#if defined(SERIAL_NONBLOCK_WRITE_TEST)
static rt_bool_t nonblock_write(rt_device_t uart_dev, rt_uint8_t *recvbuf, rt_uint8_t *buf)
{
    rt_ssize_t wr_sz = 0, recv_sz = 0, tmp = 0, i;
    rt_tick_t tick1, tick2;
    for (tmp = 0; tmp < 1024; tmp++) {
        recvbuf[tmp] = '0' + (tmp % 60);
    }
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
                               | RT_DEVICE_OFLAG_NONBLOCKING
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_NON_BLOCKING
                               | RT_DEVICE_FLAG_TX_NON_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    rt_device_write(uart_dev, 0, "\n\nnonblock WRITE\n", rt_strlen("\n\nnonblock WRITE\n"));
    rt_device_write(uart_dev, 0, "3\n", 2);
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "2\n", 2);
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "1\n", 2);
    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    tmp = 0;
    for (i = 0; i < 100; i++) {
        wr_sz = 0;
        while(wr_sz < 1024) {
            wr_sz += rt_device_write(uart_dev, 0, &recvbuf[wr_sz], 1024-wr_sz);
        }
        tmp += wr_sz;
    }
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nnonblock : write %d / 1024*10 bytes used %d ticks\n", tmp, tick2 - tick1);
    wr_sz = 0;
    tmp = rt_strlen(buf);
    while(wr_sz < tmp) {
        wr_sz += rt_device_write(uart_dev, 0, &buf[wr_sz], tmp-wr_sz);
    }

    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    tmp = rt_device_write(uart_dev, 0, &recvbuf[wr_sz], 16);
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nnonblock : write %d / 16 bytes used %d ticks\n", tmp, tick2 - tick1);
    wr_sz = 0;
    tmp = rt_strlen(buf);
    while(wr_sz < tmp) {
        wr_sz += rt_device_write(uart_dev, 0, &buf[wr_sz], tmp-wr_sz);
    }
    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    tmp = rt_device_write(uart_dev, 0, &recvbuf[wr_sz], 64);
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nnonblock : write %d / 64 bytes used %d ticks\n", tmp, tick2 - tick1);
    wr_sz = 0;
    tmp = rt_strlen(buf);
    while(wr_sz < tmp) {
        wr_sz += rt_device_write(uart_dev, 0, &buf[wr_sz], tmp-wr_sz);
    }
    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    tmp = rt_device_write(uart_dev, 0, &recvbuf[wr_sz], 128);
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nnonblock : write %d / 128 bytes used %d ticks\n", tmp, tick2 - tick1);
    wr_sz = 0;
    tmp = rt_strlen(buf);
    while(wr_sz < tmp) {
        wr_sz += rt_device_write(uart_dev, 0, &buf[wr_sz], tmp-wr_sz);
    }

// nonblock ticks
    tick1 = rt_tick_get();
    tmp = 0;
    for (i = 0; i < 8; i++) {
        wr_sz = 0;
        while(wr_sz < 16) {
            wr_sz += rt_device_write(uart_dev, 0, &recvbuf[wr_sz], 16-wr_sz);
        }
        tmp += wr_sz;
    }
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\n\nnonblock : write %d / 128 bytes used %d ticks\n", tmp, tick2 - tick1);
    wr_sz = 0;
    tmp = rt_strlen(buf);
    while(wr_sz < tmp) {
        wr_sz += rt_device_write(uart_dev, 0, &buf[wr_sz], tmp-wr_sz);
    }

    return RT_TRUE;
}
#endif // SERIAL_NONBLOCK_WRITE_TEST

#if defined(SERIAL_NONBLOCK_READ_TEST)
static rt_bool_t nonblock_read(rt_device_t uart_dev, rt_uint8_t *recvbuf, rt_uint8_t *buf)
{
    rt_ssize_t recv_sz = 0;

    rt_device_write(uart_dev, 0, "\n\nnonblock READ\n", rt_strlen("\n\nnonblock READ\n"));
    rt_device_write(uart_dev, 0, "\nSEND somethings\n", rt_strlen("\nSEND somethings\n"));
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "3\n", 2);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nnonblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "2\n", 2);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nnonblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "1\n", 2);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nnonblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nnonblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));

    rt_sprintf(buf, "\rnonblock read END\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));

#ifdef RT_USING_SERIAL_X
    rt_device_flush(uart_dev);
#endif
    return RT_TRUE;
}
#endif // SERIAL_NONBLOCK_READ_TEST

#if defined(SERIAL_BLOCK_WRITE_TEST)
static rt_bool_t block_write(rt_device_t uart_dev, rt_uint8_t *recvbuf, rt_uint8_t *buf)
{
    rt_ssize_t wr_sz = 0, recv_sz = 0, tmp = 0, i;
    rt_tick_t tick1, tick2;
    for (tmp = 0; tmp < 1024; tmp++) {
        recvbuf[tmp] = '0' + (tmp % 60);
    }
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
                               | RT_DEVICE_OFLAG_BLOCKING
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return RT_FALSE;
    }
    rt_device_write(uart_dev, 0, "\n\nblock WRITE\n", rt_strlen("\n\nblock WRITE\n"));
    rt_device_write(uart_dev, 0, "3\n", 2);
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "2\n", 2);
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "1\n", 2);
    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    wr_sz = 0;
    for (i = 0; i < 100; i++) {
        wr_sz += rt_device_write(uart_dev, 0, recvbuf, 1024);
    }
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nblock : write %d / 128 bytes use %d ticks\n", wr_sz, tick2 - tick1);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));

    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    wr_sz = rt_device_write(uart_dev, 0, recvbuf, 16);
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nblock : write %d / 16 bytes use %d ticks\n", wr_sz, tick2 - tick1);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    wr_sz = rt_device_write(uart_dev, 0, recvbuf, 64);
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nblock : write %d / 64 bytes use %d ticks\n", wr_sz, tick2 - tick1);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    tick1 = rt_tick_get();
    wr_sz = rt_device_write(uart_dev, 0, recvbuf, 128);
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\nblock : write %d / 128 bytes use %d ticks\n", wr_sz, tick2 - tick1);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
// block ticks
    tick1 = rt_tick_get();
    wr_sz = 0;
    for (i = 0; i < 8; i++) {
        wr_sz += rt_device_write(uart_dev, 0, recvbuf, 16);
    }
    tick2 = rt_tick_get();
    rt_sprintf(buf, "\n\nblock : write %d / 128 bytes use %d ticks\n", wr_sz, tick2 - tick1);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));

    return RT_TRUE;
}
#endif // SERIAL_BLOCK_WRITE_TEST

#if defined(SERIAL_BLOCK_READ_TEST)
static rt_bool_t block_read(rt_device_t uart_dev, rt_uint8_t *recvbuf, rt_uint8_t *buf)
{
    rt_ssize_t recv_sz = 0;

    rt_device_write(uart_dev, 0, "\n\nblock READ\n", rt_strlen("\n\nblock READ\n"));
    rt_device_write(uart_dev, 0, "\nSEND somethings\n", rt_strlen("\nSEND somethings\n"));
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "3\n", 2);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "2\n", 2);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    rt_device_write(uart_dev, 0, "1\n", 2);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\nblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
    rt_thread_mdelay(1000);
    recv_sz = rt_device_read(uart_dev, -1, recvbuf, 512);
    rt_sprintf(buf, "\rblock : %d bytes read\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));

    rt_sprintf(buf, "\rblock read END\n", recv_sz);
    rt_device_write(uart_dev, 0, buf, rt_strlen(buf));
#ifdef RT_USING_SERIAL_X
    rt_device_flush(uart_dev);
#endif
    return RT_TRUE;
}
#endif // SERIAL_BLOCK_READ_TEST

static void uart_test_thread(void *parameter)
{
    rt_device_t uart_dev;
    rt_uint8_t *recvbuf = RT_NULL, *buf = RT_NULL;
    rt_ssize_t wr_sz = 0, recv_sz = 0, tmp = 0, i;
    rt_tick_t tick1, tick2;
    struct serial_configure uart_conf = RT_SERIAL_CONFIG_USER;

    uart_dev = rt_device_find(SERIAL_DEV_NAME);
    if (uart_dev == RT_NULL) {
        rt_kprintf("Find device: %s failed\n", SERIAL_DEV_NAME);
        return;
    }
    rt_device_write(uart_dev, 0, "device found\n", rt_strlen("device found\n"));
    rt_device_control(uart_dev, RT_DEVICE_CTRL_CONFIG, &uart_conf);
#if 0
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return;
    }
    rt_kprintf("POLL mode opened\n");
#endif
#ifdef RT_USING_SERIAL_X
    rt_device_flush(uart_dev);
#endif
    recvbuf = rt_malloc(1024);
    buf = rt_malloc(64);
//    rt_memset(recvbuf, 0x5A, 512);
    for (tmp = 0; tmp < 1024; tmp++) {
        recvbuf[tmp] = '0' + (tmp % 60);
    }

// close reopen test
#if defined(SERIAL_CLOSEOPEN_TEST)
    close_open(uart_dev);
    rt_thread_mdelay(1000);
#endif // SERIAL_CLOSEOPEN_TEST

#if defined(SERIAL_POLL_TEST)
    pool_write(uart_dev, recvbuf);
    rt_thread_mdelay(1000);
#endif // SERIAL_POLL_TEST

// flush test
#if defined(SERIAL_FLUSH_TEST)
    flush_write(uart_dev, recvbuf);
#endif // SERIAL_FLUSH_TEST

// nonblock write ,more than tx buffer room
#if defined(SERIAL_NONBLOCK_WRITE_TEST)
    nonblock_write(uart_dev, recvbuf, buf);
    rt_thread_mdelay(1000);
#endif // SERIAL_NONBLOCK_WRITE_TEST

// nonblock read
#if defined(SERIAL_NONBLOCK_READ_TEST)
    nonblock_read(uart_dev, recvbuf, buf);
#endif // SERIAL_NONBLOCK_READ_TEST

// block write ,more than tx buffer room
#if defined(SERIAL_BLOCK_WRITE_TEST)
    block_write(uart_dev, recvbuf, buf);
    rt_thread_mdelay(1000);
#endif // SERIAL_BLOCK_WRITE_TEST

// block read
#if defined(SERIAL_BLOCK_READ_TEST)
    block_read(uart_dev, recvbuf, buf);
#endif // SERIAL_BLOCK_READ_TEST

// read write loopback test
    rt_device_close(uart_dev);
    if (rt_device_open(uart_dev, RT_DEVICE_OFLAG_RDWR
#ifdef RT_USING_SERIAL_X
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
                               | RT_DEVICE_OFLAG_BLOCKING
#elif defined(RT_USING_SERIAL_V2)
                               | RT_DEVICE_FLAG_RX_BLOCKING
                               | RT_DEVICE_FLAG_TX_BLOCKING
#else
    #ifdef RT_SERIAL_USING_DMA
                               | RT_DEVICE_FLAG_DMA_RX
                               | RT_DEVICE_FLAG_DMA_TX
    #else
                               | RT_DEVICE_FLAG_INT_RX
                               | RT_DEVICE_FLAG_INT_TX
    #endif
#endif
                        ) != RT_EOK)
    {
        rt_kprintf("Open device: %s failed\n", SERIAL_DEV_NAME);
        return;
    }
    rt_device_write(uart_dev, 0, "\n\nblock LOOPBACK\n", rt_strlen("\n\nblock loopback\n"));

    while (1) {
#if defined(SERIAL_WRITE_TEST)
        rt_device_write(uart_dev, 0, recvbuf, 12);
        rt_thread_mdelay(100);
#elif defined(SERIAL_READ_TEST)
        recv_sz = rt_device_read(uart_dev, -1, recvbuf, 128);
        rt_thread_delay(2);
#elif defined(SERIAL_RW_TEST)
        recv_sz = rt_device_read(uart_dev, -1, recvbuf, 128);
        if (recv_sz > 0) {
            recvbuf[recv_sz] = 0;
            rt_device_write(uart_dev, 0, recvbuf, recv_sz);
        }
        rt_thread_delay(2);
#else
        break;
#endif
    }
    rt_device_write(uart_dev, 0, "\n\nTest END\n", rt_strlen("\n\ntest end\n"));
}

int uart_test_init(void)
{
    uart_tid = rt_thread_create("uartX",              
                           uart_test_thread, RT_NULL,
                           2048,
                           0,
                           20);
    if (uart_tid != RT_NULL)
        rt_thread_startup(uart_tid);

    return 0;
}
INIT_APP_EXPORT(uart_test_init);
