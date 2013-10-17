/*
 * tool Functions
 *
 *  create by zbunix@gmail.com
 */

#include <common.h>
#include <command.h>
#include <mmc.h>

static int D = 0;

#define writel(v,a)  (*(volatile unsigned int *)(a) = (v))
#define readl(a)     (*(volatile unsigned int *)(a))
#if 0
#define REG(addr)   ((volatile unsigned int *)(addr))
#define WRITE_BITS_REG(addr, startbit, width, val)   *REG(addr) = (*REG(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit))
#define READ_BITS_REG(addr, startbit, width)         ((*REG(addr) & ((1<<(width)) - 1) << (startbit)) >> (startbit))
#else
static void WRITE_BITS_REG(unsigned int addr, int startbit, int width, unsigned int val)
{
    unsigned int reg_val;

    reg_val = readl(addr);

    reg_val &= ~(((1<<width) - 1) << startbit);
    reg_val |= (val << startbit);

    writel(reg_val, addr);
}

static unsigned int READ_BITS_REG(unsigned int addr, int startbit, int width)
{
    unsigned int reg_val;

    reg_val = readl(addr);

    reg_val &= (((1<<width) - 1) << startbit);

    return (reg_val>>startbit);
}
#endif

int do_reg( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

	unsigned int reg      = 0;
    unsigned int reg_val  = 0;
    unsigned int old_val  = 0;
    unsigned int val      = 0;
    int start_bit         = 0;
    int width_bit         = 32;
    enum {FUNC_REG_READ=0, FUNC_REG_WRITE};
    int func = FUNC_REG_READ;

	if (D>0) printf ("argc=%d\n", argc);

	if(!strncmp(argv[1],"debug", 5)){
        printf("%s: debug\n", __FUNCTION__);
		if (argc>=3) {
            D = simple_strtoul(argv[2], NULL, 10);
            printf("set: D=%d\n", D);
        }else{
            printf("get: D=%d\n", D);
        }

        return 0;
    }

	reg       = simple_strtoul(argv[1], NULL, 16);
	start_bit = simple_strtoul(argv[2], NULL, 10);
	width_bit = simple_strtoul(argv[3], NULL, 10);
	if (argc>=5) {
		func = FUNC_REG_WRITE;
		val = simple_strtoul(argv[4], NULL, 16);
	}

	if (FUNC_REG_READ==func) {
            reg_val = readl(reg);
            old_val = reg_val;
            old_val &= (((1<<width_bit) - 1) << start_bit);
            val = (old_val>>start_bit);
            printf("R [0x%08x %02d,%02d]=0x%x  (0x%08x)\n", reg,start_bit,width_bit,val,reg_val);
    }else{
            reg_val = readl(reg);
            reg_val &= ~(((1<<width_bit) - 1) << start_bit);
            reg_val |= (val << start_bit);
            writel(reg_val, reg);
            printf("W [0x%08x %02d,%02d]=0x%x  (0x%08x)\n", reg,start_bit,width_bit,val,reg_val);       
	}

	return 0;
}

U_BOOT_CMD(
	reg,	5,	0,	do_reg,
	"reg  - reg r/w\n",
	"reg  <reg> <start_bit> <width_bit> <val>	 -- write\n"
	"reg  <reg> <start_bit> <width_bit> 	     -- read\n"
);
/**********************************************************************/

struct s5p_gpio_bank {
        unsigned int    con;
        unsigned int    dat;
        unsigned int    pull;
        unsigned int    drv;
        //unsigned int    pdn_con;
        //unsigned int    pdn_pull;
        //unsigned char   res1[8];
};

struct _gpio_bank {
    char	      name[8];
	unsigned int  base;
	unsigned int  gpio;
};

static struct _gpio_bank gpio_bank[] = {
    /* bank a0 */
{"a0.0", 0xe0200000, 0},
{"a0.1", 0xe0200000, 1},
{"a0.2", 0xe0200000, 2},
{"a0.3", 0xe0200000, 3},
{"a0.4", 0xe0200000, 4},
{"a0.5", 0xe0200000, 5},
{"a0.6", 0xe0200000, 6},
{"a0.7", 0xe0200000, 7},

