#ifndef _MOTO_CONTROL_H_
#define _MOTO_CONTROL_H_

// Motor and clock parameters
// 4096 * 90 / 12 = 30720
#define STEPS_PER_ROTATION 30735 //  adjusted steps for a full turn of minute rotor, 30735 is based on experiment.


extern uint32_t currHourMoto;
extern uint32_t currMinuteMoto;

void MotoInitialize();
void Rotate(int step);
void RotateFast(int step);
void CheckTimeAndRotateMoto();




#endif