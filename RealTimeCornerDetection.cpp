/*

@authors Asael Fernandez, Benjamin Godoy
 November 30th 2020

*/
#include <iostream>
#include <opencv2/opencv.hpp> 


using namespace std;
using namespace cv;

void bordes(Mat, Mat*, double, Mat*, double);
void filtroBox3(Mat, Mat*);
void filtroGauss5(Mat, Mat*);
void filtroMax5(Mat, Mat*);


int main()
{
    VideoCapture c;
    c.open(1);
    if (!c.isOpened()) {
        cout << "Error en la camara" << endl;
        return -1;
    }
    while (true) {

        Mat frame1, frame2, suave, esquinas;
        c >> frame1;
        c >> frame2;
        resize(frame1, frame1, Size(220, 160));
        resize(frame2, frame2, Size(220, 160));
        Mat canalVerde(frame2.rows, frame2.cols, CV_8UC1);
        Vec3b* img_fila;
        uchar* verdefila;

        for (int i = 0; i < frame2.rows; i++) {
            img_fila = frame2.ptr<Vec3b>(i);
            verdefila = canalVerde.ptr<uchar>(i);
            for (int j = 0; j < frame2.cols; j++) {
                verdefila[j] = img_fila[j][1];
            }
        }

        canalVerde.copyTo(suave);
        canalVerde.copyTo(esquinas);
       


        filtroBox3(canalVerde, &suave);

        bordes(suave, &esquinas, 900, &frame1, .1);
        resize(frame1, frame1, Size(680, 480));
        imshow("Final", frame1);
        if (waitKey(30) >= 0) {
            break;
        }


    }
    c.release();

    waitKey(0);

    return 0;
}

void bordes(Mat frame_src, Mat* frame_dst, double th, Mat* final, double alpha) {
    float gradx, grady,v;


    Mat He11(frame_src.rows, frame_src.cols, CV_32F);
    Mat He22(frame_src.rows, frame_src.cols, CV_32F);
    Mat He12(frame_src.rows, frame_src.cols, CV_32F);
    Mat A(frame_src.rows, frame_src.cols, CV_32F);
    Mat B(frame_src.rows, frame_src.cols, CV_32F);
    Mat C(frame_src.rows, frame_src.cols, CV_32F);
    Mat V2(frame_src.rows, frame_src.cols, CV_32F);
    

    int Hx[3][3] = { {-1,0,1},{-1,0,1},{-1,0,1} };
    int Hy[3][3] = { {-1,-1,-1},{0,0,0},{1,1,1} };
    Vec3b* pfila_src;
    Vec3b* pfila_dst;
    

    for (int i = 1; i < (frame_src.rows - 1); i++) {

        pfila_src = frame_src.ptr<Vec3b>(i);
        pfila_dst = frame_dst->ptr<Vec3b>(i);
        
        for (int j = 1; j < frame_src.cols - 1; j++) {

            

            gradx = (Hx[0][0] * (frame_src.at<uchar>(i - 1, j - 1)) + Hx[0][1] * (frame_src.at<uchar>(i - 1, j)) + Hx[0][2] * (frame_src.at<uchar>(i - 1, j + 1)) +
                Hx[1][0] * (frame_src.at<uchar>(i, j - 1)) + Hx[1][1] * (frame_src.at<uchar>(i, j)) + Hx[1][2] * (frame_src.at<uchar>(i, j + 1)) +
                Hx[2][0] * (frame_src.at<uchar>(i + 1, j - 1)) + Hx[2][1] * (frame_src.at<uchar>(i + 1, j)) + Hx[2][2] * (frame_src.at<uchar>(i + 1, j + 1))) / 2;

            grady = (Hy[0][0] * (frame_src.at<uchar>(i - 1, j - 1)) + Hy[0][1] * (frame_src.at<uchar>(i - 1, j)) + Hy[0][2] * (frame_src.at<uchar>(i - 1, j + 1)) +
                Hy[1][0] * (frame_src.at<uchar>(i, j - 1)) + Hy[1][1] * (frame_src.at<uchar>(i, j)) + Hy[1][2] * (frame_src.at<uchar>(i, j + 1)) +
                Hy[2][0] * (frame_src.at<uchar>(i + 1, j - 1)) + Hy[2][1] * (frame_src.at<uchar>(i + 1, j)) + Hy[2][2] * (frame_src.at<uchar>(i + 1, j + 1))) / 2;

                
            

           
             He11.at<float>(i,j) = gradx * gradx;
             He22.at<float>(i, j) = grady * grady;
             He12.at<float>(i, j) = gradx * grady;
               

             
           

        }
    }

    filtroGauss5(He11, &A);
    filtroGauss5(He22, &B);
    filtroGauss5(He12, &C);

    for (int i = 1; i < (frame_src.rows - 1); i++) {
        for (int j = 1; j < frame_src.cols - 1; j++) {
            V2.at<float>(i, j) = ((A.at<float>(i, j) * B.at<float>(i, j)) - (pow(C.at<float>(i, j), 2))
                - (alpha) * (pow(A.at<float>(i, j) + B.at<float>(i, j), 2)));

            v = V2.at<float>(i, j);

            if (v < th) {
                V2.at<float>(i, j) = 0;
            }
            

        }
    }

    Mat ImagenMax(frame_src.rows, frame_src.cols, CV_32F);
    filtroMax5(V2, &ImagenMax);

    imshow("A", A);
    imshow("B", B);
    imshow("C", C);
    imshow("V2", V2);
    imshow("ImagenMax", ImagenMax);

    float* ptr_ImagenMax;

    for (size_t i = 0; i < frame_src.rows; i++)
    {
        pfila_dst = final->ptr<Vec3b>(i);
        ptr_ImagenMax = ImagenMax.ptr<float>(i);
        for (size_t j = 0; j < frame_src.cols; j++)
        {
            if (ptr_ImagenMax[j] > 0)
            {
                pfila_dst[j][0] = 69;
                pfila_dst[j][1] = 138;
                pfila_dst[j][2] = 21;
            }
        }
    }

    

}

