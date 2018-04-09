#include "hardware.h"

namespace Hardware {

	namespace Gpio {

		void init() { // set default modes, output values, etc of pins, other that the laser and scanner:

			// ========= Setting Digital pins
			// * NOTE 1: if we want to really speed up things on pin xx, we could use on the DUE:
			// g_APinDescription[ xx ].pPort -> PIO_SODR = g_APinDescription[8].ulPin;
			// g_APinDescription[ xx ].pPort -> PIO_CODR = g_APinDescription[8].ulPin;
			// [Under the hood, Arduino IDE uses Atmel’s CMSIS compliant libraries]
			// [it seems that this methods in a loop() generate a 16MHz square signal while
			// the use of the digitalWrite methods can only go up to 200Hz...]
			pinMode(PIN_LED_DEBUG, OUTPUT);   digitalWrite(PIN_LED_DEBUG, LOW);     // for debug, etc
			pinMode(PIN_LED_MESSAGE, OUTPUT); digitalWrite(PIN_LED_MESSAGE, LOW); // to signal good message reception

			// ========= Configure PWM frequency and resolution (digital and DAC pins)
			// * NOTE 1:  As for april 2018, the core library is NOT working fine. We need
			// to use a contributed library!
			// 			analogWriteResolution(12)
			// * NOTE 2: changing the PWM frequency affects the resolution; for 40kHz, this is 11 bits [0-2047]
			// * NOTE 3: this affects all the PWM pins, that is pins 2 to 13
			setPWMFreq(FREQ_PWM); // frequency in HZ.
			//setPWMDuty(1023); // <-- as this is used to set lasers, the default value is set in the Lasers
			// namespace init() method.

			// ========= DAC: of course, the DACs are not PWM pins. They not need setting.
			// Also note that the voltage they output varies only from 1/6 to 5/6 of 3.3V
			// [chec]


			// ===== Configure PWM frequency:
			// * NOTE 1: this needs revision, the DUE core libraries are not good.
			//setpwmFreq(FREQ_PWM); // this will affet many things: careful
		}

		// Change PWM frequency [and duty resolution to 11bits on all 8 PWM timers - by default
		// they are set to 1kHz]. The code below affect ALL PWM pins [pins 2 to 13]
		// Check here: https://forum.arduino.cc/index.php?topic=453208.0
		void setPWMFreq(uint16_t _freq) { // frequency in HZ

			// PWM set-up on pins DAC1, A8, A9, A10, D9, D8, D7 and D6 for channels 0 through to 7 respectively
			REG_PMC_PCER1 |= PMC_PCER1_PID36;                                               // Enable PWM
			REG_PIOB_ABSR |= PIO_ABSR_P19 | PIO_ABSR_P18 | PIO_ABSR_P17 | PIO_ABSR_P16;     // Set the port B PWM pins to peripheral type B
			REG_PIOC_ABSR |= PIO_ABSR_P24 | PIO_ABSR_P23 | PIO_ABSR_P22 | PIO_ABSR_P21;     // Set the port C PWM pins to peripheral type B
			REG_PIOB_PDR |= PIO_PDR_P19 | PIO_PDR_P18 | PIO_PDR_P17 | PIO_PDR_P16;          // Set the port B PWM pins to outputs
			REG_PIOC_PDR |= PIO_PDR_P24 | PIO_PDR_P23 | PIO_PDR_P22 | PIO_PDR_P21;          // Set the port C PWM pins to outputs
			REG_PWM_CLK = PWM_CLK_PREA(0) | PWM_CLK_DIVA(1);                                // Set the PWM clock A rate to 84MHz (84MHz/1)
			//REG_PWM_SCM |= PWM_SCM_SYNC7 | PWM_SCM_SYNC6 | PWM_SCM_SYNC5 | PWM_SCM_SYNC4 |  // Set the PWM channels as synchronous
			//               PWM_SCM_SYNC3 | PWM_SCM_SYNC2 | PWM_SCM_SYNC1 | PWM_SCM_SYNC0;
			for (uint8_t i = 0; i < PWMCH_NUM_NUMBER; i++)                      // Loop for each PWM channel (8 in total)
			{
				PWM->PWM_CH_NUM[i].PWM_CMR =  PWM_CMR_CPRE_CLKA;                  // Enable single slope PWM and set the clock source as CLKA
				PWM->PWM_CH_NUM[i].PWM_CPRD = 84000000/_freq;                     // Set the PWM period register 84MHz/(40kHz)=2100;
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

		// Change pwm duty cycle (11 bit resolution, so the range is from 0 to 2047
			void setPWMDuty(uint8_t _pinPWM, uint16_t _duty) { // _duty is from 0 to 2047 (11 bit res)
				if (_pinPWM<PWMCH_NUM_NUMBER)
				if (_duty<2048) PWM->PWM_CH_NUM[_pinPWM].PWM_CDTYUPD = _duty;
			}
		}

		namespace Lasers {
			void init() {
				pinMode(PIN_SWITCH_RED, OUTPUT);  digitalWrite(PIN_SWITCH_RED, LOW);

				// Power: will use the PWM pins. No need to set as output, plus its frequency
				// is set in the Gpio init().

				//Default values:
				setSwitchRed(LOW); // no need to have a switch state for the time being (digital output works as a boolean state)
				setPowerRed(0);    // half power (rem: no need to have a "power" variable for the time being - PWM is done by hardware)
			}

			void test() {

			}

			void setSwitchRed(bool _state) { // I make a method here in case there will be some need to do more than digitalWrite
				digitalWrite(PIN_SWITCH_RED, _state);
			}

			void setPowerRed(uint16_t _power) {
				// * NOTE 1: analogWrite(PIN_PWM_RED, _power) does not works properly plus the carrier is 1kHz.
				// * NOTE 2: the 11 bit constraint is done in the setPWMDuty method: mo need to do it here.
				Gpio::setPWMDuty(PIN_PWM_RED, _power);
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
