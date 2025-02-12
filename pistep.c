/*
  pistep.c - Raspberry Pi stepper signal control

  Copyright (c) 2021 Josh Silverstone
*/

#include "pistep.h"


#include <bcm2835.h>


#define S0_EN_PIN 12
#define S1_EN_PIN 4
#define S0_STEP_PIN 19
#define S1_STEP_PIN 18
#define S0_DIR_PIN 13
#define S1_DIR_PIN 24
#define LASER_TH_PIN 16
#define LASER_WP_PIN 21
#define LASER_PWM_PIN 20

static uint8_t en_pins[] = {S0_EN_PIN, S1_EN_PIN};
static uint8_t step_pins[] = {S0_STEP_PIN, S1_STEP_PIN};
static uint8_t dir_pins[] = {S0_DIR_PIN, S1_DIR_PIN};
static uint8_t laser_pins[] = {LASER_TH_PIN, LASER_WP_PIN, LASER_PWM_PIN};

volatile uint16_t pistep_laser_power;

int pistep_init()
{
	// Init the bcm2835 lib
	if (!bcm2835_init())
		return 1;
	
	
	// Set all relevant pins as outputs
	for (int stepper = 0; stepper < 2; stepper++)
	{
		bcm2835_gpio_fsel(en_pins[stepper], BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(step_pins[stepper], BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(dir_pins[stepper], BCM2835_GPIO_FSEL_OUTP);
	}
	
	for (int pin = 0; pin < 3; pin++)
		bcm2835_gpio_fsel(laser_pins[pin], BCM2835_GPIO_FSEL_OUTP);
	
	// Set up water interlock override
	bcm2835_gpio_write(LASER_WP_PIN, 0);
	
	return 0;
}


int pistep_set_state(pistep_signal_t signal, uint8_t stepper_id, uint8_t state)
{
	switch (signal)
	{
		case pistep_en:
			bcm2835_gpio_write(en_pins[stepper_id], state);
			break;
		
		case pistep_step:
			bcm2835_gpio_write(step_pins[stepper_id], state);
			break;
		
		case pistep_dir:
			bcm2835_gpio_write(dir_pins[stepper_id], state);
			break;
		
		default:
			return 1;
	}
	
	return 0;
}


int
pistep_set_laser_power(float power)
{
	if (power < PISTEP_LASER_MIN_POWER)
	{
		power = PISTEP_LASER_MIN_POWER;
	}
	else
	if (power > PISTEP_LASER_MAX_POWER)
	{
		power = PISTEP_LASER_MAX_POWER;
	}
	
	float p = (power - PISTEP_LASER_MIN_POWER)
				/ (PISTEP_LASER_MAX_POWER - PISTEP_LASER_MIN_POWER);
	
	pistep_laser_power = (uint16_t)(p * 0xFFFF);
	
	// FIXME: This should just rely on run loop PWM counter; delete me
	bcm2835_gpio_write(LASER_PWM_PIN, (pistep_laser_power > 0x8000) );
	
	return 0;
}


int
pistep_set_laser_enable(unsigned char en)
{
	en = !(!en);
	
	bcm2835_gpio_write(LASER_TH_PIN, en);
	
	return 0;
}