//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
//	CMOS Input - Software model of a CMOS input
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------

class CMOSInput {
	private:
		// supply voltage
		float Vdd = 12.0f;
		
		// voltages for guaranteed high and low
		float vLow = Vdd  * 0.3f;
		float vHigh = Vdd  * 0.7f;
		
		// threshold around which noise causes instability
		float vThresh = Vdd / 2.0;
		
		// the current input voltage
		float voltage = 0.0f;
		
		// the current state of the input
		bool currentState = true;
		
	public:

		void reset() {
			currentState = false;
			voltage = 0.0f;
		}

		bool process(float in) {
			
			voltage = in;

			// TODO: add instability at middle threshold


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
