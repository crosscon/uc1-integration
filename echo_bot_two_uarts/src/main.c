#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#define MSG_SIZE 64

K_MSGQ_DEFINE(uart0_msgq, MSG_SIZE, 10, 4);
K_MSGQ_DEFINE(uart1_msgq, MSG_SIZE, 10, 4);

const struct device *uart0 = DEVICE_DT_GET(DT_NODELABEL(flexcomm3));
const struct device *uart1 = DEVICE_DT_GET(DT_NODELABEL(flexcomm2));

static char rx_buf0[MSG_SIZE];
static char rx_buf1[MSG_SIZE];
static int rx_pos0 = 0;
static int rx_pos1 = 0;

static void serial_cb(const struct device *dev, void *user_data)
{
	char *rx_buf;
	int *rx_pos;
	struct k_msgq *msgq;

	if (dev == uart0) {
		rx_buf = rx_buf0;
		rx_pos = &rx_pos0;
		msgq = &uart0_msgq;
	} else if (dev == uart1) {
		rx_buf = rx_buf1;
		rx_pos = &rx_pos1;
		msgq = &uart1_msgq;
	} else {
		return;
	}

	if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev)) {
		return;
	}

	uint8_t c;
	while (uart_fifo_read(dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && *rx_pos > 0) {
			rx_buf[*rx_pos] = '\0';
			k_msgq_put(msgq, rx_buf, K_NO_WAIT);
			*rx_pos = 0;
		} else if (*rx_pos < MSG_SIZE - 1) {
			rx_buf[(*rx_pos)++] = c;
		}
	}
}

static void print_uart(const struct device *dev, const char *msg)
{
	while (*msg) {
		uart_poll_out(dev, *msg++);
	}
}

int main(void)
{
	if (!device_is_ready(uart0) || !device_is_ready(uart1)) {
		printk("UART devices not ready!\n");
		return 1;
	}

	uart_irq_callback_user_data_set(uart1, serial_cb, NULL);
	uart_irq_rx_enable(uart1);  // Only enable input for uart1

	print_uart(uart1, "UART1 ready. Type something:\r\n");

	char tx_buf[MSG_SIZE];

	while (1) {
		// Periodic message to UART0
		print_uart(uart0, "UART0 says: Hello from echo bot!\r\n");

		// Handle UART1 echo
		if (k_msgq_get(&uart1_msgq, &tx_buf, K_NO_WAIT) == 0) {
			print_uart(uart1, "UART1: ");
			print_uart(uart1, tx_buf);
			print_uart(uart1, "\r\n");
		}
	}
}
