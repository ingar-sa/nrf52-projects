
#include "motor_control.h"

#define LOG_MODULE_NAME motor
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define PWM_PERIOD 20000000 // 20ms

#define ZERO_DEG 500000 // 1.5ms
#define SIXTY_DEG 1500000
#define ONE_TWENTY_DEG 2000000
#define ONE_EIGHTY_DEG 2400000

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

int 
motor_init(void)
{
    int err;
    LOG_INF("Initializing motor");

    if(!device_is_ready(pwm_led0.dev)) {
        LOG_ERR("Error: PWM device %s is not ready",
                pwm_led0.dev->name);
        return -EBUSY;
    }

    err = pwm_set_dt(&pwm_led0, PWM_PERIOD, ZERO_DEG);
    if (err) {
        LOG_ERR("Error setting PWM: %d", err);
    }

    return err;
}

int
set_motor_angle(uint8_t angle) {
    switch (angle)
    {
    case 1:
        pwm_set_dt(&pwm_led0, PWM_PERIOD, ZERO_DEG);
        break;
    case 2:
        pwm_set_dt(&pwm_led0, PWM_PERIOD, SIXTY_DEG);
        break;
    case 3:
        pwm_set_dt(&pwm_led0, PWM_PERIOD, ONE_TWENTY_DEG);
        break;
    case 4:
        pwm_set_dt(&pwm_led0, PWM_PERIOD, ONE_EIGHTY_DEG);
        break;
    default:
        LOG_ERR("Invalid choice of angle. Err: %d", angle);
        return -1;
        break;
    }

    return 0;
}