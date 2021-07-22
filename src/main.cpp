#include "grabber.h"
#include <linux/can.h>
#include <linux/can/raw.h>

#include <endian.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

// Use sstream to create image names including integer
#include <sstream>
#include <cstdlib>



#define SET 1
#define UNSET 0

#define PROGNAME  "socketcan-raw-demo"
#define VERSION  "2.0.0"



using namespace Pylon;
using namespace GenApi;
using namespace cv;










namespace {

    struct steeringAngle {
        std::uint16_t angle;
        // TODO: Some more hypothetical data
    };


    struct canflags{

        bool SENSOR1_FLAG = UNSET;
        bool SENSOR2_FLAG = UNSET;
        uint16_t sense1;
        uint16_t sense2;

    };


    std::sig_atomic_t signalValue;

    void onSignal(int value) {
        signalValue = static_cast<decltype(signalValue)>(value);
    }

    void usage() {
        std::cout << "Usage: " PROGNAME " [-h] [-V] [-f] interface" << std::endl
                  << "Options:" << std::endl
                  << "  -h  Display this information" << std::endl
                  << "  -V  Display version information" << std::endl
                  << "  -f  Run in the foreground" << std::endl
                  << std::endl;
    }

    void version() {
        std::cout << PROGNAME " version " VERSION << std::endl
                  << "Compiled on " __DATE__ ", " __TIME__ << std::endl
                  << std::endl;
    }

    void processFrame(const struct canfd_frame& frame, canflags& theflag) {
        switch (frame.can_id) {
            case 0x1A:
            {


                theflag.SENSOR1_FLAG = SET;
                theflag.sense1 = be16toh(*(std::uint16_t *)(frame.data + 0));
                std::cout << "sensor1 value is  " << theflag.sense1 << std::endl;



            }
                break;


            case 0x1B:
            {

                theflag.SENSOR2_FLAG = SET;
                theflag.sense2 = be16toh(*(std::uint16_t *)(frame.data + 0));
                std::cout << "sensor2 value is  " << theflag.sense2 << std::endl;


            }

            break;

            default:
                // Should never get here if the receive filters were set up correctly
                std::cerr << "Unexpected CAN ID: 0x"
                          << std::hex << std::uppercase
                          << std::setw(3) << std::setfill('0')
                          << frame.can_id << std::endl;
                std::cerr.copyfmt(std::ios(nullptr));



                break;
        }
    }

} // namespace