    /* bank a1 */
{"a1.0", 0xe0200020, 0},
{"a1.1", 0xe0200020, 1},
{"a1.2", 0xe0200020, 2},
{"a1.3", 0xe0200020, 3},

    /* bank b */
{"b.0", 0xe0200040, 0},
{"b.1", 0xe0200040, 1},
{"b.2", 0xe0200040, 2},
{"b.3", 0xe0200040, 3},
{"b.4", 0xe0200040, 4},
{"b.5", 0xe0200040, 5},
{"b.6", 0xe0200040, 6},
{"b.7", 0xe0200040, 7},

    /* bank c0 */
{"c0.0", 0xe0200060, 0},
{"c0.1", 0xe0200060, 1},
{"c0.2", 0xe0200060, 2},
{"c0.3", 0xe0200060, 3},
{"c0.4", 0xe0200060, 4},

    /* bank c1 */
{"c1.0", 0xe0200080, 0},
{"c1.1", 0xe0200080, 1},
{"c1.2", 0xe0200080, 2},
{"c1.3", 0xe0200080, 3},
{"c1.4", 0xe0200080, 4},


    /* bank d0 */
{"d0.0", 0xe02000a0, 0},
{"d0.1", 0xe02000a0, 1},
{"d0.2", 0xe02000a0, 2},
{"d0.3", 0xe02000a0, 3},


    /* bank d1 */
{"d1.0", 0xe02000c0, 0},
{"d1.1", 0xe02000c0, 1},
{"d1.2", 0xe02000c0, 2},
{"d1.3", 0xe02000c0, 3},
{"d1.4", 0xe02000c0, 4},
{"d1.5", 0xe02000c0, 5},

    /* bank e0 */
{"e0.0", 0xe02000e0, 0},
{"e0.1", 0xe02000e0, 1},
{"e0.2", 0xe02000e0, 2},
{"e0.3", 0xe02000e0, 3},
{"e0.4", 0xe02000e0, 4},
{"e0.5", 0xe02000e0, 5},
{"e0.6", 0xe02000e0, 6},
{"e0.7", 0xe02000e0, 7},

    /* bank e1 */
{"e1.0", 0xe0200100, 0},
{"e1.1", 0xe0200100, 1},
{"e1.2", 0xe0200100, 2},
{"e1.3", 0xe0200100, 3},
{"e1.4", 0xe0200100, 4},

    /* bank f0 */
{"f0.0", 0xe0200120, 0},
{"f0.1", 0xe0200120, 1},
{"f0.2", 0xe0200120, 2},
{"f0.3", 0xe0200120, 3},
{"f0.4", 0xe0200120, 4},
{"f0.5", 0xe0200120, 5},
{"f0.6", 0xe0200120, 6},
{"f0.7", 0xe0200120, 7},

    /* bank f1 */
{"f1.0", 0xe0200140, 0},
{"f1.1", 0xe0200140, 1},
{"f1.2", 0xe0200140, 2},
{"f1.3", 0xe0200140, 3},
{"f1.4", 0xe0200140, 4},
{"f1.5", 0xe0200140, 5},
{"f1.6", 0xe0200140, 6},
{"f1.7", 0xe0200140, 7},

    /* bank f2 */
{"f2.0", 0xe0200160, 0},
{"f2.1", 0xe0200160, 1},
{"f2.2", 0xe0200160, 2},
{"f2.3", 0xe0200160, 3},
{"f2.4", 0xe0200160, 4},
{"f2.5", 0xe0200160, 5},
{"f2.6", 0xe0200160, 6},
{"f2.7", 0xe0200160, 7},


    /* bank f3 */
{"f3.0", 0xe0200180, 0},
{"f3.1", 0xe0200180, 1},
{"f3.2", 0xe0200180, 2},
{"f3.3", 0xe0200180, 3},
{"f3.4", 0xe0200180, 4},
{"f3.5", 0xe0200180, 5},

    /* bank g0 */
{"g0.0", 0xe02001a0, 0},
{"g0.1", 0xe02001a0, 1},
{"g0.2", 0xe02001a0, 2},
{"g0.3", 0xe02001a0, 3},
{"g0.4", 0xe02001a0, 4},
{"g0.5", 0xe02001a0, 5},
{"g0.6", 0xe02001a0, 6},

