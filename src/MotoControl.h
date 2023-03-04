#ifndef _MOTO_CONTROL_H_
#define _MOTO_CONTROL_H_

// Motor and clock parameters
// 4096 * 90 / 12 = 30720
#define STEPS_PER_ROTATION 30720 // steps for a full turn of minute rotor

void MotoInitialize();
void Rotate(int step);
void RotateFast(int step);




#endif