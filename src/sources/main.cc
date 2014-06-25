#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"

#include "sybie/datain/datain.hh"
#include "sybie/common/ManagedRes.hh"

const std::string WindowName = "Potrait";
enum { FrameWidth = 1280, FrameHeight = 720 };

const double
    PotraitUpperBound = 0.3,
    PotraitLowerBound = 0.4,
    PotraitWidth = 0.2;

const double
    BGWidth = 0.1,
    BGTop = 0.7;


int main()
{
    cv::namedWindow(WindowName, CV_WINDOW_AUTOSIZE);

    //拍照
    cv::VideoCapture cam(-1);
    cam.set(CV_CAP_PROP_FRAME_WIDTH, FrameWidth);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, FrameHeight);
    cv::Mat frame;

    while(cv::waitKey(1) == -1)
    {
        cam.read(frame);
        cv::imshow(WindowName, frame);
    }

    //人脸检测
    cv::CascadeClassifier face_cascade;
    {
        sybie::common::TemporaryFile CascadeFile =
            sybie::datain::GetTemp("haarcascade_frontalface_alt.xml");
        if (!face_cascade.load(CascadeFile.GetFilename()))
        {
            std::cerr<<"Failed load cascade file: "<<CascadeFile.GetFilename()<<std::endl;
            return 1;
        }
    }
    cv::Mat frame_gray;
    std::vector<cv::Rect> faces;
    cvtColor(frame, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    face_cascade.detectMultiScale(
        frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
    if (faces.size() == 0)
    {
        std::cout<<"Failed detect face."<<std::endl;
        return 0;
    }

    //裁剪人像
    cv::Rect area = faces[0];
    area.x = area.x - (int)(area.width * PotraitWidth);
    area.width = (int)(area.width * (1 + PotraitWidth * 2));
    area.y = area.y - (int)(area.height * PotraitUpperBound);
    area.height = (int)(area.height * (1 + PotraitUpperBound + PotraitLowerBound));
    std::cout<<"Cut:"<<area<<std::endl;
    cv::Mat img_potrait(frame, area);

    //组织mask
    cv::Mat mask(img_potrait.rows, img_potrait.cols, CV_8UC1);
    for (int r = 0 ; r < mask.rows ; r++)
        for (int c = 0 ; c < mask.cols ; c++)
        {
            uint8_t& m = mask.at<uint8_t>(r,c);
            if ((c < mask.cols * BGWidth || c > mask.cols * (1- BGWidth)) && r < mask.rows * BGTop)
                m = cv::GC_BGD;
            else
                m = cv::GC_PR_FGD;
        }

    //分离
    cv::Mat bgModel,fgModel; //临时空间
    cv::grabCut(img_potrait, mask, cv::Rect(), bgModel,fgModel, 3,
                cv::GC_INIT_WITH_MASK);

    //mask应用到照片
    for (int r = 0 ; r < img_potrait.rows ; r++)
        for (int c = 0 ; c < img_potrait.cols ; c++)
        {
            uint8_t m = mask.at<uint8_t>(r,c);
            if (m == cv::GC_BGD || m == cv::GC_PR_BGD)
                img_potrait.at<cv::Vec3b>(r,c) = 0;
        }

    //显示结果
    cv::namedWindow(WindowName+"0", CV_WINDOW_AUTOSIZE);
    cv::imshow(WindowName+"0", img_potrait);

    cv::waitKey(0);
    return 0;
}