    /* bank g1 */
{"g1.0", 0xe02001c0, 0},
{"g1.1", 0xe02001c0, 1},
{"g1.2", 0xe02001c0, 2},
{"g1.3", 0xe02001c0, 3},
{"g1.4", 0xe02001c0, 4},
{"g1.5", 0xe02001c0, 5},
{"g1.6", 0xe02001c0, 6},

    /* bank g2 */
{"g2.0", 0xe02001e0, 0},
{"g2.1", 0xe02001e0, 1},
{"g2.2", 0xe02001e0, 2},
{"g2.3", 0xe02001e0, 3},
{"g2.4", 0xe02001e0, 4},
{"g2.5", 0xe02001e0, 5},
{"g2.6", 0xe02001e0, 6},

    /* bank g3 */
{"g3.0", 0xe0200200, 0},
{"g3.1", 0xe0200200, 1},
{"g3.2", 0xe0200200, 2},
{"g3.3", 0xe0200200, 3},
{"g3.4", 0xe0200200, 4},
{"g3.5", 0xe0200200, 5},
{"g3.6", 0xe0200200, 6},

    /* bank i */
{"i.0", 0xe0200220, 0},
{"i.1", 0xe0200220, 1},
{"i.2", 0xe0200220, 2},
{"i.3", 0xe0200220, 3},
{"i.4", 0xe0200220, 4},
{"i.5", 0xe0200220, 5},
{"i.6", 0xe0200220, 6},


    /* bank j0 */
{"j0.0", 0xe0200240, 0},
{"j0.1", 0xe0200240, 1},
{"j0.2", 0xe0200240, 2},
{"j0.3", 0xe0200240, 3},
{"j0.4", 0xe0200240, 4},
{"j0.5", 0xe0200240, 5},
{"j0.6", 0xe0200240, 6},
{"j0.7", 0xe0200240, 7},

    /* bank j1 */
{"j1.0", 0xe0200260, 0},
{"j1.1", 0xe0200260, 1},
{"j1.2", 0xe0200260, 2},
{"j1.3", 0xe0200260, 3},
{"j1.4", 0xe0200260, 4},
{"j1.5", 0xe0200260, 5},


    /* bank j2 */
{"j2.0", 0xe0200280, 0},
{"j2.1", 0xe0200280, 1},
{"j2.2", 0xe0200280, 2},
{"j2.3", 0xe0200280, 3},
{"j2.4", 0xe0200280, 4},
{"j2.5", 0xe0200280, 5},
{"j2.6", 0xe0200280, 6},
{"j2.7", 0xe0200280, 7},

    /* bank j3 */
{"j3.0", 0xe02002a0, 0},
{"j3.1", 0xe02002a0, 1},
{"j3.2", 0xe02002a0, 2},
{"j3.3", 0xe02002a0, 3},
{"j3.4", 0xe02002a0, 4},
{"j3.5", 0xe02002a0, 5},
{"j3.6", 0xe02002a0, 6},
{"j3.7", 0xe02002a0, 7},


    /* bank j4 */
{"j4.0", 0xe02002c0, 0},
{"j4.1", 0xe02002c0, 1},
{"j4.2", 0xe02002c0, 2},
{"j4.3", 0xe02002c0, 3},
{"j4.4", 0xe02002c0, 4},

    /* bank h0 */
{"h0.0", 0xe0200c00, 0},
{"h0.1", 0xe0200c00, 1},
{"h0.2", 0xe0200c00, 2},
{"h0.3", 0xe0200c00, 3},
{"h0.4", 0xe0200c00, 4},
{"h0.5", 0xe0200c00, 5},
{"h0.6", 0xe0200c00, 6},
{"h0.7", 0xe0200c00, 7},

    /* bank h1 */
{"h1.0", 0xe0200c20, 0},
{"h1.1", 0xe0200c20, 1},
{"h1.2", 0xe0200c20, 2},
{"h1.3", 0xe0200c20, 3},
{"h1.4", 0xe0200c20, 4},
{"h1.5", 0xe0200c20, 5},
{"h1.6", 0xe0200c20, 6},
{"h1.7", 0xe0200c20, 7},