int main(int argc, char** argv) {

    int exitCode = 0;

   std::auto_ptr <grabber::BaslerGrabber> grab;

    using namespace std::chrono_literals;

    // Options
    const char* interFace;
    bool foreground = false;
    canflags myflag;
    // Service variables
    struct sigaction sa;
    int rc;

    // CAN connection variables
    struct sockaddr_can addr;
    struct ifreq ifr;
    int sockfd;

    // Parse command line arguments
    {
        int opt;

        // Parse option flags
        while ((opt = ::getopt(argc, argv, "Vfh")) != -1) {
            switch (opt) {
                case 'V':
                    version();
                    return EXIT_SUCCESS;
                case 'f':
                    foreground = true;
                    break;
                case 'h':
                    usage();
                    return EXIT_SUCCESS;
                default:
                    usage();
                    return EXIT_FAILURE;
            }
        }

        // Check for the one positional argument
        if (optind != (argc - 1)) {
            std::cerr << "Missing network interface option!" << std::endl;
            usage();
            return EXIT_FAILURE;
        }

        // Set the network interface to use
        interFace = argv[optind];
    }

    // Check if the service should be run as a daemon
    if (!foreground) {
        if (::daemon(0, 1) == -1) {
            std::perror("daemon");
            return EXIT_FAILURE;
        }
    }

    // Register signal handlers
    sa.sa_handler = onSignal;
    ::sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    ::sigaction(SIGINT, &sa, nullptr);
    ::sigaction(SIGTERM, &sa, nullptr);
    ::sigaction(SIGQUIT, &sa, nullptr);
    ::sigaction(SIGHUP, &sa, nullptr);

    // Initialize the signal value to zero
    signalValue = 0;

    // Open the CAN network interface
    sockfd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (-1 == sockfd) {
        std::perror("socket");
        goto errSocket;
    }

    // Set a receive filter so we only receive select CAN IDs
    {
        int numberFilter = 2;
        struct can_filter filter[numberFilter];
        filter[0].can_id   = 0x0A;
        filter[0].can_mask = CAN_SFF_MASK;
        filter[1].can_id   = 0x0B;
        filter[1].can_mask = CAN_SFF_MASK;



        rc = ::setsockopt(
                sockfd,
                SOL_CAN_RAW,
                CAN_RAW_FILTER,
                &filter,
                sizeof(filter)
        );
        if (-1 == rc) {
            std::perror("setsockopt filter");
            goto errSetup;
        }
    }

    // Enable reception of CAN FD frames
    {
        int enable = 1;

        rc = ::setsockopt(
                sockfd,
                SOL_CAN_RAW,
                CAN_RAW_FD_FRAMES,
                &enable,
                sizeof(enable)
        );
        if (-1 == rc) {
            std::perror("setsockopt CAN FD");
            goto errSetup;
        }
    }

    // Get the index of the network interface
    std::strncpy(ifr.ifr_name, interFace, IFNAMSIZ);
    if (::ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        std::perror("ioctl");
        goto errSetup;
    }

    // Bind the socket to the network interface
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    rc = ::bind(
            sockfd,
            reinterpret_cast<struct sockaddr*>(&addr),
            sizeof(addr)
    );
    if (-1 == rc) {
        std::perror("bind");
        goto errSetup;
    }

    // Log that the service is up and running
    std::cout << "Started" << std::endl;

    // Main loop


    while (0 == signalValue) {
        struct canfd_frame frame;
        // Read in a CAN frame
        auto numBytes = ::read(sockfd, &frame, CANFD_MTU);
        switch (numBytes) {
            case CAN_MTU: {

		std::chrono::high_resolution_clock::time_point startTime = high_resolution_clock::now();

                processFrame(frame, myflag);

		std::chrono::high_resolution_clock::time_point endTime = high_resolution_clock::now();

		auto duration = duration_cast< microseconds > (startTime - endTime).coutn();


                std::cout << "my flag is " << myflag.SENSOR1_FLAG << " " << myflag.SENSOR2_FLAG << std::endl;

                bool capture = ((myflag.SENSOR1_FLAG) && (myflag.SENSOR2_FLAG));

                if(capture){

                    std::cout << "Resetting the flags" << std::endl;
                    myflag.SENSOR2_FLAG = UNSET;
                    myflag.SENSOR1_FLAG = UNSET;

                   std::cout << " the value of sensor1 and sensor2 is " << myflag.sense1 << myflag.sense2 << std::endl;

                    //preprocessing entity
                    uint16_t  preprocessing = myflag.sense1 - 0.5*myflag.sense2;
		//    grab.reset(new grabber::BaslerGrabber(1, preprocessing));
                    
                }



            }
                break;
            case CANFD_MTU:
                // TODO: Should make an example for CAN FD
                break;
            case -1:
                // Check the signal value on interrupt
                if (EINTR == errno)
                    continue;

                // Delay before continuing
                std::perror("read");
                std::this_thread::sleep_for(100ms);
            default:
                continue;
        }
    }

    // Cleanup
    if (::close(sockfd) == -1) {
        std::perror("close");
        return errno;
    }

    std::cout << std::endl << "Bye!" << std::endl;
    return EXIT_SUCCESS;

    // Error handling (reverse order cleanup)
    errSetup:
    ::close(sockfd);
    errSocket:
    return errno;
}
