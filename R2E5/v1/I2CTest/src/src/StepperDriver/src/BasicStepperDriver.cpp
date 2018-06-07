/*
 * Generic Stepper Motor Driver Driver
 * Indexer mode only.

 * Copyright (C)2015-2017 Laurentiu Badea
 *
 * This file may be redistributed under the terms of the MIT license.
 * A copy of this license has been included with this distribution in the file LICENSE.
 *
 * Linear speed profile calculations based on
 * - Generating stepper-motor speed profiles in real time - David Austin, 2004
 * - Atmel AVR446: Linear speed control of stepper motor, 2006
 */
#include "BasicStepperDriver.h"

/*
 * Basic connection: only DIR, STEP are connected.
 * Microstepping controls should be hardwired.
 */
BasicStepperDriver::BasicStepperDriver(short steps, short dir_pin, short step_pin)
:motor_steps(steps), dir_pin(dir_pin), step_pin(step_pin)
{}


/*
 * Initialize pins, calculate timings etc
 */
void BasicStepperDriver::begin(short rpm, short microsteps){
    pinMode(dir_pin, OUTPUT);
    digitalWrite(dir_pin, HIGH);

    pinMode(step_pin, OUTPUT);
    digitalWrite(step_pin, LOW);

    this->rpm = rpm;
    setMicrostep(microsteps);
}

/*
 * Set stepping mode (1:microsteps)
 * Allowed ranges for BasicStepperDriver are 1:1 to 1:128
 */
short BasicStepperDriver::setMicrostep(short microsteps){
    for (short ms=1; ms <= getMaxMicrostep(); ms<<=1){
        if (microsteps == ms){
            this->microsteps = microsteps;
            break;
        }
    }
    return this->microsteps;
}

/*
 * Set speed profile - CONSTANT_SPEED, LINEAR_SPEED (accelerated)
 * accel and decel are given in [full steps/s^2]
 */
void BasicStepperDriver::setSpeedProfile(Mode mode, short accel, short decel){
    profile.mode = mode;
    profile.accel = accel;
}
void BasicStepperDriver::setSpeedProfile(struct Profile profile){
    this->profile = profile;
}

/*
 * Move the motor a given number of steps.
 * positive to move forward, negative to reverse
 */
void BasicStepperDriver::move(int steps){
    startMove(steps);
    while (nextAction());
}
/*
 * Move the motor with sub-degree precision.
 * Note that using this function even once will add 1K to your program size
 * due to inclusion of float support.
 */
void BasicStepperDriver::rotate(float deg){
    move(calcStepsForRotation(deg));
}

/*
 * Set up a new move or alter an active move (calculate and save the parameters)
 */
void BasicStepperDriver::startMove(int steps){
    //long speed;
    if (steps_remaining){
        alterMove(steps);
    } else {
        // set up new move
        dir_state = (steps >= 0) ? HIGH : LOW;
        last_action_end = 0;
        steps_remaining = abs(steps);
        step_count = 0;
        rest = 0;
        // CONSTANT_SPEED MODE ONLY:
        /*
        switch (profile.mode){
        case LINEAR_SPEED:
            // speed is in [steps/s]
            speed = rpm * motor_steps / 60;
            // how many steps from 0 to target rpm
            steps_to_cruise = speed * speed * microsteps / (2 * profile.accel);
            // how many steps are needed from target rpm to a full stop
            steps_to_brake = steps_to_cruise * profile.accel / profile.accel;
            if (steps_remaining < steps_to_cruise + steps_to_brake){
                // cannot reach max speed, will need to brake early
                steps_to_cruise = steps_remaining * profile.accel / (profile.accel + profile.accel);
                steps_to_brake = steps_remaining - steps_to_cruise;
            }
            // Initial pulse (c0) including error correction factor 0.676 [us]
            step_pulse = (1e+6)*0.676*sqrt(2.0f/(profile.accel*microsteps));
            break;
        */
        //case CONSTANT_SPEED:
        //default:
            step_pulse = STEP_PULSE(rpm, motor_steps, microsteps);
            steps_to_cruise = 0;
            steps_to_brake = 0;
        //}
    }
}
/*
 * Alter a running move by adding/removing steps
 * FIXME: This is a naive implementation and it only works well in CRUISING state
 */
