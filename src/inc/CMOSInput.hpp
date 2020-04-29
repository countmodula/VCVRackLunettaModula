//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
//	CMOS Input - Software model of a CMOS input
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
class CMOSInput {
	private:
		// supply voltage
		float Vdd = VDD;
		
		// voltages for guaranteed high and low
		float vLow = Vdd  / 3.0f;
		float vHigh = Vdd / 1.5f;
		
		// threshold around which noise causes instability
		float vThresh = Vdd / 2.0;
		
		// the current state of the input
		bool currentState = true;
		
		int inputMode = VCVRACK_STANDARD;
	public:

		void reset() {
			currentState = false;
		}

		void setMode(int mode) {
			switch(mode) {
				case CMOS_NON_SCHMITT:
				case CMOS_SCHMITT:
					vLow = Vdd  * 0.3f;
					vHigh = Vdd  * 0.7f;
					inputMode = mode;
					break;
				case VCVRACK_STANDARD:
				default:
					vLow = 0.1f;
					vHigh = 2.0f;
					inputMode = VCVRACK_STANDARD;
					break;
			}
		}

		bool process(float in) {
			
			// TODO: add different mode behaviours here
			if (currentState) {
				// HIGH to LOW
				if (in <= vLow) {
					currentState = false;
				}
			}
			else {
				// LOW to HIGH
				if (in >= vHigh) {
					currentState = true;
				}
			}
			
			return currentState;
		}

		bool isHigh() {
			return currentState;
		}
};
