//author: Mohammed Asif Chand
//testing the speed of data retrievel using camera

#include "grabber.h"
#include <iostream>
#include <chrono>
#include <fstream>


using namespace std;
using namespace std::chrono;


int main(){




	for( int i =0; i<100 ; i++){

	  grabber::BaslerGrabber mygrabber(1,i); 


	}


return 0;
}

