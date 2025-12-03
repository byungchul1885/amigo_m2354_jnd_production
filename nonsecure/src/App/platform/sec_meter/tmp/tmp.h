#ifndef TMP_H
#define TMP_H 1

#define ATE_CAL_TEMP    (22.0F) // 22C is the ATE's Vref cal. temp.


// Initialize the temperature.
void tmp_init(void);

// Deinitialize (shut down) the temperature, used for sleep.
void tmp_deinit(void);

// Read a valid STEMP, after performing a measurement.
int16_t tmp_stemp(void);

// Measure the temperature.  Actually may take the previous
// measurement, and start a new measurement.
float tmp(void);

/* busy-wait for the temperaure measurement to complete. */
bool tmp_busy_wait (void);

void tmp_start(void);
float get_inst_tmpf(void);
int16_t get_inst_stemp(void);
int8_t get_inst_temp(void);
float get_inst_tempf_for_adj(void);
void tmp_meas(void);

#define	MAX_TEMP_READ			86


#endif // undefined TMP_H
