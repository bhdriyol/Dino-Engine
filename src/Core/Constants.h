#pragma once

namespace Config {
	// --- CONFIGURATION SETTINGS ---
	const int SCREEN_WIDTH = 1289;   //! The width of the application window in pixels
	const int SCREEN_HEIGHT = 720;  //! The height of the application window in pixels
	//! DON'T CHANGE FOR NOW!!
	//? 800x600 is ok?
	const int UI_HEIGHT = 80;       //! Height of the bottom UI panel (for buttons)
	const int SIM_HEIGHT_PIXELS = SCREEN_HEIGHT - UI_HEIGHT; //! The actual play area height
	const int SCALE = 4;            //! Scaling factor: 1 simulation cell = 4x4 screen pixels (Optimization)

	//! Calculated dimensions of the simulation grid based on the scale
	const int SIM_WIDTH = SCREEN_WIDTH / SCALE;
	const int SIM_HEIGHT = SIM_HEIGHT_PIXELS / SCALE;
}
