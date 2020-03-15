#include <iostream>
#include "web-server.h"

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "frame_grabber.h"
#include <unistd.h> // usleep

using namespace std;

void video_feed(const Request & request, Response & response) {
    response.enable_multipart();
    FrameGrabber grabber;
    cv::VideoCapture cap;
    cap.set(cv::CAP_PROP_FRAME_WIDTH,320);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,240);
    cap.open("/dev/video0");
    WorkQueue<cv::Mat> listener(1);
    grabber.frames_topic.add_listener(&listener);
    grabber.begin_grabbing(&cap, "video_feed");
    std::vector<uchar> jpeg_buff;
    cv::Mat frame;
    uint32_t frame_number = 0;
    uint32_t sent_frame_number = 0;
    while(!response.is_closed()) {
        if(!listener.try_pop(frame, 500)){
            continue;
        }
        ++frame_number;

        cv::flip(frame, frame, -1);


        // try to draw chessboard corners
        {
            /*
            //usleep(1E6);
            cv::Size board_size(6,9);
            std::vector<std::vector<cv::Point2f> > imagePoints(1);
            bool found = findChessboardCorners(frame, board_size, imagePoints[0]);
            if(found) drawChessboardCorners(frame, board_size, cv::Mat(imagePoints[0]), found );
            */
        }

        for(int i=0;i<2;++i) {

            // write frame number on frame
            {
                ++sent_frame_number;
                float scale=1.0;
                auto color = cv::Scalar(255,255,255);
                auto font = cv::FONT_HERSHEY_COMPLEX_SMALL;
                auto position = cv::Point(50,50);
                string text = "Cap frame number: " + to_string(frame_number) + " sent: " + to_string(sent_frame_number);
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
            response.write_content("image/jpeg", (char *)&jpeg_buff[0], jpeg_buff.size());
            while(response.bytes_pending() > 0 && ! response.is_closed()) {
                usleep(100);
            }
            cout << "Cap frame number: " + to_string(frame_number) + " sent: " + to_string(sent_frame_number) << endl;
        }
    }
    grabber.frames_topic.remove_listener(&listener);
    grabber.end_grabbing();
    
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
