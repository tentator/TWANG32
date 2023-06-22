/*
 *  This creates sound tones by outputting a square wave on a DAC pin. The
 *  volume of the tone is the level of the DAC pin.
 *  
 *  The square wave is created by a timer. The timer runs at 2x the freq, because
 *  it needs to transition high and then low.
 * 
 *   
 */
#ifndef SOUND_H
  #define SOUND_H

#include "esp32-hal-timer.h";

#define ESP32_F_CPU         80000000  // the speed of the processor
#define AUDIO_INTERRUPT_PRESCALER   80 
#define SOUND_TIMER_NO 0
//#define AUDIO_PIN 25
#define MIN_FREQ 20
#define MAX_FREQ 16000

hw_timer_t * sndTimer = NULL;

bool sound_on = true;
bool sound_wave_high = true;  // this toggles to create the high/low transitions of the wave
uint8_t sound_volume = 0;

void sound_init(int pin);
bool sound(uint16_t freq, uint8_t volume);
void soundOff();
void sound_stop();

int dac_pin;



void IRAM_ATTR onSoundTimer() 
{    
  if (sound_on) {    
			dacWrite(dac_pin, (sound_wave_high?sound_volume:0));
      sound_wave_high = ! sound_wave_high;
   
  }
  else
    dacWrite(dac_pin, 0);
}

void sound_init(int pin){  // pin must be a DAC pin number !! (typically 25 or 26)
  dac_pin = pin;
	sound_on = false;
	pinMode(dac_pin, OUTPUT);
	sound_volume = 0;
	
	sndTimer = timerBegin(SOUND_TIMER_NO, AUDIO_INTERRUPT_PRESCALER, true);
  timerAttachInterrupt(sndTimer, &onSoundTimer, true);          
  timerAlarmWrite(sndTimer, ESP32_F_CPU/AUDIO_INTERRUPT_PRESCALER/MIN_FREQ, true); // lower timer freq
  timerAlarmEnable(sndTimer);	
}

void sound_stop() // remove interrupt timer for sound
{
	if (sndTimer != NULL)
  {
    timerAlarmDisable(sndTimer);	
    timerDetachInterrupt(sndTimer);          
    timerEnd(sndTimer);
  }
}

void sound_pause() // this prevents the interrupt from firing ... use during eeprom write
{
	if (sndTimer != NULL)
  {
  	timerStop(sndTimer);	
  }
}

void sound_resume() // resume from pause ... after eeprom write
{
	if (sndTimer != NULL)
		timerRestart(sndTimer);
}

bool sound(uint16_t freq, uint8_t volume){
	if (volume == 0) {
		soundOff();
		return false;
	}
	if (freq < MIN_FREQ || freq > MAX_FREQ) {
		return false;
	}
	sound_on = true;
	sound_volume = volume;
  timerAlarmWrite(sndTimer, ESP32_F_CPU/AUDIO_INTERRUPT_PRESCALER/(freq * 2), true);
	return true;	
}

void soundOff(){  
 sound_on = false;
 sound_volume = 0;
 timerAlarmWrite(sndTimer, ESP32_F_CPU/AUDIO_INTERRUPT_PRESCALER/(MIN_FREQ), true);  // lower timer freq
}



#endif






