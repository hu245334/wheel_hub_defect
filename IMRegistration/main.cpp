#include<iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <cstring>        // for strcat()
#include <io.h>
#include <direct.h>
#include "Registration_Hough.h"
#include "Registration_geometric.h"
#include "Defect_extraction.h"

using namespace std;
using namespace cv;


Mat org, img, dst;
Point local[2] = { (-1, -1), (-1, -1) };//��һ��ͼƬ�����Ͻǣ���֮����׼ͼƬ��Բ��
int width, height;
static CvRect ROI_rect;//��һ��ͼƬԲ�ĵ�����

//��ͼƬ��·��
char path[100] = "IMG\\*";
char openpath[100];
_finddata_t findData;

void on_mouse(int event, int x, int y, int flags, void *ustc)//flags���������ק�¼�
{
	static bool check_line_state = false;
	CvPoint center;
	int thickness = 2;
	int radius;
	if (event == CV_EVENT_LBUTTONDOWN)//�������
	{
		//��ǰ���λ�ã�x��y��  
		ROI_rect.x = x;
		ROI_rect.y = y;
		check_line_state = true;
	}
	else if (check_line_state&&event == CV_EVENT_MOUSEMOVE)//����ƶ�
	{
		org.copyTo(img);
		Mat MsrcIMG = img;
		IplImage psrcImg = MsrcIMG;
		IplImage *srcImg = &psrcImg;

		center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ��Բ�� 
		radius = (int)sqrt(double(x - center.x)*(x - center.x) + (y - center.y)*(y - center.y));//HJ:����뾶
		cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), thickness, CV_AA, 0);
		cvShowImage("img", srcImg);
	}
	else if (check_line_state&&event == CV_EVENT_LBUTTONUP)//�ſ����
	{
		check_line_state = false;
		cvWaitKey(20);

		org.copyTo(img);
		Mat MsrcIMG = img;
		IplImage psrcImg = MsrcIMG;
		IplImage *srcImg = &psrcImg;
		center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ��Բ�� 
		radius = (int)sqrt(double(x - center.x)*(x - center.x) + (y - center.y)*(y - center.y));//HJ:����뾶
		cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), thickness, CV_AA, 0);
		local[0].x = ROI_rect.x - radius;
		local[0].y = ROI_rect.y - radius;
		dst = org(Rect(local[0].x, local[0].y, width, height));
		width = height = 2 * radius;
		cvShowImage("img", srcImg);
	}
}