    /* bank h2 */
{"h2.0", 0xe0200c40, 0},
{"h2.1", 0xe0200c40, 1},
{"h2.2", 0xe0200c40, 2},
{"h2.3", 0xe0200c40, 3},
{"h2.4", 0xe0200c40, 4},
{"h2.5", 0xe0200c40, 5},
{"h2.6", 0xe0200c40, 6},
{"h2.7", 0xe0200c40, 7},

    /* bank h3 */
{"h3.0", 0xe0200c60, 0},
{"h3.1", 0xe0200c60, 1},
{"h3.2", 0xe0200c60, 2},
{"h3.3", 0xe0200c60, 3},
{"h3.4", 0xe0200c60, 4},
{"h3.5", 0xe0200c60, 5},
{"h3.6", 0xe0200c60, 6},
{"h3.7", 0xe0200c60, 7},
};

#define MAX_BANK  sizeof(gpio_bank)/sizeof(gpio_bank[0])

static int find_bank(char *name)
{
    int i = 0;

    for (i=0; i<MAX_BANK; i++) {
        if (D>0) printf("strcmp: %s  %s(%s)\n", name, gpio_bank[i].name, __FUNCTION__);
        if (!strcmp(name, gpio_bank[i].name))
            return i;
    }

    return -1;
}

/* Pin configurations */
#define GPIO_INPUT	0x0
#define GPIO_OUTPUT	0x1
#define GPIO_IRQ	0xf
#define GPIO_FUNC(x)	(x)

/* Pull mode */
#define GPIO_PULL_NONE	0x0
#define GPIO_PULL_DOWN	0x1
#define GPIO_PULL_UP	0x2

/* Drive Strength level */
#define GPIO_DRV_1X	0x0
#define GPIO_DRV_2X	0x1
#define GPIO_DRV_3X	0x2
#define GPIO_DRV_4X	0x3
#define GPIO_DRV_FAST	0x0
#define GPIO_DRV_SLOW	0x1

#define CON_MASK(x)		(0xf << ((x) << 2))
#define CON_SFR(x, v)		((v) << ((x) << 2))

#define DAT_MASK(x)		(0x1 << (x))
#define DAT_SET(x)		(0x1 << (x))

#define PULL_MASK(x)		(0x3 << ((x) << 1))
#define PULL_MODE(x, v)		((v) << ((x) << 1))

#define DRV_MASK(x)		(0x3 << ((x) << 1))
#define DRV_SET(x, m)		((m) << ((x) << 1))
#define RATE_MASK(x)		(0x1 << (x + 16))
#define RATE_SET(x)		(0x1 << (x + 16))



void gpio_set_cfg(char *name, int cfg)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    WRITE_BITS_REG(&bank->con, gpio*2, 4, cfg);
}

int gpio_get_cfg(char *name)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    return READ_BITS_REG(&bank->con, gpio*2, 4);
}

void gpio_direction_output(char *name)
{
	gpio_set_cfg(name, GPIO_OUTPUT);
}

void gpio_direction_input(char *name)
{
	gpio_set_cfg(name, GPIO_INPUT);
}

int gpio_get_direction(char *name)
{
    gpio_get_cfg(name);
}

void gpio_set_value(char *name, int set)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    WRITE_BITS_REG(&bank->dat, gpio<<0, 1, set);
}

int gpio_get_value(char *name)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    return READ_BITS_REG(&bank->dat, gpio<<0, 1);
}

void gpio_set_pull(char *name, int set)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    WRITE_BITS_REG(&bank->pull, gpio<<1, 2, set);
}

int gpio_get_pull(char *name)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    return READ_BITS_REG(&bank->pull, gpio<<1, 2);
}

int gpio_get_drv(char *name)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    return READ_BITS_REG(&bank->drv, gpio<<1, 2);
}

void gpio_set_drv(char *name, int set)
{
    int index;
    int gpio;
    struct s5p_gpio_bank * bank;

    index = find_bank(name);
    gpio  = gpio_bank[index].gpio;
    bank   = (struct s5p_gpio_bank *)(gpio_bank[index].base);

    WRITE_BITS_REG(&bank->drv, gpio<<1, 2, set);
}

