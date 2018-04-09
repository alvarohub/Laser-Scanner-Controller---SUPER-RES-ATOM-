#include "hardware.h"

namespace Hardware {

	namespace Gpio {

		void init() { // set default modes, output values, etc of pins, other that the laser and scanner:

			pinMode(PIN_LED_DEBUG, OUTPUT);   digitalWrite(PIN_LED_DEBUG, LOW);     // for debug, etc
			pinMode(PIN_LED_MESSAGE, OUTPUT); digitalWrite(PIN_LED_MESSAGE, LOW); // to signal good message reception

			// Configure PWM resolution:
			// NOTE: this affect of course the PWM resolution on PWM
			// capable pins, but also the DAC resolution, which are
			// capable of 12 bit resolution.
			analogWriteResolution(12);

			// ===== Configure PWM frequency (set pwm freq to 40kHz on all PWM capable pins, making them also 11 bit res).
			// NOTE: call this before initScanners() - I did not test conflicts with the DueTimer setting yet.
			//setpwmFreq(FREQ_PWM); // this will affet many things: careful
		}

		// Change PWM frequency and resolution to 11bits on all 8 PWM timers - by default
		// they are set to 1kHz?:
		void setPWMFreq(uint16_t _freq) {
			// PWM set-up on pins DAC1, A8, A9, A10, D9, D8, D7 and D6 for channels 0
			// through to 7 respectively
			REG_PMC_PCER1 |= PMC_PCER1_PID36;                                               // Enable PWM
			REG_PIOB_ABSR |= PIO_ABSR_P19 | PIO_ABSR_P18 | PIO_ABSR_P17 | PIO_ABSR_P16;     // Set the port B PWM pins to peripheral type B
			REG_PIOC_ABSR |= PIO_ABSR_P24 | PIO_ABSR_P23 | PIO_ABSR_P22 | PIO_ABSR_P21;     // Set the port C PWM pins to peripheral type B
			REG_PIOB_PDR |= PIO_PDR_P19 | PIO_PDR_P18 | PIO_PDR_P17 | PIO_PDR_P16;          // Set the port B PWM pins to outputs
			REG_PIOC_PDR |= PIO_PDR_P24 | PIO_PDR_P23 | PIO_PDR_P22 | PIO_PDR_P21;          // Set the port C PWM pins to outputs
			REG_PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(1);                                // Set the PWM clock A rate to 84MHz (84MHz/1)
			//REG_PWM_SCM |= PWM_SCM_SYNC7 | PWM_SCM_SYNC6 | PWM_SCM_SYNC5 | PWM_SCM_SYNC4 |  // Set the PWM channels as synchronous
			//               PWM_SCM_SYNC3 | PWM_SCM_SYNC2 | PWM_SCM_SYNC1 | PWM_SCM_SYNC0;
			for (uint8_t i = 0; i < PWMCH_NUM_NUMBER; i++)   // Loop for each PWM channel (8 in total)
			{
				PWM->PWM_CH_NUM[i].PWM_CMR =  PWM_CMR_CPRE_CLKA;                  // Enable single slope PWM and set the clock source as CLKA
				PWM->PWM_CH_NUM[i].PWM_CPRD = (uint16_t)84000000/_freq;// Set the PWM period register. Example: 40kHz => 84MHz/(40kHz)=2100;
			}
			//REG_PWM_ENA = PWM_ENA_CHID0;           // Enable the PWM channels, (only need to set channel 0 for synchronous mode)
			REG_PWM_ENA = PWM_ENA_CHID7 | PWM_ENA_CHID6 | PWM_ENA_CHID5 | PWM_ENA_CHID4 |    // Enable all PWM channels
			PWM_ENA_CHID3 | PWM_ENA_CHID2 | PWM_ENA_CHID1 | PWM_ENA_CHID0;
			for (uint8_t i = 0; i < PWMCH_NUM_NUMBER; i++)                      // Loop for each PWM channel (8 in total)
			{
				PWM->PWM_CH_NUM[i].PWM_CDTYUPD = 1050;                            // Set the PWM duty cycle to 50% (2100/2=1050) on all channels
			}
			//REG_PWM_SCUC = PWM_SCUC_UPDULOCK;      // Set the update unlock bit to trigger an update at the end of the next PWM period
		}
	}

	namespace Lasers {
		void init() {
			pinMode(PIN_SWITCH_RED, OUTPUT);  digitalWrite(PIN_SWITCH_RED, LOW);

			// Power: will use the PWM pins. No need to set as output, plus its frequency is set in the Gpio init [because
			// it affects other pins.

			//Default values:
			setSwitchRed(LOW); // no need to have a switch state for the time being (digital output works as a boolean state)
			setPowerRed(0); // half power (rem: no need to have a "power" variable for the time being - PWM is done by hardware)
		}

		void test() {

		}

		void setSwitchRed(bool _state) { // I make a method here in case there will be some need to do more than digitalWrite
			digitalWrite(PIN_SWITCH_RED, _state);
		}

		void setPowerRed(uint16_t _power) {
			analogWrite(PIN_PWM_RED, _power);
		}
	}

	namespace Scanner {

		void init() {
			// set pins and interrupt routines on circular buffer - uses DueTimer.h and DSP.h
		}

		//inline void setMirrorsTo(uint16_t _posX, uint16_t _posY);
		//inline void recenterMirrors();

		void testMirrorRange(uint16_t _durationMs) {
			uint16_t startTime = millis();
			while (millis()-startTime<_durationMs) {
				for (uint16_t x = MIN_MIRRORS_ADX; x < MAX_MIRRORS_ADX; x+=10) {
					for (uint16_t y = MIN_MIRRORS_ADY; y < MAX_MIRRORS_ADY; y+=10) {
						analogWrite( PIN_ADCX, x );
						analogWrite( PIN_ADCY, y );
						delay(1); // in ms (ATTN: of course delay() not be used in the ISR stuff)
					}
				}
			}
		}

		void testCircleRange(uint16_t _durationMs) {
			uint16_t startTime = millis();
			float phi;
			while (millis()-startTime<_durationMs) {
				for (uint16_t k=0;k<360;k++) {
					phi = DEG_TO_RAD*k;
					int16_t x=(int16_t)(1.0*CENTER_MIRROR_ADX*cos(phi));
					int16_t y=(int16_t)(1.0*CENTER_MIRROR_ADX*sin(phi));
					analogWrite( PIN_ADCX, x );
					analogWrite( PIN_ADCY, y );
					delay(1);
				}
			}
		}
	}

	// ======== OTHERS GENERIC HARWARE ROUTINES:
	void init() {
		//initSerial(); // make a namespace for serial? TODO
		Gpio::init();
		Lasers::init();
		Scanner::init();
	}

	//Software reset: better than using the RST pin (noisy?)
	void resetBoard() {
		RSTC->RSTC_MR = 0xA5000801; // Set RST pin to go low for 256 clock cycles on reset
		RSTC->RSTC_CR = 0xA5000013; // Reset processor, internal peripherals, and pull external RST
	}


}