int main(){
	//����ͼ��
	int mark = 0;//����Ƿ��ǵ�һ��ͼƬ
	intptr_t handle;

	//ʹ��Hough������׼����
	//��pathĿ¼���½�Hough�ļ���
	strcpy_s(openpath, path);
	openpath[strlen(openpath) - 1] = '\0';
	strcat_s(openpath, "Hough\\");
	_mkdir(openpath);

	handle = _findfirst(path, &findData);    // ����Ŀ¼�еĵ�һ���ļ�
	if (handle == -1){
		cout << "�򿪵�һ���ļ�ʧ��!\n";
		return -1;
	}

	do{
		if (findData.attrib & _A_SUBDIR){//�Ƿ�����Ŀ¼����"."��".."
			//cout << findData.name << "\t<dir>\n";
		}
		else{
			strcpy_s(openpath, path);
			openpath[strlen(openpath) - 1] = '\0';
			strcat_s(openpath, findData.name);
			if (0 == mark){
				mark = 1;//�Ѿ�������һ��ͼƬ,��ǳ���
				Mat srcImage = imread(openpath);//����ͼ���ַ
				//imshow("srcImage", srcImage); //��ʾԭͼ

				//�������ڴ�ŶԱȶȺ����ȱ仯���ͼ��
				srcImage.copyTo(org);
				org.copyTo(img);
				namedWindow("img");
				setMouseCallback("img", on_mouse, 0);
				imshow("img", org);
				//�ȴ���ͼ����
				while (1)
				{
					int key;
					key = waitKey(20);
					if (char(key) == 27)
					{
						break;
					}
				}
				Mat MsrcIMG = org(Rect(local[0].x, local[0].y, width, height));
				IplImage psrcImg = MsrcIMG;
				IplImage *srcImg = &psrcImg;
				//cvNamedWindow("Src", CV_WINDOW_AUTOSIZE);
				cvNamedWindow("Result", CV_WINDOW_AUTOSIZE);
				cvNamedWindow("Temp", CV_WINDOW_AUTOSIZE);

				IplImage *houghImg = cvCreateImage(cvGetSize(srcImg), IPL_DEPTH_8U, 3);
				IplImage *houghTmp = cvCreateImage(cvGetSize(srcImg), IPL_DEPTH_8U, 3);
				local[1] = houghTrans_r(srcImg, houghImg, houghTmp, 200, 1, 1);

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ��Բ��
				//�ú����Ĺ����ǽ���ȱ����ȡ
				Defect_extraction(MsrcIMG, srcImage, org, center, width, local[0]);


				//�任��IMG\\Hough\\A009����-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "Hough\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);

			}
			else{
				Mat srcImage = imread(openpath);//����ͼ���ַ

				srcImage.copyTo(org);
				IplImage psrcImg = org;
				IplImage *srcImg = &psrcImg;

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ��Բ�� 
				int radius = width/2;//HJ:����뾶
				cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), 2, CV_AA, 0);
				cvShowImage("img", srcImg);

				//�任��IMG\\Hough\\A009����-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "Hough\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);


				srcImage.copyTo(org);
				Mat MsrcIMG = org(Rect(local[0].x, local[0].y, width, height));
				psrcImg = MsrcIMG;
				srcImg = &psrcImg;

				IplImage *houghImg = cvCreateImage(cvGetSize(srcImg), IPL_DEPTH_8U, 3);
				IplImage *houghTmp = cvCreateImage(cvGetSize(srcImg), IPL_DEPTH_8U, 3);
				//��ͼ���Բ������
				Point shift = houghTrans_r(srcImg, houghImg, houghTmp, 200, 1, 1);

				//ȫ�ַ�Χ��Բ������
				center.x = ROI_rect.x + shift.x - local[1].x;
				center.y = ROI_rect.y + shift.y - local[1].y;

				//�ú����Ĺ����ǽ���ȱ����ȡ
				Mat IMG_ROI = org(Rect(local[0].x + shift.x - local[1].x, local[0].y + shift.y - local[1].y, width, height));
				CvPoint tmp_local = cvPoint(local[0].x + shift.x - local[1].x, local[0].y + shift.y - local[1].y);
				Defect_extraction(IMG_ROI, srcImage, org, center, width, tmp_local);
				
				//�任��IMG\\Hough\\A009����-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "Hough\\");
				strcat_s(openpath, findData.name);
				openpath[strlen(openpath) - 4] = '\0';
				strcat_s(openpath, "_��׼ͼ.bmp");
				imwrite(openpath, org);

			}
		}
	} while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�

	_findclose(handle);    // �ر��������

	//�ٴν�mark��0
	mark = 0;
	//ʹ��geom������׼����
	strcpy_s(openpath, path);
	openpath[strlen(openpath) - 1] = '\0';
	strcat_s(openpath, "geom\\");
	_mkdir(openpath);

	handle = _findfirst(path, &findData);    // ����Ŀ¼�еĵ�һ���ļ�
	if (handle == -1){
		cout << "�򿪵�һ���ļ�ʧ��!\n";
		return -1;
	}

	do{
		if (findData.attrib & _A_SUBDIR){//�Ƿ�����Ŀ¼����"."��".."
			//cout << findData.name << "\t<dir>\n";
		}
		else{
			strcpy_s(openpath, path);
			openpath[strlen(openpath) - 1] = '\0';
			strcat_s(openpath, findData.name);
			if (0 == mark){
				mark = 1;//�Ѿ�������һ��ͼƬ
				Mat srcImage = imread(openpath);//����ͼ���ַ
				srcImage.copyTo(org);

				Mat MsrcIMG = org;
				Mat dst2, tmp;
				local[1] = geometricTrans_r(MsrcIMG, dst2, tmp, 200, local[0], width, height);

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ��Բ�� 

				CvPoint tmp_local = cvPoint(1, 1);
				//�ú����Ĺ����ǽ���ȱ����ȡ
				Defect_extraction(MsrcIMG, srcImage, org, center, width, tmp_local);

				//�任��IMG\\geom\\A009����-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "geom\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);
			}
			else{
				Mat srcImage = imread(openpath);//����ͼ���ַ
				srcImage.copyTo(org);

				IplImage psrcImg = org;
				IplImage *srcImg = &psrcImg;

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ��Բ�� 
				int radius = width / 2;//HJ:����뾶
				cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), 2, CV_AA, 0);

				//�任��IMG\\geom\\A009����-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "geom\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);

				srcImage.copyTo(org);

				//��ԭͼ����ȥ
				Mat MsrcIMG = org;
				Mat dst2, tmp;
				Point shift = geometricTrans_r(MsrcIMG, dst2, tmp, 200, local[0], width, height);


				//Բ������Ҫ����ƫ��
				center.x = ROI_rect.x + shift.x - local[1].x;
				center.y = ROI_rect.y + shift.y - local[1].y;

				CvPoint tmp_local = cvPoint(1, 1);
				//�ú����Ĺ����ǽ���ȱ����ȡ
				Defect_extraction(MsrcIMG, srcImage, org, center, width, tmp_local);

				//�任��IMG\\result\\A009����-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "geom\\");
				strcat_s(openpath, findData.name);
				openpath[strlen(openpath) - 4] = '\0';
				strcat_s(openpath, "_��׼ͼ.bmp");
				imwrite(openpath, org);
			}
		}
	} while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�

	_findclose(handle);    // �ر��������

	system("pause");
	return 0;
}