static int do_gpio( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char *name = NULL;
    int level  = 0;
    int cfg    = GPIO_OUTPUT;
    int pull   = GPIO_PULL_UP;
    int drv    = GPIO_DRV_2X;
    int direct = GPIO_OUTPUT;
    enum {FUNC_GPIO_INFO=0, FUNC_GPIO_SET,FUNC_GPIO_GET};
    int func = FUNC_GPIO_GET;

        
    if (argc>=2) {
        if (2==argc) {
            func = FUNC_GPIO_GET;
            goto end_gpiorw;   
        }
    }


    if (argc>=3) {
        /*********level/info***********************/
        if(!strncmp(argv[2],"info", 4)){
            func = FUNC_GPIO_INFO;
            goto end_gpiorw;        
        }

        level = simple_strtoul(argv[2], NULL, 10);
        if (D>=1) printf("%s: level=%02d\n", __FUNCTION__, level);
        func = FUNC_GPIO_SET;
    }


    if (argc>=4) {
        /*********cfg**********************/
        cfg = simple_strtoul(argv[3], NULL, 10);
        if (D>=1) printf("%s: cfg=%02d\n", __FUNCTION__, cfg);
    }

    if (argc>=5) {
        direct = simple_strtoul(argv[4], NULL, 10);
        if (D>=1) printf("%s: direct=0x%08x\n", __FUNCTION__, direct);
    }

    if (argc>=6) {
        /*********pull**********************/
        pull = simple_strtoul(argv[5], NULL, 10);
        if (D>=1) printf("%s: pull=0x%08x\n", __FUNCTION__, pull);
    }

    if (argc>=7) {
        drv = simple_strtoul(argv[6], NULL, 10);
        if (D>=1) printf("%s: drv=0x%02x\n", __FUNCTION__, drv);
    }

 end_gpiorw:
        name = argv[1];

        if (FUNC_GPIO_INFO==func) {
            cfg        = gpio_get_cfg(name);
            pull       = gpio_get_pull(name);
            direct     = gpio_get_direction(name);
            level      = gpio_get_value(name);
            printf("info gpio=%s  cfg=%d  pull=%d direct=%d level=%d\n", argv[1], cfg,pull,direct, level);
        }

        if (FUNC_GPIO_SET==func) {
            printf("FUNC_GPIO_SET: gpio=%s level=%d cfg=%d pull=%d direct=%d\n", argv[1],level,cfg,pull,direct);
            gpio_set_cfg(name, cfg);
            gpio_set_pull(name, pull);
            //gpio_set_direction(name, direct); 
            gpio_set_drv(name, drv);
            gpio_set_value(name, level);
            printf("set gpio=%s  level=%d\n", name,level);  
        }
        
         if (FUNC_GPIO_GET==func) {
            gpio_set_cfg(name, GPIO_INPUT);
            gpio_set_pull(name, GPIO_PULL_UP);
            gpio_set_drv(name, GPIO_DRV_2X);
            //gpio_set_direction(name, GPIO_INPUT);
            udelay(10);
            level = gpio_get_value(name);
            printf("get gpio=%s  level=%d\n", name, level);
        }


    return 0;
}


U_BOOT_CMD(
	gpio,	7,	0,	do_gpio,
	"gpio  - gpio test\n",
	"gpio  <gpio_index>  <info>	                              -- show gpio info\n"
	"gpio  <gpio_index> 	                                  -- get gpio level\n"
    "gpio  <gpio_index>  <level> [cfg] [direct] [pull] [drv]  -- set gpio level\n"
);


#if 1
static int ack0, ack1, ack2, ack3;
#define DELAY(us)            time_delay_us(us)  
#define SCL             (1 << 0)     
#define SDA             (1 << 1)  
#define GPD1CON    	(*(volatile unsigned *)0xE02000C0)
#define GPD1DAT    	(*(volatile unsigned *)0xE02000C4)
#define GPD1PUD    	(*(volatile unsigned *)0xE02000C8) 

static void set_scl_direct(int output)
{
    if (output)
        GPD1CON = (GPD1CON & ~(0xf<<12))|(0x1<<12);
    else
        GPD1CON &= ~(0xf<<12);
}
  
static void set_scl_level(int level)
{
    if (level)
        GPD1DAT |= (0x1<<3);
    else
        GPD1DAT &= ~(0x1<<3);
}  

