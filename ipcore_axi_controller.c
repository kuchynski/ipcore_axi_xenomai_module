
#include "ipcore_axi.h"

static int irq_handler_controller(rtdm_irq_t *irq_handle)
{
	struct controller_data *controller = rtdm_irq_get_arg(irq_handle, struct controller_data);

	WriteDWordController(controller, 0x204, 0);
	atomic_set(&controller->event_irq, 1);
	rtdm_sem_up(&controller->sem);

	return RTDM_IRQ_HANDLED;
}

unsigned WaitEvent(struct controller_data *controller)
{
	unsigned status = 0;

	if(rtdm_sem_timeddown(&controller->sem, 10000000, 0) == 0) {
		if(atomic_read(&controller->event_irq) != 0) {
			atomic_set(&controller->event_irq, 0);
			status |= IO_IPCORE_IRQ_MASK_INT;
		} 
	}

	return status;
}

int OpenController(struct controller_data *controller)
{
	controller->irq_line = CONTROLLER_IP_CORE_IRQ_LINE;
	controller->address_physical = CONTROLLER_IP_CORE_MEMORY_BASE;
	controller->address_virtual = NULL;

	if(NULL == request_mem_region(controller->address_physical, CONTROLLER_IP_CORE_MEMORY_SIZE, IPCORE_DRV_NAME)) {
		rtdm_printk("module: failed to request memory region\n");
		goto error_exit_0;
	}

	controller->address_virtual = ioremap(controller->address_physical, CONTROLLER_IP_CORE_MEMORY_SIZE);
	if(controller->address_virtual == NULL) {
		rtdm_printk("module: failed to ioremap\n");
		goto error_exit_1;
	}

	rtdm_sem_init(&controller->sem, 0);
	atomic_set(&controller->event_irq, 0);

	if(controller->irq_line) {
		if(rtdm_irq_request(&controller->irq_m, controller->irq_line, irq_handler_controller, 0, IPCORE_DRV_NAME_FULL, (void *)controller)) {
			rtdm_printk("module: no access to IRQ %d\n", controller->irq_line);
			controller->irq_line = 0;
			//goto error_exit_2;
		}
	}

	rtdm_printk("module: request controller: memory %X-%X.\n",
			controller->address_physical, controller->address_physical + CONTROLLER_IP_CORE_MEMORY_SIZE - 1);
	if(controller->irq_line) {
		rtdm_printk("irq controller #%d.\n", controller->irq_line);
		rtdm_irq_enable(&controller->irq_m);
	}

	return 0;

//error_exit_2:
//	iounmap(controller->address_virtual);
error_exit_1:
	release_mem_region(controller->address_physical, CONTROLLER_IP_CORE_MEMORY_SIZE);
error_exit_0:
	return -1;
}

int CloseController(struct controller_data *controller)
{
	if(controller->irq_line)
		rtdm_irq_free(&controller->irq_m);
	iounmap(controller->address_virtual);
	release_mem_region(controller->address_physical, CONTROLLER_IP_CORE_MEMORY_SIZE);
	rtdm_sem_up(&controller->sem);

	return 0;
}

unsigned char inline ReadByteController(struct controller_data *controller, unsigned address)
{
	//rtdm_printk("r %p\n", controller->address_virtual);
	return ioread8(controller->address_virtual + address);
}

unsigned short inline ReadWordController(struct controller_data *controller, unsigned address)
{
	return ioread16(controller->address_virtual + address);
}

unsigned int inline ReadDWordController(struct controller_data *controller, unsigned address)
{
	return ioread32(controller->address_virtual + address);
}

void inline WriteByteController(struct controller_data *controller, unsigned address, unsigned char value)
{
	iowrite8(value, controller->address_virtual + address);
}

void inline WriteWordController(struct controller_data *controller, unsigned address, unsigned short value)
{
	iowrite16(value, controller->address_virtual + address);
}

void inline WriteDWordController(struct controller_data *controller, unsigned address, unsigned int value)
{
	iowrite32(value, controller->address_virtual + address);
}
