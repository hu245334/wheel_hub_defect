#include "Registration_geometric.h"
using namespace std;
using namespace cv;

//�ü��η����srcImg����local��width,height������Ȥ�����а뾶Ϊr��Բ��Բ�ģ�������Բ������
Point geometricTrans_r(Mat &srcImg, Mat &dst, Mat &temp, float r, Point local, int width, int height){
	Point find;
	dst = srcImg(Rect(local.x, local.y, width, height));
	cvtColor(dst, dst, CV_BGR2GRAY);//��ͨ����ͼת��Ϊ1ͨ���ĻҶ�ͼ
	threshold(dst, dst, 90, 255, THRESH_BINARY);//ͨ����ֵ�����ѻҶ�ͼ��ɶ�ֵͼ

	int midx = dst.cols / 2;//��
	int midy = dst.rows / 2;//��
	midx = midx + local.x + 1;
	midy = midy + local.y + 1;

	srcImg.copyTo(dst);
	cvtColor(dst, dst, CV_BGR2GRAY);//��ͨ����ͼת��Ϊ1ͨ���ĻҶ�ͼ
	threshold(dst, dst, 90, 255, THRESH_BINARY);//ͨ����ֵ�����ѻҶ�ͼ��ɶ�ֵͼ

	int remark[4][2];
	//��
	for (int i = midx; i >= 0; i--){
		if (dst.at<uchar>(midy, i) == 0){
			remark[0][0] = midy;//��
			remark[0][1] = i + 1;
			break;
		}
	}
	//��
	for (int i = midx; i <= dst.cols; i++){
		if (dst.at<uchar>(midy, i) == 0){
			remark[1][0] = midy;//��
			remark[1][1] = i - 1;
			break;
		}
	}
	//��
	for (int j = midy; j >= 0; j--){
		if (dst.at<uchar>(j, midx) == 0){
			remark[2][0] = j + 1;//��
			remark[2][1] = midx;
			break;
		}
	}
	//��
	for (int j = midy; j <= dst.rows; j++){
		if (dst.at<uchar>(j, midx) == 0){
			remark[3][0] = j - 1;//��
			remark[3][1] = midx;
			break;
		}
	}

	int gz = (remark[0][1] + remark[1][1]) / 2;//��
	int hz = (remark[2][0] + remark[3][0]) / 2;//��
	find.x = gz;
	find.y = hz;

	srcImg.copyTo(temp);
	IplImage tmp = IplImage(temp);
	CvArr* arr = (CvArr*)&tmp;
	cvCircle(arr, cvPoint(gz, hz), r, cvScalar(0, 255, 0), 1, 8, 0);
	imshow("dst", temp);
	cvWaitKey(0);

	return find;
}