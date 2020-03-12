
#include <rtdm/driver.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include "ipcore_io.h"

#define CONTROLLER_IP_CORE_MEMORY_BASE	0xC0000000
#define CONTROLLER_IP_CORE_MEMORY_SIZE	0x00100000
#define CONTROLLER_IP_CORE_IRQ_LINE		43


struct controller_data {
	void __iomem* address_virtual;
	unsigned address_physical;

	rtdm_irq_t irq_m;
    unsigned irq_line;
	rtdm_sem_t sem;
    atomic_t event_irq;
};

struct sample_module_data {
    char name[20];
    struct rtdm_device device;

	struct controller_data controller;
};

int OpenController(struct controller_data *controller);
int CloseController(struct controller_data *controller);

unsigned char  ReadByteController(struct controller_data *controller, unsigned address);
unsigned short ReadWordController(struct controller_data *controller, unsigned address);
unsigned int ReadDWordController(struct controller_data *controller, unsigned address);

void WriteByteController(struct controller_data *controller, unsigned address, unsigned char value);
void WriteWordController(struct controller_data *controller, unsigned address, unsigned short value);
void WriteDWordController(struct controller_data *controller, unsigned address, unsigned int value);

unsigned WaitEvent(struct controller_data *controller);
