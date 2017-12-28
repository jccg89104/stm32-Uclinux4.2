#include <common.h>
#include <clock.h>
#include <malloc.h>
#include <spi.h>
#include <asm/arch/stm32f2_gpio.h>
//#include <stdio.h>
//#include <asm/arch/lpc18xx_scu.h>
//#include <asm/arch/lpc18xx_gpio.h>
#if 0
/* Control Register bits definition */
#define SPI_CR_BITENABLE	(1 << 2)
#define SPI_CR_CPHA		(1 << 3)
#define SPI_CR_CPOL		(1 << 4)
#define SPI_CR_MSTR		(1 << 5)
#define SPI_CR_LSBF		(1 << 6)
#define SPI_CR_SPIE		(1 << 7)
#define SPI_CR_BITS(x)		((x) << 8)

/* Status Register bits definition */
#define SPI_SR_ABRT		(1 << 3)
#define SPI_SR_MODF		(1 << 4)
#define SPI_SR_ROVR		(1 << 5)
#define SPI_SR_WCOL		(1 << 6)
#define SPI_SR_SPIF		(1 << 7)

/* SPI Clock Counter Register */
#define SPI_CCR_COUNTER(x)	((x) << 0)

/*
 * SPI registers
 */
struct lpc_spi {
	u32 cr;
	u32 sr;
	u32 dr;
	u32 ccr;
	u32 tcr;
	u32 tsr;
	u32 reserved;
	u32 interrupt;
};
#define LPC_SPI_BASE 0x40100000
static volatile struct lpc_spi *lpc_spi = (struct lpc_spi *)LPC_SPI_BASE;
#endif

#define SPI_SR_TXE_RXNE			0x03
#define STM32_RCC_ENR_SPI1		(1 << 12)	/* SPI1 module clock  */

/* SPI pins configuration */
static const struct stm32f2_gpio_dsc spi1_dsc[] = {
			// CS
			{STM32F2_GPIO_PORT_A, STM32F2_GPIO_PIN_15},
			// SCK
			{STM32F2_GPIO_PORT_B, STM32F2_GPIO_PIN_3},
			// MOSI
			{STM32F2_GPIO_PORT_B, STM32F2_GPIO_PIN_5},
			// MISO
			{STM32F2_GPIO_PORT_B, STM32F2_GPIO_PIN_4}
};


//static const struct lpc18xx_iomux_dsc lpc_cs_gpio = CONFIG_LPC_CS_GPIO;

/*
 * Private data structure for an SPI slave
 */
struct stm_spi_slave {
	struct spi_slave	slave;		/* Generic slave */
	u32			spccr;		/* SPI bus rate */
	u32			mode;		/* SPI bus mode */
};

/*
 * Handler to get access to the driver specific slave data structure
 * @param c		generic slave
 * @returns		driver specific slave
 */
static inline struct stm_spi_slave *to_stm_spi(struct spi_slave *slave)
{
	return container_of(slave, struct stm_spi_slave, slave);
}

/*
 * Initialization of the entire driver
 */
void spi_init()
{
	/* Configure PINs */
	stm32f2_gpio_config(&spi1_dsc[0], STM32F2_GPIO_ROLE_GPOUT);
	stm32f2_gpio_config(&spi1_dsc[1], STM32F2_GPIO_ROLE_SPI);
	stm32f2_gpio_config(&spi1_dsc[2], STM32F2_GPIO_ROLE_SPI);
	stm32f2_gpio_config(&spi1_dsc[3], STM32F2_GPIO_ROLE_SPI);


	/* Configure Chip Select GPIO */
	stm32f2_gpout_set(&spi1_dsc[0], 1);
	
	/*
	 * Enable SPI1 interface clock
	 */
	STM32_RCC->apb2enr |= STM32_RCC_ENR_SPI1;

	/* The SPI operates in Master mode. */
	STM32_SPI1->cr1 = 0x00000054;	// SPI1 enable, fPCLK/8, Master enable
	//printf("spi_init\n");
}

