#include <iostream>
#include "web-server.h"

#include <opencv2/core/core.hpp>
//#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;

void video_feed(const Request & request, Response & response) {
    response.enable_multipart();
    cv::VideoCapture cap;
    cap.open("/dev/video0");
    std::vector<uchar> jpeg_buff;
    cv::Mat frame;
    uint32_t frame_number = 0;
    while(!response.is_closed()) {
        if(!cap.read(frame)){
            usleep(1000);
            continue;
        }
        ++frame_number;

        cv::flip(frame, frame, -1);

        // write frame number on frame
        {
            float scale=1.0;
            auto color = cv::Scalar(255,255,255);
            auto font = cv::FONT_HERSHEY_COMPLEX_SMALL;
            auto position = cv::Point(50,50);
            string text = "Sent frame number: " + to_string(frame_number);
            cv::putText(frame, text.c_str(), position, font, scale, color);
        }

        // encode to jpeg
        {
            std::vector<int> param(2);
            param[0] = cv::IMWRITE_JPEG_QUALITY;
            param[1] = 80;//default(95) 0-100
            cv::imencode(".jpg", frame, jpeg_buff, param);
        }

        // flush socket
        while(response.bytes_pending() > 0 && ! response.is_closed()) {
            usleep(1000);
        }

        response.write_content("image/jpeg", (char *)&jpeg_buff[0], jpeg_buff.size());
    }
}


void video_page(const Request & request, Response & response) {
    response.write_status();
    string body = "<html><h1>mjpeg-server</h1><image src='video_feed'></image></html>";
    response.write_content("text/html", body.c_str(), body.size());
}

int main(){
    try {
        WebServer server;
        server.add_handler("GET", "/", video_page);
        server.add_handler("GET", "/video_feed", video_feed);
        server.run(8080);
    } catch (string e) {
        cout << "Error caught in main: " << e << endl;
    }
}
