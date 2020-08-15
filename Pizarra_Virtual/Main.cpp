#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/types.hpp"


using namespace cv;
using namespace std;


int main( int argc, char** argv )
{
	Scalar Amarillo = Scalar(0, 255, 255);
	Scalar Rojo = Scalar(0, 0, 255);
	Scalar Verde = Scalar(0, 255, 0);
	Scalar Azul = Scalar(255, 0, 0);

	Scalar lineColor = Rojo;
	
    VideoCapture cap(1); //capturamos la webcam

    if ( !cap.isOpened() )  // si no lo encuentra sale del programa
    {
         cout << "No se puede abrir el webcam" << endl;
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
	Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );
 

	while (true){
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // lee los frames de la camara



         if (!bSuccess) //si hay un error
        {
             cout << "No se puede leer los frames de la camara" << endl;
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

		rectangle(imgOriginal, Point(0,0),Point(70,70), Amarillo, 4); //amarillo
		rectangle(imgOriginal, Point(70, 0), Point(140, 70), Azul, 4); //azul
		rectangle(imgOriginal, Point(140, 0), Point(210, 70), Rojo, 4); //rojo
		rectangle(imgOriginal, Point(210, 0), Point(280, 70), Verde, 4); //verde
		
		rectangle(imgOriginal, Point(320,0),Point(400,70), Scalar(29,112,246), 4); //limpiar pantalla
		putText(imgOriginal, "Limpiar", Point(330,30), 5,0.6, Scalar(29,112,246), 1); //(mat::, nombre, punto, tipografia, tamaño, color, grosor)
		putText(imgOriginal, "Todo", Point(340,50), 5, 0.6, Scalar(29,112,246), 1);
		
		rectangle(imgOriginal, Point(420,0),Point(490,70), Scalar(128,0,128), 4); //limpiar pantalla
		putText(imgOriginal, "Guardar", Point(425,40), 5, 0.6, Scalar(128, 0, 128), 1);
		
		putText(imgOriginal, "Presiona 'ESC' para salir", Point(400, 470), 5, 0.7, Scalar(51, 51, 51), 1);
		//calculo los movimientos en la imagen limite
		Moments oMoments = moments(imgThresholded);
		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;
		
		// si la area <= 10000, para los objetos y ruido 
		if (dArea > 10000)
		{       
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;
			
			if(posX > 0 && posX <70 && posY > 0 && posY < 70){
				lineColor = Amarillo;
			}
			if(posX > 70 && posX <140 && posY > 0 && posY < 70){
				lineColor = Azul;
			}
			if(posX > 140 && posX <210 && posY > 0 && posY < 70){
				lineColor = Rojo;
			}
			if(posX > 210 && posX <280 && posY > 0 && posY < 70){
				lineColor = Verde;
			}
			if(posX > 320 && posX < 400 && posY > 0 && posY < 70){
				imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );
			}
			if(posX > 420 && posX < 490 && posY > 0 && posY < 70){
				imwrite("imagen.jpg", imgLines);
			}

			
			//calcula la posicion del objeto
			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0){
				//dibuja una linea roja desde el punto previo al punto actual
				line(imgLines, Point(posX, posY), Point(iLastX, iLastY), lineColor, 4);
			}
			else
				imgLines = imgLines;
			
			iLastX = posX;
			iLastY = posY;
		}
		else {
			iLastX = -1;
			iLastY = -1;
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
