#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
#define MSG_SIZE 64

static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(flexcomm2));
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

void uart_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev)) {
		return;
	}

	while (uart_fifo_read(dev, &c, 1) == 1) {
		if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}

		/* End of response (rudimentary check) */
		if (c == '\n' || c == '\r') {
			rx_buf[rx_buf_pos] = '\0';
			printk("Modem says: %s\n", rx_buf);
			rx_buf_pos = 0;
		}
	}
}

void uart_send(const char *cmd)
{
	while (*cmd) {
		uart_poll_out(uart_dev, *cmd++);
	}
}

int main(void)
{
	if (!device_is_ready(uart_dev)) {
		printk("UART device not ready!\n");
		return 0;
	}

	uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
	uart_irq_rx_enable(uart_dev);

	printk("Sending AT command: AT+RST\r\n");
	uart_send("AT+RST\r\n");

	/* Main loop does nothing, we just wait for callback output */
	while (1) {
		k_sleep(K_MSEC(100));
	}

	return 0;
}
