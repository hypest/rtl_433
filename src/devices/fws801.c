/* Firstline Weather station protocol
 *
 * the sensor sends 36 bits 7 times, before the first packet there is a pulse sent
 * the packets are pwm modulated
 *
 * the data is grouped in 9 nibles
 * [id0] [rid0] [rid1] [data0] [temp0] [temp1] [temp2] [humi0] [humi1]
 *
 * id0 is always 1001,9
 * rid is a random id that is generated when the sensor starts, could include battery status
 * the same batteries often generate the same id
 * data(3) is 0 the battery status, 1 ok, 0 low, first reading always say low
 * data(2) is 1 when the sensor sends a reading when pressing the button on the sensor
 * data(1,0)+1 forms the channel number that can be set by the sensor (1-3)
 * temp is 12 bit signed scaled by 10
 * humi0 is always 1100,c if no humidity sensor is available
 * humi1 is always 1100,c if no humidity sensor is available
 *
 */
#include "rtl_433.h"

static int fws801_callback(uint8_t bb[BITBUF_ROWS][BITBUF_COLS],int16_t bits_per_row[BITBUF_ROWS]) {
    int rid;

    int16_t tsign;
    int16_t tint;
    int16_t tdec;

    /* FIXME validate the received message better */
    if ((bb[0][0] == 0x00) &&
        (bb[1][0] != 0x00) &&
        (bb[1][0] == bb[2][0]) &&
        (bb[1][1] == bb[2][1]) &&
        (bb[1][2] == bb[2][2]) &&
        (bb[1][3] == bb[2][3])) {

        /* FWS801 sensor */
        tsign = (bb[1][1] & 0x80) == 0x80 ? -1 : 1;
        tint = (int16_t)(bb[1][1] & 0x7F);
        tdec = (uint16_t)(bb[1][2] & 0x0F);
        fprintf(stderr, "Sensor temperature event:\n");
        fprintf(stderr, "protocol      = FWS801, %d bits\n", bits_per_row[1]);
        fprintf(stderr, "temp          = %s%d.%d\n", (tsign < 0) ? "-" : "", tint, tdec);
        fprintf(stderr, "channel       = %d\n", (bb[1][1] & 0x03) + 1);
        fprintf(stderr, "id            = %d\n", (bb[1][0] & 0xF0) >> 4);
        rid = ((bb[1][0] & 0x0F) <<4 ) | (bb[1][1] & 0xF0) >> 4;
        fprintf(stderr, "rid           = %d\n", rid);
        fprintf(stderr, "hrid          = %02x\n", rid);

        fprintf(stderr, "%02x %02x %02x %02x %02x\n",bb[1][0],bb[1][1],bb[1][2],bb[1][3],bb[1][4]);

        if (debug_output)
            debug_callback(bb, bits_per_row);

        return 1;
    }
    return 0;
}

r_device fws801 = {
    /* .id             = */ 2,
    /* .name           = */ "Firstline FWS801 Temperature Sensor",
    /* .modulation     = */ OOK_PWM_D,
    /* .short_limit    = */ 3500/4,
    /* .long_limit     = */ 7000/4,
    /* .reset_limit    = */ 15000/4,
    /* .json_callback  = */ &fws801_callback,
};