static void set_sda_direct(int output)
{
    if (output)
        GPD1CON = (GPD1CON & ~(0xf<<8))|(0x1<<8);
    else
        GPD1CON &= ~(0xf<<8);
}
  
static void set_sda_level(int level)
{
    if (level)
        GPD1DAT |= (0x1<<2);
    else
        GPD1DAT &= ~(0x1<<2);
}  

static int get_sda_level(void)
{
    return ((GPD1DAT & (0x1<<2))>>2);
} 

/*  
 * I2C by GPIO simulated  clear 0 routine. 
 * 
 * @param whichline: GPIO control line 
 * 
 */  
static void i2c_clr(unsigned char whichline)  
{  
    //unsigned char regvalue;  
      
    if(whichline == SCL)  
    {  
        set_scl_direct(1);  
        set_scl_level(0);

        return;  
    }  
    else if(whichline == SDA)  
    {  
        set_sda_direct(1);  
        set_sda_level(0);

        return;  
    }  
    else if(whichline == (SDA|SCL))  
    {  
        set_scl_direct(1); 
        set_scl_level(0); 

        set_sda_direct(1);  
        set_sda_level(0);

        return;  
    }  
    else  
    {  
        printf("Error input.\n"); 
         
        return;  
    }  
      
}  
  
/*  
 * I2C by GPIO simulated  set 1 routine. 
 * 
 * @param whichline: GPIO control line 
 * 
 */  
static void  i2c_set(unsigned char whichline)  
{  
    //unsigned char regvalue;  
      
    if(whichline == SCL)  
    {  
        set_scl_direct(1);  
        set_scl_level(1);

        return;  
    }  
    else if(whichline == SDA)  
    {  
        set_sda_direct(1);  
        set_sda_level(1);

        return;  
    }  
    else if(whichline == (SDA|SCL))  
    {  
        set_scl_direct(1); 
        set_scl_level(1); 

        set_sda_direct(1);  
        set_sda_level(1);

        return;  
    }  
    else  
    {  
        printf("Error input.\n");  
        return;  
    }  
}  
  
/* 
 *  delays for a specified number of micro seconds rountine. 
 * 
 *  @param usec: number of micro seconds to pause for 
 * 
 */  
void time_delay_us(unsigned int usec)  
{  
    int i,j;  
      
    for(i=0;i<usec * 5;i++)  
    {  
        for(j=0;j<47;j++)  
        {;}  
    }  
}  
  
/*  
 * I2C by GPIO simulated  read data routine. 
 * 
 * @return value: a bit for read  
 * 
 */  
 static unsigned char i2c_data_read(void)  
{  
    unsigned char level;  
      
    set_sda_direct(0);
    DELAY(1);  
          
    level = get_sda_level();

    return level;
}  
  
/* 
 * sends a start bit via I2C rountine
 * 
 */  
static void i2c_start_bit(void)  
{  
        DELAY(1);  
        i2c_set(SDA | SCL);  
        DELAY(1);  
        i2c_clr(SDA);  
        DELAY(2);  
}  
  
/* 
 * sends a stop bit via I2C rountine. 
 * 
 */  
static void i2c_stop_bit(void)  
{  
        /* clock the ack */  
        DELAY(1);  
        i2c_set(SCL);  
        DELAY(1);   
        i2c_clr(SCL);    
  
        /* actual stop bit */  
        DELAY(1);  
        i2c_clr(SDA);  
        DELAY(1);  
        i2c_set(SCL);  
        DELAY(1);  
        i2c_set(SDA);  
        DELAY(1);  
}  
  
/* 
 * sends a character over I2C rountine. 
 * deng@sz-sentry.com
 * @param  c: character to send 
 * 
 */  
static void i2c_send_byte(unsigned char c)  
{  
    int i;  

    for (i=0; i<8; i++)  
    {  
        DELAY(1);  
        i2c_clr(SCL);  
        DELAY(1);  
  
        if (c & (1<<(7-i)))  
            i2c_set(SDA);  
        else  
            i2c_clr(SDA);  
  
        DELAY(1);  
        i2c_set(SCL);  
        DELAY(1);  
        i2c_clr(SCL);  
    }  
    DELAY(1);
}  
  