void filtroBox3(Mat img_src, Mat* img_dst) {
   
    for (int i = 1; i < img_src.rows - 1; i++) {
        for (int j = 1; j < img_src.cols - 1; j++) {
            img_dst->at<uchar>(i, j) = (img_src.at<uchar>(i - 1, j - 1) + img_src.at<uchar>(i - 1, j) + img_src.at<uchar>(i - 1, j + 1)
                + img_src.at<uchar>(i, j - 1) + img_src.at<uchar>(i, j) + img_src.at<uchar>(i, j + 1)
                + img_src.at<uchar>(i + 1, j - 1) + img_src.at<uchar>(i + 1, j) + img_src.at<uchar>(i + 1, j + 1)) / 9;

        }
    }
}

void filtroGauss5(Mat img_src, Mat* img_dst) {
    for (int i = 2; i < img_src.rows - 2; i++) {
        for (int j = 2; j < img_src.cols - 2; j++) {
            img_dst->at<float>(i, j) = (img_src.at<float>(i - 2, j - 1) * 1 + img_src.at<float>(i - 2, j) * 2 + img_src.at<float>(i - 2, j + 1) * 1
                + img_src.at<float>(i - 1, j - 2) * 1 + img_src.at<float>(i - 1, j - 1) * 3 + img_src.at<float>(i - 1, j) * 5 + img_src.at<float>(i - 1, j + 1) * 3 + img_src.at<float>(i - 1, j + 2) * 1
                + img_src.at<float>(i, j - 2) * 2 + img_src.at<float>(i, j - 1) * 5 + img_src.at<float>(i, j) * 9 + img_src.at<float>(i, j + 1) * 5 + img_src.at<float>(i, j + 2) * 2
                + img_src.at<float>(i + 1, j - 2) * 1 + img_src.at<float>(i + 1, j - 1) * 3 + img_src.at<float>(i + 1, j) * 5 + img_src.at<float>(i + 1, j + 1) * 3 + img_src.at<float>(i + 1, j + 2) * 1
                + img_src.at<float>(i + 2, j - 1) * 1 + img_src.at<float>(i + 2, j) * 2 + img_src.at<float>(i + 2, j + 1) * 1
                ) / 57;

        }
    }
};

void filtroMax5(Mat img_src, Mat* img_dst) {
    for (int i = 2; i < img_src.rows - 2; i++) {
        for (int j = 2; j < img_src.cols - 2; j++) {

            int max = 0;


            for (int x = 0; x < 5; x++) {
                for (int y = 0; y < 5; y++) {
                    if (img_src.at<float>(i - 2 + x, j - 2 + y) > max) {
                        max = img_src.at<float>(i - 2 + x, j - 2 + y);
                    }
                }
            }

            img_dst->at<float>(i, j) = (float)max;
        }
    }
}


