/*
 * Sigfox demo (using spidev driver)
 *
 * Copyright (C) 2017 NXP semiconductors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <time.h> 

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MAX_BUF_LEN 40

static void pabort(const char *s)
{
	perror(s);
	abort();
}

char device[10];
uint32_t mode;
uint8_t bits = 8;
uint32_t speed = 500000;
uint16_t delay;
int verbose;
int rzN = 1;
uint8_t wakeup_tx[] = {0x02,0x01};
uint8_t test_tx[] = {0x07,0x02,0x01,0x02,0x03,0x04,0x05};
uint8_t network_standard_tx[] = {0x02,0x14};
uint8_t sleep_tx[] = {0x02,0x03};
uint8_t payload_tx[] = {0x07,0x04,0x02,0x00,0x00,0x00,0x00};
uint8_t default_tx[30];
uint8_t default_rx[30];
char * rzstr[]= {"RCZ1 ETSI Europe (863 MHz – 870 MHz)","RCZ2 FCC US (902 MHz – 928 MHz)","RCZ3 ARIB Japan, Korea (915 MHz – 930 MHz)","RCZ4 FCC Latin America, Australia, New Zealand (902 MHz – 915 MHz)"};

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr.tx_buf = 0;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
/*	printf("tx:");
	int i;
	for(i=0;i<len;i++)
	    printf("0x%02x ",tx[i]);
	printf("\n");
	printf("rx:");
	for(i=0;i<len;i++)
	    printf("0x%02x ",rx[i]);
	printf("\n");
*/
}

void parse_config(char *config_file)
{
	FILE * file = fopen(config_file, "r");
	if(file ==  NULL)
	{
		printf("[error] open %s failed\n",config_file);
		exit(1);
	}
	char buf[MAX_BUF_LEN];
	char param[20];
	char value[20];
	while(fgets(buf, MAX_BUF_LEN, file) != NULL)
	{
		sscanf(buf,"%s %s",param,value);
		printf("%s=%s\n",param,value);
		switch (param[0])
		{
			case 'D':
				strcpy(device,value);
				break;
			case 's':
				speed = atoi(value);
				break;
			case 'b':
				bits = atoi(value);
				break;
			case 'n':
				rzN = atoi(value);
		}
	}
	fclose(file);
}

void set_gpio_ack(void)
{
	system("echo 434 > /sys/class/gpio/export");
	system("echo in > /sys/class/gpio/gpio434/direction");
}

int get_response(int fd)
{
	int fd_gpio;
	char value_str[3];
	int value;
	int len;
	int ret = -1;

	do
	{
		sleep(1);
		fd_gpio = open("/sys/class/gpio/gpio434/value", O_RDONLY);
		if(fd_gpio == -1)
		{
			printf("open gpio434 error\n");
			close(fd);
			exit(1);
		}
		read(fd_gpio, value_str, 3);
		value = atoi(value_str);
		close(fd_gpio);
	}while(value == 1);

	if(value == 0)
	{
		transfer(fd, default_tx, default_rx, 1);
		len = default_rx[0];
		if(len > 0)
		{
			transfer(fd, default_tx, default_rx, len - 1);
			ret = default_rx[0];
		}
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	uint8_t *tx;
	uint8_t *rx;
	int size;

	parse_config("/etc/sigfox.conf");

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	mode |= SPI_CPHA;
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	/* wakeup sleep */
	printf("wakeup sigfox device:\n");
	transfer(fd, wakeup_tx, default_rx, wakeup_tx[0]);
	sleep(3);
	printf("...........[ok]\n");

	set_gpio_ack();

	/* Test SPI connective */
	printf("Test SPI connective:\n");
	transfer(fd, test_tx, default_rx, test_tx[0]);
	ret = get_response(fd);
	if(ret != 0 )
	{
		printf("Test SPI connective error, error code = %d\n",ret);
		close(fd);
		exit(1);
	}
	printf("...........[ok]\n");

	/* set network standard */
	sleep(1);
	printf("set network standard:%s :\n",rzstr[rzN]);
	network_standard_tx[1] += rzN;
	transfer(fd, network_standard_tx, default_rx, network_standard_tx[0]);
	ret = get_response(fd);
	if(ret != 0 )
	{
		printf("set network standard error, error code = %d\n",ret);
		close(fd);
		exit(1);
	}
	printf("...........[ok]\n");

	sleep(1);
	printf("do you want to send a message?[N/y]:\n");
	while(getchar()=='y')
	{
		getchar();
		/* send one frame */
		time_t t;
		struct tm * lt;
		time (&t);
		lt = localtime (&t);
		payload_tx[6] += lt->tm_min;
		transfer(fd, payload_tx, default_rx, payload_tx[0]);
		printf("sending message:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",payload_tx[2],
			payload_tx[3],payload_tx[4],payload_tx[5],payload_tx[6]);
		
		ret = get_response(fd);
		if(ret != 0 )
		{
			printf("send one frame error, error code = %d\n",ret);
		}
		printf("send success\n");
		printf("do you want to send a message?[N/y]:\n");
		sleep(1);
	}
	/* sleep sigfox device */
	printf("set sigfox device sleep\n");
	transfer(fd, sleep_tx, default_rx, sleep_tx[0]);
	close(fd);

	return ret;
}
