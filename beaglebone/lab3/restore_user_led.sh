#!/bin/bash
# restore_user_leds.sh
# Shell program to restore user LEDs

# Stop user LED0
cd /sys/class/leds
cd beaglebone\:green\:usr0
echo heartbeat > trigger
echo "Restore user LED0"

# Restore user LED1
cd /sys/class/leds
cd beaglebone\:green\:usr1
echo mmc0 > trigger
echo "Restore user LED1"

# Restore user LED2
cd /sys/class/leds
cd beaglebone\:green\:usr2
echo cpu0 > trigger
echo "Restore user LED2"

# Restore user LED3
cd /sys/class/leds
cd beaglebone\:green\:usr3
echo mmc1 > trigger
echo "Restore user LED3" 
