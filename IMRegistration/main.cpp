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
Point local[2] = { (-1, -1), (-1, -1) };//第一张图片的左上角，和之后配准图片的圆心
int width, height;
static CvRect ROI_rect;//第一张图片圆心的坐标

//打开图片的路径
char path[100] = "IMG\\*";
char openpath[100];
_finddata_t findData;

void on_mouse(int event, int x, int y, int flags, void *ustc)//flags代表鼠标拖拽事件
{
	static bool check_line_state = false;
	CvPoint center;
	int thickness = 2;
	int radius;
	if (event == CV_EVENT_LBUTTONDOWN)//按下左键
	{
		//当前鼠标位置（x，y）  
		ROI_rect.x = x;
		ROI_rect.y = y;
		check_line_state = true;
	}
	else if (check_line_state&&event == CV_EVENT_MOUSEMOVE)//鼠标移动
	{
		org.copyTo(img);
		Mat MsrcIMG = img;
		IplImage psrcImg = MsrcIMG;
		IplImage *srcImg = &psrcImg;

		center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ：圆心 
		radius = (int)sqrt(double(x - center.x)*(x - center.x) + (y - center.y)*(y - center.y));//HJ:计算半径
		cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), thickness, CV_AA, 0);
		cvShowImage("img", srcImg);
	}
	else if (check_line_state&&event == CV_EVENT_LBUTTONUP)//放开左键
	{
		check_line_state = false;
		cvWaitKey(20);

		org.copyTo(img);
		Mat MsrcIMG = img;
		IplImage psrcImg = MsrcIMG;
		IplImage *srcImg = &psrcImg;
		center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ：圆心 
		radius = (int)sqrt(double(x - center.x)*(x - center.x) + (y - center.y)*(y - center.y));//HJ:计算半径
		cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), thickness, CV_AA, 0);
		local[0].x = ROI_rect.x - radius;
		local[0].y = ROI_rect.y - radius;
		dst = org(Rect(local[0].x, local[0].y, width, height));
		width = height = 2 * radius;
		cvShowImage("img", srcImg);
	}
}


