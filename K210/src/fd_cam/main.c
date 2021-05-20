#include <dvp.h>
#include <fpioa.h>
#include <sysctl.h>
#include <syslog.h>

#include "cambus.h"
#include "gc0328.h"
static const char *TAG = "camera_init";


static void io_mux_init(void) {
    /* Init DVP IO map and function settings */
    fpioa_set_function(40, FUNC_SCCB_SDA);
    fpioa_set_function(41, FUNC_SCCB_SCLK);
    fpioa_set_function(42, FUNC_CMOS_RST);
    fpioa_set_function(43, FUNC_CMOS_VSYNC);
    fpioa_set_function(44, FUNC_CMOS_PWDN);
    fpioa_set_function(45, FUNC_CMOS_HREF);
    fpioa_set_function(46, FUNC_CMOS_XCLK);
    fpioa_set_function(47, FUNC_CMOS_PCLK);

    sysctl_set_spi0_dvp_data(1);
}

static void io_set_power(void) {
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

void camera_init() {
    io_mux_init();
    io_set_power();

    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    // lcd output
    // dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_YUV_FORMAT);
    dvp_set_image_size(320, 240);
    // DVP SCL(41) SDA(40) pin ->software i2c
    cambus_init(8, 2, 41, 40, 0, 0);

    gc0328_reset();
    gc0328_init();
}

void init_sensors() {
    // camera init
    camera_init();
    // uart init
    // wifi init


}

int main(void) {
    init_sensors();
    
    return 0;
}