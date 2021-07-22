#ifndef _GRABBER_H_
#define _GRABBER_H_

// Author: Asif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

// Use sstream to create image names including integer
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace Pylon; // Not encouraged but it's a mess otherwise
using namespace std;   //not using namepace of opencv because I prefer not doing that
#define DEBUGGING 1

namespace grabber{


    void saveFile(double value,cv::Mat Image);

    void ImageProcess(cv::Mat Image);  //TODO: reduce resolutiion of image


class BaslerGrabber{


public:

BaslerGrabber(){} // default constructor
~BaslerGrabber(){} // destructor 

BaslerGrabber(int shots, double angle): steeringValue(angle), numberOfShots(shots){

    Pylon::PylonAutoInitTerm autoInitTerm;

    CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

    camera.MaxNumBuffer = 10;

    CImageFormatConverter formatConverter;
    formatConverter.OutputPixelFormat= PixelType_BGR8packed;
    CPylonImage pylonImage;
    //Image = new cv::Mat[numberOfShots]; // create image on heap, might run out of memory !!VERY IMPORTANT

    camera.StartGrabbing(numberOfShots);
    CGrabResultPtr ptrGrabResult;

    while ( camera.IsGrabbing() ) {



        // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
        camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);


        if (ptrGrabResult->GrabSucceeded()) {
            // Access the image data.
	           cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
	           cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
            const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
	           cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;

            // Convert the grabbed buffer to pylon imag
            formatConverter.Convert(pylonImage, ptrGrabResult);
            // Create an OpenCV image out of pylon image
            Image = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3,
                             (uint8_t *) pylonImage.GetBuffer());



            // cv::namedWindow("Mywindow", cv::WINDOW_AUTOSIZE);// Create a window for display.
            // cv::imshow("Display window", Image);                   // Show our image inside it.


             //cv::waitKey(0);
		
	    const clock_t begin_time = clock();
            grabber::saveFile(steeringValue,Image);
	   	
            std::cout << float( clock() - begin_time) /CLOCKS_PER_SEC;  
	   



	    break;
        }

        else
        {
            cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
        }

    }


















}


private: 

cv::Mat Image;
double steeringValue;
int numberOfShots =1; // default



};


}

#endif //EOF 