/*
 * Prepare to use an SPI slave
 * @param b		SPI controller
 * @param cs		slave Chip Select
 * @param hz		max freq this slave can run at
 * @param m		slave access mode
 * @returns		driver specific slave
 */
struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int hz, unsigned int mode)
{
	struct stm_spi_slave *s;
	struct spi_slave *slave = NULL;
	unsigned int spccr;

	if (bus != 0 || cs != 0) {
		goto done;
	}

	//spccr = (clock_get(CLOCK_SPI) / hz) & 0xfe;
	//if (spccr < 8) {
	//	goto done;
	//}

	s = malloc(sizeof(struct stm_spi_slave));
	if (!s) {
		goto done;
	}

	s->spccr = spccr;
	s->mode = mode;

	slave = &s->slave;

	slave->bus = bus;
	slave->cs = cs;

 done:
 //printf("spi_slave\n");
	return slave;
}

/*
 * Done with an an SPI slave
 * @param slv		SPI slave
 */
void spi_free_slave(struct spi_slave *slv)
{
	struct stm_spi_slave *s = to_stm_spi(slv);
	free(s);
}

/*
 * Set up the SPI controller
 * @param slv		SPI slave
 * @returns		0->success; !0->failure
 */
int spi_claim_bus(struct spi_slave *slv)
{
	struct stm_spi_slave *s = to_stm_spi(slv);

	/* set speed */
	//lpc_spi->ccr = s->spccr;

	/* set mode */
#if 0
	if (s->mode & SPI_CPHA) {
		lpc_spi->cr |= SPI_CR_CPHA;
	} else {
		lpc_spi->cr &= ~SPI_CR_CPHA;
	}
	if (s->mode & SPI_CPOL) {
		lpc_spi->cr |= SPI_CR_CPOL;
	} else {
		lpc_spi->cr &= ~SPI_CR_CPOL;
	}
#endif
//printf("spi_claim_bus\n");
	return 0;
}

/*
 * Shut down the SPI controller
 * @param slv		SPI slave
 */
void spi_release_bus(struct spi_slave *slv)
{

}

/*
 * Perform an SPI transfer
 * @param slv		SPI slave
 * @param bl		transfer length (in bits)
 * @param dout		data out
 * @param din		data in
 * @param fl		transfer flags
 * @returns		0->success; !0->failure
 */
int spi_xfer(struct spi_slave *slv, unsigned int bl,
	     const void *dout, void *din, unsigned long fl)
{
	u8 dummy = 0xff;
	int i;
	u32 sr;
	int ret = 0;
	const u8	*_dout = dout;
	u8		*_din = din;
	int len = bl >> 3;

//printf("spi_xfer\n");
	if (fl & SPI_XFER_BEGIN) {
		/* Enable chip select */
		//lpc_gpio_clear(lpc_cs_gpio);
		spi_cs_activate(slv);
		//printf("cs low\n");
	}

	for (i = 0; i < len; i++) {
		if (dout == NULL) {
			STM32_SPI1->dr = dummy;
			//printf("spi_dummy %02X\n", dummy);
		} else {
			STM32_SPI1->dr = _dout[i];
			//printf("spi_dout %02X\n", _dout[i]);
		}

		do {
			sr = STM32_SPI1->sr;
			//printf("sr: %08X\n", sr);
		}
		while ((sr & SPI_SR_TXE_RXNE) != 3);
		sr = STM32_SPI1->sr;
		//printf("sr: %08X\n", sr);
		if (din == NULL) {
			dummy = STM32_SPI1->dr;
		} else {
			_din[i] = STM32_SPI1->dr;
			//printf("spi_din %02X\n", _din[i]);
		}
		sr = STM32_SPI1->sr;
	}

	if (fl & SPI_XFER_END) {
		/* Disable chip select */
		//lpc_gpio_set(lpc_cs_gpio);
		spi_cs_deactivate(slv);
		//printf("cs high\n");
	}

	return ret;
}