/*  receives a character from I2C rountine. 
 * 
 *  @return value: character received 
 * 
 */  
static unsigned char i2c_receive_byte(void)  
{  
    int j=0;  
    int i;  
    //unsigned char regvalue;  
  
    for (i=0; i<8; i++)  
    {  
        DELAY(1);  
        i2c_clr(SCL);  
        DELAY(2);  
        i2c_set(SCL);  
          
        set_sda_direct(0);
        DELAY(1);  
          
        if (i2c_data_read())  
            j+=(1<<(7-i));  
  
        DELAY(1);  
        i2c_clr(SCL);  
    }  
    DELAY(1);  
  
    return j;  
}  
  
/*  receives an acknowledge from I2C rountine. 
 * 
 *  @return value: 0--Ack received; 1--Nack received 
 *           
 */  
static int i2c_receive_ack(void)  
{  
    int nack = -1;  
      
    DELAY(1);  
    set_sda_direct(0);
      
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
    i2c_set(SCL);  
    DELAY(1);  
 
    nack = i2c_data_read();  
  
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  

    return nack;
}  
  
/*  
 * sends an acknowledge over I2C rountine. 
 * 
 */  
static void i2c_send_ack(void)  
{  
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
    i2c_set(SDA);  
    DELAY(1);  
    i2c_set(SCL);  
    DELAY(1);  
    i2c_clr(SCL);  
    DELAY(1);  
    i2c_clr(SDA);  
    DELAY(1);  
}  
  
static int gpio_i2c_detect(unsigned char addr_8bit)  
{  
    int ack = -1;  
      
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(addr_8bit));  
    ack = i2c_receive_ack();  
    i2c_stop_bit();  
  
    return ack;
} 

/*   
 *  read data from the I2C bus by GPIO simulated of a device rountine. 
 * 
 *  @param  devaddress:  address of the device 
 *  @param  address: address of register within device 
 *    
 *  @return value: data from the device readed 
 *  
 */  
unsigned char gpio_i2c_read(unsigned char addr_8bit, unsigned char reg)  
{  
    int data;  
      
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(addr_8bit));  
    ack0 = i2c_receive_ack();  
    i2c_send_byte(reg);  
    ack1 = i2c_receive_ack();     
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(addr_8bit) | 1);  
    ack2 = i2c_receive_ack();  
    data = i2c_receive_byte();  
    i2c_send_ack();  
    i2c_stop_bit();  
  
    return data;  
}  
  
/* 
 *  writes data to a device on the I2C bus rountine.  
 * 
 *  @param  devaddress:  address of the device 
 *  @param  address: address of register within device 
 *  @param  data:   data for write to device 
 * 
 */  
void gpio_i2c_write(unsigned char addr_8bit, unsigned char reg, unsigned char data)  
{
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(addr_8bit));  
    i2c_receive_ack();  
    i2c_send_byte(reg);  
    i2c_receive_ack();  
    i2c_send_byte(data);   
    i2c_stop_bit();  
}  
  
  
/*   
 *  read data from the I2C bus by GPIO simulated of a digital camera device rountine. 
 * 
 *  @param  devaddress:  address of the device 
 *  @param  address: address of register within device 
 *   
 */  
 /*
unsigned char gpio_sccb_read(unsigned char addr_8bit, unsigned char reg)  
{  
    int data;  
  
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(addr_8bit));  
    i2c_receive_ack();  
    i2c_send_byte(reg);  
    i2c_receive_ack();  
    i2c_stop_bit();  
    i2c_start_bit();  
    i2c_send_byte((unsigned char)(addr_8bit) | 1);  
    i2c_receive_ack();  
    data = i2c_receive_byte();  
    i2c_send_ack();  
    i2c_stop_bit();  
  
    return data;  
}  
 */
 
/* 
 * initializes I2C interface routine. 
 * 
 * @return value:0--success; 1--error. 
 * 
 */  
static int gpio_i2c_init(void)  
{  
    i2c_clr(SCL | SDA);
    
    return 0;  
}  
#endif


