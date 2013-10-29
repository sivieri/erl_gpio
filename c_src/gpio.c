#include <wiringPi.h>
#include <softPwm.h>
#include <erl_nif.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#define PINS 17
#define HW_PWM 1
#define HW_PWM_RANGE 1024
#define SOFT_PWM_RANGE 100

typedef struct {
  ErlNifEnv *env;
  int pin_pwm_enabled[PINS];
} state_t;

extern int errno;

static state_t state;

static int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static ERL_NIF_TERM setup_0(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
	int error, i;

	error = wiringPiSetup();
	if (error != 0) {
	    error = errno;
	    return enif_make_tuple2(env, enif_make_atom(env, "error"), enif_make_int(env, error));
	}
	state.env = env;
	for (i = 0; i < PINS; ++i) {
	    state.pin_pwm_enabled[i] = 0;
	}
	state.pin_pwm_enabled[HW_PWM] = 1;
	
	return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM pin_mode_2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    int pin, mode;
    
    if (argc != 2 || !enif_is_number(env, argv[0]) || !enif_is_number(env, argv[1])) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[0], &pin)) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[1], &mode)) {
        return enif_make_badarg(env);
    }
    if (pin < 0 || pin > PINS - 1) {
        return enif_make_badarg(env);
    }
    switch (mode) {
        case INPUT:
            pinMode(pin, INPUT);
            break;
        case OUTPUT:
            pinMode(pin, OUTPUT);
            break;
        case PWM_OUTPUT:
            pinMode(pin, PWM_OUTPUT);
            break;
        default:
            enif_make_badarg(env);
            break;
    }
    
    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM digital_write_2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    int pin, value;
    
    if (argc != 2 || !enif_is_number(env, argv[0]) || !enif_is_number(env, argv[1])) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[0], &pin)) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[1], &value)) {
        return enif_make_badarg(env);
    }
    if (pin < 0 || pin > PINS - 1) {
        return enif_make_badarg(env);
    }
    switch (value) {
        case LOW:
            digitalWrite(pin, LOW);
            break;
        case HIGH:
            digitalWrite(pin, HIGH);
            break;
        default:
            enif_make_badarg(env);
            break;
    }
    
    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM analog_write_2(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    int pin, value, error;
    
    if (argc != 2 || !enif_is_number(env, argv[0]) || !enif_is_number(env, argv[1])) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[0], &pin)) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[1], &value)) {
        return enif_make_badarg(env);
    }
    if (pin < 0 || pin > PINS - 1) {
        return enif_make_badarg(env);
    }
    if (value < 0 || value > HW_PWM_RANGE) {
        return enif_make_badarg(env);
    }
    if (pin == HW_PWM) {
        pwmWrite(pin, value);
    }
    else {
        if (state.pin_pwm_enabled[pin] == 0) {
            error = softPwmCreate(pin, 0, SOFT_PWM_RANGE);
            if (error != 0) {
                error = errno;
                return enif_make_tuple2(env, enif_make_atom(env, "error"), enif_make_int(env, error));
            }
            state.pin_pwm_enabled[pin] = 1;
        }
        value = map(value, 0, HW_PWM_RANGE, 0, SOFT_PWM_RANGE);
        softPwmWrite(pin, value);
    }
    
    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM digital_read_1(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    int pin, res;
    
    if (argc != 1 || !enif_is_number(env, argv[0])) {
        return enif_make_badarg(env);
    }
    if (!enif_get_int(env, argv[0], &pin)) {
        return enif_make_badarg(env);
    }
    if (pin < 0 || pin > PINS - 1) {
        return enif_make_badarg(env);
    }
    res = digitalRead(pin);
    
    return enif_make_int(env, res);
}

static ErlNifFunc gpio_NIFs[] = {
    {"setup", 0, &setup_0},
    {"pin_mode", 2, &pin_mode_2},
    {"digital_write", 2, &digital_write_2},
    {"digital_read", 1, &digital_read_1},
    {"analog_write", 2, &analog_write_2}
};

ERL_NIF_INIT(gpio, gpio_NIFs, NULL, NULL, NULL, NULL);
