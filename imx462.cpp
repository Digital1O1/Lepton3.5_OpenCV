#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    // 0 is usually the default camera, change if needed
    // gst-launch-1.0 libcamerasrc ! video/x-raw,width=1920,height=1080,format=NV12 ! videoconvert ! autovideosink

    cv::VideoCapture cap(
        "libcamerasrc ! video/x-raw,width=640,height=480,format=NV12,framerate=30/1 ! "
        "videoconvert ! appsink",
        cv::CAP_GSTREAMER);

    if (!cap.isOpened())
    {
        std::cerr << "Error: Could not open IMX462 camera." << std::endl;
        return -1;
    }

    // Optional: Set resolution for IMX462 (e.g., 1920x1080)
    // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    cv::Mat frame;
    while (true)
    {
        cap >> frame; // Capture a new frame
        if (frame.empty())
            break;

        cv::flip(frame, frame, 0);

        cv::imshow("IMX462 Feed", frame);

        // Press 'ESC' to exit
        if (cv::waitKey(1) == 27)
            break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