void BasicStepperDriver::alterMove(int steps){
    switch (getCurrentState()){
    case ACCELERATING: // this also works but will keep the original speed target
    case CRUISING:
        if (steps >= 0){
            steps_remaining += steps;
        } else {
            steps_remaining = max(steps_to_brake, steps_remaining+steps);
        };
        break;
    case DECELERATING:
        // would need to start accelerating again -- NOT IMPLEMENTED
        break;
    case STOPPED:
        startMove(steps);
        break;
    }
}
/*
 * Brake early.
 */
void BasicStepperDriver::startBrake(void){
    switch (getCurrentState()){
    case CRUISING:  // this applies to both CONSTANT_SPEED and LINEAR_SPEED modes
      steps_remaining = steps_to_brake;
    break;
    /*
    case ACCELERATING:
      steps_remaining = step_count * profile.accel / profile.accel;
    break;*/

    default:
      break; // nothing to do if already stopped or braking
    }
}
/*
 * Stop movement immediately.
 */
void BasicStepperDriver::stop(void){
    steps_remaining = 0;
}
/*
 * Return calculated time to complete the given move
 */
long BasicStepperDriver::getTimeForMove(int steps){
    // USING CONSTANT SPEED MODE ONLY:
    /*long t;
    switch (profile.mode){
        case LINEAR_SPEED:
            startMove(steps);
            t = 2*sqrt(2 * steps_to_cruise / profile.accel) + (steps_remaining - steps_to_cruise - steps_to_brake) * STEP_PULSE(rpm, motor_steps, microsteps);
            break;
        case CONSTANT_SPEED:
        default:
            t = STEP_PULSE(rpm, motor_steps, microsteps);
    }
    return t;
    */
    return STEP_PULSE(rpm, motor_steps, microsteps);
}
/*
 * Move the motor with sub-degree precision.
 * Note that calling this function will increase program size substantially
 * due to inclusion of float support.
 */
void BasicStepperDriver::startRotate(float deg){
    startMove(calcStepsForRotation(deg));
}

/*
 * calculate the interval til the next pulse
 */
void BasicStepperDriver::calcStepPulse(void){
    if (steps_remaining <= 0){  // this should not happen, but avoids strange calculations
        return;
    }

    steps_remaining--;
    step_count++;

    /*if (profile.mode == LINEAR_SPEED){
        switch (getCurrentState()){
        case ACCELERATING:
            step_pulse = step_pulse - (2*step_pulse+rest)/(4*step_count+1);
            rest = (step_count < steps_to_cruise) ? (2*step_pulse+rest) % (4*step_count+1) : 0;
            break;

        case DECELERATING:
            step_pulse = step_pulse - (2*step_pulse+rest)/(-4*steps_remaining+1);
            rest = (2*step_pulse+rest) % (-4*steps_remaining+1);
            break;

        default:
            break; // no speed changes
        }
    }*/
}
/*
 * Yield to step control
 * Toggle step and return time until next change is needed (micros)
 */
long BasicStepperDriver::nextAction(void){
    if (steps_remaining > 0){
        delayMicros(next_action_interval, last_action_end);
        /*
         * DIR pin is sampled on rising STEP edge, so it is set first
         */
        digitalWrite(dir_pin, dir_state);
        digitalWrite(step_pin, HIGH);
        unsigned m = micros();
        long pulse = step_pulse; // save value because calcStepPulse() will overwrite it
        calcStepPulse();
        m = micros() - m;
        // We should pull HIGH for 1-2us (step_high_min)
        if (m < step_high_min){ // fast MCPU or CONSTANT_SPEED
            delayMicros(step_high_min-m);
            m = step_high_min;
        };
        digitalWrite(step_pin, LOW);
        // account for calcStepPulse() execution time; sets ceiling for max rpm on slower MCUs
        last_action_end = micros();
        next_action_interval = (pulse > m) ? pulse - m : 1;
    } else {
        // end of move
        last_action_end = 0;
        next_action_interval = 0;
    }
    return next_action_interval;
}

enum BasicStepperDriver::State BasicStepperDriver::getCurrentState(void){
    enum State state;
    if (steps_remaining <= 0){
        state = STOPPED;
    } else {
        if (steps_remaining <= steps_to_brake){
            state = DECELERATING;
        } else if (step_count <= steps_to_cruise){
            state = ACCELERATING;
        } else {
            state = CRUISING;
        }
    }
    return state;
}

short BasicStepperDriver::getMaxMicrostep(){
    return BasicStepperDriver::MAX_MICROSTEP;
}
