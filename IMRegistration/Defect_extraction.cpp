#include "Defect_extraction.h"
#include "RemoveSmallRegion.h"
using namespace std;
using namespace cv;


void imageblur(Mat& src, Mat& dst, Size size, int threshold){
	int height = src.rows;
	int width = src.cols;
	blur(src, dst, size);
	for (int i = 0; i < height; i++)
	{
		uchar* p = dst.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			if (p[j] < threshold)
				p[j] = 0;
			else p[j] = 255;
		}
	}
}

//��MsrcIMG��ѡ�еľ��ν���ȱ����ȡ�������org����ʾ�����أ�ͬʱԲҲ��������
//����ROI_rect�Ǹ���Ȥ�����Բ�ģ�width�Ǹ���Ȥ�����local��Ҫ�õ���������Ͻ�
void Defect_extraction(cv::Mat& MsrcIMG, cv::Mat& srcImage, cv::Mat& org, CvPoint& center, int width, cv::Point local){
	//��ROI(����)ͼ�������ֵ�˲����˴������Ƚ��лҶȱ任���Ӷ��Ż����
	Mat MID_IMG;
	medianBlur(MsrcIMG, MID_IMG, 15);
	Mat result;
	subtract(MsrcIMG, MID_IMG, result);
	Mat thre_IMG;
	threshold(result, thre_IMG, 4, 255, THRESH_BINARY);
	//imshow("������ͼ��", thre_IMG);
	//��������ROI���������ͼ�����ͳ�ƣ����߽�����̬ѧ�ı任
	Mat ele = getStructuringElement(MORPH_CROSS, Size(3, 3));
	Mat thre_IMG1;
	thre_IMG.copyTo(thre_IMG1);

	erode(thre_IMG1, thre_IMG1, ele);//��ʴ
	//imshow("��ʴ���ͼ��", thre_IMG1);
	dilate(thre_IMG1, thre_IMG1, ele);//����
	dilate(thre_IMG1, thre_IMG1, ele);//����
	//imshow("��ʴ���ͺ��ͼ��", thre_IMG1);
	//�����������ص㣬ɸ�������港ʴ���ͺ�û�еĵط�
	for (int y = 0; y < thre_IMG1.rows; y++){
		for (int x = 0; x < thre_IMG1.cols; x++){
			for (int c = 0; c < 3; c++){
				if (thre_IMG1.at<Vec3b>(y, x)[c] == 0){
					thre_IMG.at<Vec3b>(y, x)[c] = 0;
				}
			}
		}
	}
	thre_IMG.copyTo(thre_IMG1);
	//�������ͣ���θ�ʴ�����Ĵ����ͣ��õ����յ�Ȧ
	dilate(thre_IMG1, thre_IMG1, ele);//����
	dilate(thre_IMG1, thre_IMG1, ele);//����
	dilate(thre_IMG1, thre_IMG1, ele);//����
	erode(thre_IMG1, thre_IMG1, ele);//��ʴ
	erode(thre_IMG1, thre_IMG1, ele);//��ʴ
	erode(thre_IMG1, thre_IMG1, ele);//��ʴ
	erode(thre_IMG1, thre_IMG1, ele);//��ʴ
	erode(thre_IMG1, thre_IMG1, ele);//��ʴ
	dilate(thre_IMG1, thre_IMG1, ele);//����
	dilate(thre_IMG1, thre_IMG1, ele);//����
	dilate(thre_IMG1, thre_IMG1, ele);//����
	dilate(thre_IMG1, thre_IMG1, ele);//����
	//imshow("������ͼ��", thre_IMG1);
	imageblur(thre_IMG1, thre_IMG1, Size(3, 3), 128);
	//imshow("��Ե�⻬���ͼ��", thre_IMG1);

	Mat edges;
	Canny(thre_IMG1, edges, 15, 40, 3);
	int radius = width / 2;//HJ:����뾶
	cout << radius << endl;
	//����ɫȱ�ݼ����,�������Ҫ��Բ����
	for (int y = 0; y < edges.rows; y++){
		for (int x = 0; x < edges.cols; x++){
			//��������Ǳ�Ե��
			if (edges.at<uchar>(y, x) == 255){
				Point True_coord((x + local.x - 1), (y + local.y - 1));//��ʵ������
				//�ж��������Բ��,����Բ�ĵľ���С�ڵ��ڰ뾶,���Բ��Ӧ����У׼֮���Բ��
				int dis = int(sqrt(double(True_coord.x - center.x)*(True_coord.x - center.x) + \
					(True_coord.y - center.y)*(True_coord.y - center.y)));//HJ:����뾶
				if ((dis > radius + 5) || (dis < radius -170)){
					edges.at<uchar>(y, x) = 0;
				}
			}
		}
	}
	imshow("��Ե����", edges);
	Mat fill_edges;
	edges.copyTo(fill_edges);
	Size m_Size = edges.size();
	Mat temimage = Mat::zeros(m_Size.height + 2, m_Size.width + 2, edges.type());//��չͼ��
	edges.copyTo(temimage(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)));
	floodFill(temimage, Point(0, 0), Scalar(255));
	Mat cutImg;//�ü���չ��ͼ��
	temimage(Range(1, m_Size.height + 1), Range(1, m_Size.width + 1)).copyTo(cutImg);
	fill_edges = edges | (~cutImg);
	//imshow("���ͼ��", fill_edges);

	Mat final_fill_edge;
	fill_edges.copyTo(final_fill_edge);
	RemoveSmallRegion(fill_edges, final_fill_edge, 180, 1, 1);
	//imshow("�������ͼ��", final_fill_edge);

	srcImage.copyTo(org);
	IplImage psrcImg = org;
	IplImage* srcImg = &psrcImg;
	cvCircle(srcImg, center, radius, CV_RGB(0, 255, 150), 2, CV_AA, 0);
	//����ɫȱ�ݼ����,�������Ҫ��Բ����
	for (int y = 0; y < final_fill_edge.rows; y++){
		for (int x = 0; x < final_fill_edge.cols; x++){
			//��������Ǳ�Ե��
			if ((final_fill_edge.at<uchar>(y, x) == 255) && (edges.at<uchar>(y, x) == 255)){
				Point True_coord((x + local.x - 1), (y + local.y - 1));//��ʵ������
				org.at<Vec3b>(True_coord.y, True_coord.x)[0] = 0;
				org.at<Vec3b>(True_coord.y, True_coord.x)[1] = 0;
				org.at<Vec3b>(True_coord.y, True_coord.x)[2] = 255;
			}
		}
	}
	imshow("Ŀ��ͼ", org);

	waitKey(0);
}