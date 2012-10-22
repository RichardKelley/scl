///////////////////////////////////////////////////////////////
// Module    : bfr_demo.c
// Function  : Autodetect and exercise 2608 and 2620 i/o modules.
// Target OS : Linux
///////////////////////////////////////////////////////////////

//For compiling the demo in cpp instead of c.
#ifdef __cplusplus
extern "C" {
#endif

  #include "app2600.h"    // Linux api to 2600 middleware

#ifdef __cplusplus
}
#endif

#include <sensoray/CSensoray3DofIODriver.hpp>

#include <string>
#include <stdio.h>

// EXECUTABLE ////////////////////////////////////////////////////////////////////

int main()
{
  sensoray::CSensoray3DofIODriver sensorayio;
	u32 faults;

	// Open the 2600 api.  Declare one MM in system.
	if ( ( faults = S26_DriverOpen( 1 ) ) != 0 )
	{
		printf( "DriverOpen() fault: %d\n", (int)faults );
	}
	else
	{
		// Open the MM.
		if ( ( faults = S26_BoardOpen( sensorayio.getData().mm_handle_, 0, sensorayio.getData().mm_ip_addr_.c_str() ) ) != 0 )
			printf( "BoardOpen() fault: %d\n", (int)faults );

		// If MM was successfully opened ...
		else
		{
			// Reset the I/O system.
			S26_ResetNetwork( sensorayio.getData().mm_handle_ );

			// Register all iom's.  If no errors, execute the I/O control loop until it is terminated.
			if ( sensorayio.DetectAllIoms() )
			{
				// Execute the i/o control loop until it terminates.
			  sensorayio.io_control_main();
			}
		}

		// Close the api library.
		S26_DriverClose();
	}

	return 0;
}
