#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

 int main( int argc, char** argv )
 {
    VideoCapture cap(1); //capturamos la webcam

    if ( !cap.isOpened() )  // si no lo encuentra sale del programa
    {
         cout << "Cannot open the web cam" << endl;
         return -1;
    }

    namedWindow("Control"); //creamos ventana "Control"

  int iLowH = 170;
 int iHighH = 179;

  int iLowS = 150; 
 int iHighS = 255;

  int iLowV = 60;
 int iHighV = 255;

  //Creamos barras para la ventana "Control" 
 createTrackbar("LowH", "Control", &iLowH, 179); //Matiz (0 - 179)
 createTrackbar("HighH", "Control", &iHighH, 179);

  createTrackbar("LowS", "Control", &iLowS, 255); //Saturación (0 - 255)
 createTrackbar("HighS", "Control", &iHighS, 255);

  createTrackbar("LowV", "Control", &iLowV, 255);//Valor (0 - 255)
 createTrackbar("HighV", "Control", &iHighV, 255);

  int iLastX = -1; 
 int iLastY = -1;

  //Capturamos una imagen temporal de la webcam
 Mat imgTmp;
 cap.read(imgTmp); 

	//crea una imagen negra con el tamaño de la camara
 Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;
 

	while (true){
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // lee los frames de la camara



         if (!bSuccess) //si hay un error
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }
		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Covertimos BGR a HSV
 
		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the imag
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

		//cierre morfológico (elimina agujeros del primer plano)
		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

		//calculo los movimientos en la imagen limite
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		// si la area <= 10000, para los objetos y ruido 
		if (dArea > 10000)
		{
			//calcula la posicion del objeto
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;        
        
			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
			{
				//dibuja una linea roja desde el punto previo al punto actual
				line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0,0,255), 2);
			}

			iLastX = posX;
			iLastY = posY;
		}

		imshow("Imagen limite", imgThresholded); //muestra la imagen en hsv 

		imgOriginal = imgOriginal + imgLines;
		imshow("Original", imgOriginal); //muestra la imagen original

		if (waitKey(30) == 27) //esc salimos del programa
		{
			cout << "esc key is pressed by user" << endl;
			break; 
		}
    }

   return 0;
}