int main(){
	//输入图像
	int mark = 0;//标记是否是第一张图片
	intptr_t handle;

	//使用Hough进行配准代码
	//在path目录下新建Hough文件夹
	strcpy_s(openpath, path);
	openpath[strlen(openpath) - 1] = '\0';
	strcat_s(openpath, "Hough\\");
	_mkdir(openpath);

	handle = _findfirst(path, &findData);    // 查找目录中的第一个文件
	if (handle == -1){
		cout << "打开第一个文件失败!\n";
		return -1;
	}

	do{
		if (findData.attrib & _A_SUBDIR){//是否是子目录包括"."和".."
			//cout << findData.name << "\t<dir>\n";
		}
		else{
			strcpy_s(openpath, path);
			openpath[strlen(openpath) - 1] = '\0';
			strcat_s(openpath, findData.name);
			if (0 == mark){
				mark = 1;//已经遍历第一张图片,标记出来
				Mat srcImage = imread(openpath);//输入图像地址
				//imshow("srcImage", srcImage); //显示原图

				//建立用于存放对比度和亮度变化后的图像
				srcImage.copyTo(org);
				org.copyTo(img);
				namedWindow("img");
				setMouseCallback("img", on_mouse, 0);
				imshow("img", org);
				//等待画图完整
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

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ：圆心
				//该函数的功能是进行缺陷提取
				Defect_extraction(MsrcIMG, srcImage, org, center, width, local[0]);


				//变换成IMG\\Hough\\A009轮心-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "Hough\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);

			}
			else{
				Mat srcImage = imread(openpath);//输入图像地址

				srcImage.copyTo(org);
				IplImage psrcImg = org;
				IplImage *srcImg = &psrcImg;

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ：圆心 
				int radius = width/2;//HJ:计算半径
				cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), 2, CV_AA, 0);
				cvShowImage("img", srcImg);

				//变换成IMG\\Hough\\A009轮心-*.bmp
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
				//此图像的圆心坐标
				Point shift = houghTrans_r(srcImg, houghImg, houghTmp, 200, 1, 1);

				//全局范围内圆心坐标
				center.x = ROI_rect.x + shift.x - local[1].x;
				center.y = ROI_rect.y + shift.y - local[1].y;

				//该函数的功能是进行缺陷提取
				Mat IMG_ROI = org(Rect(local[0].x + shift.x - local[1].x, local[0].y + shift.y - local[1].y, width, height));
				CvPoint tmp_local = cvPoint(local[0].x + shift.x - local[1].x, local[0].y + shift.y - local[1].y);
				Defect_extraction(IMG_ROI, srcImage, org, center, width, tmp_local);
				
				//变换成IMG\\Hough\\A009轮心-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "Hough\\");
				strcat_s(openpath, findData.name);
				openpath[strlen(openpath) - 4] = '\0';
				strcat_s(openpath, "_配准图.bmp");
				imwrite(openpath, org);

			}
		}
	} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件

	_findclose(handle);    // 关闭搜索句柄

	//再次将mark置0
	mark = 0;
	//使用geom进行配准代码
	strcpy_s(openpath, path);
	openpath[strlen(openpath) - 1] = '\0';
	strcat_s(openpath, "geom\\");
	_mkdir(openpath);

	handle = _findfirst(path, &findData);    // 查找目录中的第一个文件
	if (handle == -1){
		cout << "打开第一个文件失败!\n";
		return -1;
	}

	do{
		if (findData.attrib & _A_SUBDIR){//是否是子目录包括"."和".."
			//cout << findData.name << "\t<dir>\n";
		}
		else{
			strcpy_s(openpath, path);
			openpath[strlen(openpath) - 1] = '\0';
			strcat_s(openpath, findData.name);
			if (0 == mark){
				mark = 1;//已经遍历第一张图片
				Mat srcImage = imread(openpath);//输入图像地址
				srcImage.copyTo(org);

				Mat MsrcIMG = org;
				Mat dst2, tmp;
				local[1] = geometricTrans_r(MsrcIMG, dst2, tmp, 200, local[0], width, height);

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ：圆心 

				CvPoint tmp_local = cvPoint(1, 1);
				//该函数的功能是进行缺陷提取
				Defect_extraction(MsrcIMG, srcImage, org, center, width, tmp_local);

				//变换成IMG\\geom\\A009轮心-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "geom\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);
			}
			else{
				Mat srcImage = imread(openpath);//输入图像地址
				srcImage.copyTo(org);

				IplImage psrcImg = org;
				IplImage *srcImg = &psrcImg;

				CvPoint center = cvPoint(ROI_rect.x, ROI_rect.y);//HJ：圆心 
				int radius = width / 2;//HJ:计算半径
				cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), 2, CV_AA, 0);

				//变换成IMG\\geom\\A009轮心-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "geom\\");
				strcat_s(openpath, findData.name);
				imwrite(openpath, org);

				srcImage.copyTo(org);

				//将原图传进去
				Mat MsrcIMG = org;
				Mat dst2, tmp;
				Point shift = geometricTrans_r(MsrcIMG, dst2, tmp, 200, local[0], width, height);


				//圆心坐标要进行偏移
				center.x = ROI_rect.x + shift.x - local[1].x;
				center.y = ROI_rect.y + shift.y - local[1].y;

				CvPoint tmp_local = cvPoint(1, 1);
				//该函数的功能是进行缺陷提取
				Defect_extraction(MsrcIMG, srcImage, org, center, width, tmp_local);

				//变换成IMG\\result\\A009轮心-*.bmp
				strcpy_s(openpath, path);
				openpath[strlen(openpath) - 1] = '\0';
				strcat_s(openpath, "geom\\");
				strcat_s(openpath, findData.name);
				openpath[strlen(openpath) - 4] = '\0';
				strcat_s(openpath, "_配准图.bmp");
				imwrite(openpath, org);
			}
		}
	} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件

	_findclose(handle);    // 关闭搜索句柄

	system("pause");
	return 0;
}