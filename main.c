#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Filesystem includes */
#include "filesystem.h"
#include "fio.h"

/* string includes*/
#include "string-util.h"

/* self-defined shell func*/
#include "shell.h"

/* maximun length of command */
#define CMD_MAX_LEN 100

typedef struct {
	portCHAR ch;
}serial_ch_msg;

extern const char _sromfs;

static void setup_hardware();

volatile xSemaphoreHandle serial_tx_wait_sem = NULL;
volatile xQueueHandle serial_rx_queue = NULL;


/* IRQ handler to handle USART2 interruptss (both transmit and receive
 * interrupts). */
void USART2_IRQHandler()
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;
	serial_ch_msg rx_msg;

	/* If this interrupt is for a transmit... */
	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
		/* "give" the serial_tx_wait_sem semaphore to notfiy processes
		 * that the buffer has a spot free for the next byte.
		 */
		xSemaphoreGiveFromISR(serial_tx_wait_sem, &xHigherPriorityTaskWoken);

		/* Diables the transmit interrupt. */
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		/* If this interrupt is for a receive... */
	}else if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		/* Receive the byte from the buffer. */
		rx_msg.ch = USART_ReceiveData(USART2);

		/* Queue the received byte. */
		if(!xQueueSendToBackFromISR(serial_rx_queue, &rx_msg, &xHigherPriorityTaskWoken)) {
			/* If there was an error queueing the received byte,
			 * freeze. */
			while(1);
		}
	}
	else {
		/* Only transmit and receive interrupts should be enabled.
		 * If this is another type of interrupt, freeze.
		 */
		while(1);
	}

	if (xHigherPriorityTaskWoken) {
		taskYIELD();
	}
}

void send_byte(char ch)
{
	/* Wait until the RS232 port can receive another byte (this semaphore
	 * is "given" by the RS232 port interrupt when the buffer has room for
	 * another byte.
	 */
	while (!xSemaphoreTake(serial_tx_wait_sem, portMAX_DELAY));

	/* Send the byte and enable the transmit interrupt (it is disabled by
	 * the interrupt).
	 */
	USART_SendData(USART2, ch);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void puts(char * str)
{
        int i = 0;
        while(str[i]){
                send_byte(str[i]);
                i++;
        }
}


char receive_byte()
{
	serial_ch_msg msg;
	/* Wait for a byte to be queued by the receive interrupts handler. */
	while (!xQueueReceive(serial_rx_queue, &msg, portMAX_DELAY));
	
	return msg.ch;
}

void read_romfs_task(void *pvParameters)
{
	char buf[128];
	size_t count;
	int fd = fs_open("/romfs/test.txt", 0, O_RDONLY);
	do {
		//Read from /romfs/test.txt to buffer
		count = fio_read(fd, buf, sizeof(buf));
		
		//Write buffer to fd 1 (stdout, through uart)
		fio_write(1, buf, count);
	} while (count);
	
	while (1);
}

void shell_task(void * pvParameters)
{

	char * prompt = ">>> ";
	char ch ;

	char cmd [CMD_MAX_LEN];
	int curr_ch = 0;
	
	int done;
	
	while(1){
		puts(prompt);
		curr_ch = 0;
		done = 0;
		do{
			ch = receive_byte();
			if( curr_ch < CMD_MAX_LEN){
				if( ch == '\n' || ch =='\r'){
					cmd[curr_ch] = '\0';
					done = -1;
					puts("\n\r\0");

					cmd_arbiter(cmd);// summit a command to look for reaction

				}else{
					cmd[curr_ch++] = ch;
					send_byte(ch);
				}
			}
		}while(!done);
	
	}


}

void test_task(void * pvParameters)
{
	while(1);
}

void create_queue_semaphore(){

	vSemaphoreCreateBinary(serial_tx_wait_sem);
	serial_rx_queue = xQueueCreate(1,sizeof(serial_ch_msg));

	/*check if the serial_rx_queue created*/
	if(serial_rx_queue == 0){
		puts("fail to create:serial_rx_queue\n\r");
	}
}

int main()
{
	init_rs232();
	enable_rs232_interrupts();
	enable_rs232();

	fs_init();
	fio_init();
	
	register_romfs("romfs", &_sromfs);
	
	create_queue_semaphore();

	/* Create a task to operate a shell*/
	xTaskCreate(	shell_task,
	            	(signed portCHAR *) "Shell",
	            	512 /* stack size */, 
			NULL, 
			tskIDLE_PRIORITY + 10, 
			NULL);

	xTaskCreate(	test_task,
		    	(signed portCHAR *) "Test",
			512,
			NULL,
			tskIDLE_PRIORITY + 9,
			NULL);

	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

void vApplicationTickHook()
{
}