static int do_iic( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    enum {FUNC_IIC_ENUM=0, FUNC_IIC_DUMP, FUNC_IIC_READ,FUNC_IIC_WRITE};
    int func = FUNC_IIC_ENUM;
    int addr_7bit = 0;//0..127
    int reg       = 0;
    int val       = 0;

    gpio_i2c_init();
           
    if (1==argc) {
        func = FUNC_IIC_ENUM;
        printf("IIC_ENUM:\n");
        for(addr_7bit=0; addr_7bit<128; addr_7bit++) {
               //if (0 == iic_detect(addr_7bit) )
               if (0 == gpio_i2c_detect(addr_7bit<<1) )
                   printf("OK: found addr_7bit=0x%02x  addr_8bit=0x%02x\n", addr_7bit, addr_7bit<<1);
        }
    }

    if (2==argc) {
        func = FUNC_IIC_DUMP;
        addr_7bit = simple_strtoul(argv[1], NULL, 16);
        printf("IIC_DUMP: addr_7bit=0x%02x\n", addr_7bit);
        for(reg=0; reg<=0xff; reg++) {
               //val = iic_read(addr_7bit, reg);
               val = gpio_i2c_read(addr_7bit<<1, reg);
               printf("{0x%02x, 0x%02x},\n", reg, val&0xff );
        }

    }

    if (3==argc) {
        func = FUNC_IIC_READ;
        addr_7bit = simple_strtoul(argv[1], NULL, 16);
        reg       = simple_strtoul(argv[2], NULL, 16);
        //val = iic_read(addr_7bit, reg);
        val = gpio_i2c_read(addr_7bit<<1, reg);
        printf("IIC_READ: addr_7bit=0x%02x  reg=0x%02x  val=0x%02x  (ack0=%d ack1=%d ack2=%d)\n", addr_7bit, reg, val, ack0, ack1, ack2);
    }

    if (4==argc) {
        func = FUNC_IIC_WRITE;
        addr_7bit = simple_strtoul(argv[1], NULL, 16);
        reg       = simple_strtoul(argv[2], NULL, 16);
        val       = simple_strtoul(argv[3], NULL, 16);
        printf("IIC_WRITE: addr_7bit=0x%02x  reg=0x%02x  val=0x%02x\n", addr_7bit, reg, val);
        //iic_write(addr_7bit, reg, val);
        gpio_i2c_write(addr_7bit<<1, reg, val);
    }

    return 0;
}


U_BOOT_CMD(
	iic,	4,	0,	do_iic,
	"iic  - iic test\n",
	"iic      	                     -- show iic channel slave_addr_8bit\n"
    "iic  <addr_7bit>                -- read  addr_7bit 0..127\n"
    "iic  <addr_7bit>  <reg>         -- read  addr_7bit reg\n"
    "iic  <addr_7bit>  <reg>  <val>	 -- write addr_7bit reg value\n"
);

static int do_debug( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int val;
    int on = 0;

    if (0==strncmp(argv[1], "uboot", 5) ) {
        val = run_command("sdfuse flash bootloader u-boot.bin", 0);
        printf("cmd sdfuse:  ret=%d\n", val);
        return 0;
    }

    if (0==strncmp(argv[1], "udelay", 6) ) {
        val = simple_strtoul(argv[2], NULL, 10);
        printf("pin d1.3(scl1) test:  udelay=%dus\n", val);

        gpio_set_cfg("d1.3",  GPIO_OUTPUT);
        gpio_set_pull("d1.3", GPIO_PULL_UP);
        gpio_set_drv("d1.3",  GPIO_DRV_2X);

        while (1) {
            udelay(val);
            on ^= 1;
            gpio_set_value("d1.3", on);
        }
    }

    if (0==strncmp(argv[1], "mdelay", 6) ) {
        val = simple_strtoul(argv[2], NULL, 10);
        printf("pin d1.3(scl1) test:  mdelay=%dms\n", val);

        gpio_set_cfg("d1.3",  GPIO_OUTPUT);
        gpio_set_pull("d1.3", GPIO_PULL_UP);
        gpio_set_drv("d1.3",  GPIO_DRV_2X);

        while (1) {
            udelay(val*1000);
            on ^= 1;
            gpio_set_value("d1.3", on);
        }
    }

    return 0;
}

U_BOOT_CMD(
	debug,	3,	0,	do_debug,
	"debug  - debug test\n",
    "debug  <uboot>         \n"
    "debug  <udelay>  <val> \n"
);
