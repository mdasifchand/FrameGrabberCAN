#include "grabber.h"

/*

grabber::BaslerGrabber::BaslerGrabber(double &steeringValue, int numberOfShots){ 

//TODO: takes the value of steering and gives certain number of shots then



//debug
std::cout << "Steering value is " << steeringValue << std::endl;



}
*/


void grabber::saveFile(double value, cv::Mat Image) {


    if ((!Image.data) && !DEBUGGING)                              // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl;

    }

    std::ostringstream strs;
    strs << value;
    std::string str = strs.str();
    const string Windowname = "Angle is " + str;

//    cv::namedWindow(Windowname, cv::WINDOW_AUTOSIZE);// Create a window for display.
//    cv::imshow("Display window", Image);                   // Show our image inside it.

 //    cv::waitKey(5);
        cv::imwrite(str+".jpg",Image);


}

void grabber::ImageProcess(cv::Mat Image) {

    //TODO: reduce resolution before saving image


}

