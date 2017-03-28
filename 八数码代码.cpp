#include "stdafx.h"
#include<cstdio>
#include<iostream>
#include<vector>
#include<cstring>
#include<set>
#include <windows.h> //取系统时间
#include <iomanip>
///////////////////////////
#include "opencv/cv.hpp"
#include <queue>
#include <iostream>
#include <cstdio>
#include <vector>
#include <set>
#include <utility>
#include <cstring>
#include <list>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <windows.h> //取系统时间
#include <cmath>
#include <map>

#define getEmpty(x) ((x)>>27)
#define setEmpty(x,y) (((x)&(0xffffffff-(0xf<<27)))|((y)<<27))
#define clearE(x,y) ((x)&(0xffffffff-(7<<(3*(y)))))
#define getE(x,y) ((x)>>(3*(y))&7)
#define putE(x,y,z) ((x)|((z)<<((y)*3)))

using namespace std;

const int delay_t = 50;
const int image_size = 81;

using namespace cv;


namespace draw {
	Mat background(image_size * 3, image_size * 3, CV_8UC3);
	Mat num[9];
	Mat paintImage(600, 800, CV_8UC3);

	void OptInit(const char * file_name) {
		for (int i = 1; i <= 9; ++i) {
			char fName[256];
			sprintf_s(fName, "%s%d.bmp", file_name, i);
			Mat tmp = imread(fName);
			resize(tmp, num[i - 1], Size(image_size, image_size), 0, 0, CV_INTER_LINEAR);
		}
		memset(background.data, 0xff, background.step*background.rows);
	}

	int ImageCopy(Mat & src, Mat & dis, int x = 0, int y = 0) {
		if (src.channels() != dis.channels()) {
			return -1;
		}

		int src_r = src.rows;
		int src_c = src.cols * src.channels();
		uchar * src_data = src.data;
		int src_step = src.step;

		int dis_r = dis.rows;
		int dis_c = dis.cols * dis.channels();
		uchar * dis_data = dis.data;
		int dis_step = dis.step;

		y *= dis.channels();
		for (int i = 0; i < src_r; ++i) {
			//	cout << i << ' ' << (x + i) << endl;
			memcpy(dis_data + (x + i) * dis_step + y,
				src_data + i * src_step,
				src_c);
		}
		return 0;
	}

	Mat getImage(int opt) {
		Mat res(image_size * 3, image_size * 3, CV_8UC3);
		ImageCopy(background, res);
		int emptyPlace = getEmpty(opt) & 0xf;

		int x = 0;
		for (int i = 0; i < 9; ++i) {
			if (i != emptyPlace) {
				ImageCopy(num[((opt & (7 << (i * 3))) >> (i * 3))], res, i / 3 * image_size, i % 3 * image_size);
			}
		}
		return res;
	}


	void showMove(int opt, int from, int x, int y) {
		int emptyPlace = getEmpty(opt) & 0xf;
		int sx = from / 3 * image_size, sy = from % 3 * image_size;
		int dx = emptyPlace / 3 * image_size, dy = emptyPlace % 3 * image_size;

		int cx = 0, cy = 0;
		if (sx < dx) cx = -9;
		else if (sx > dx) cx = 9;
		if (sy < dy) cy = -9;
		else if (sy > dy) cy = 9;

		for (int bx = dx, by = dy; bx != sx || by != sy; bx += cx, by += cy) {
			Mat res = getImage(opt);
			ImageCopy(num[8], res, sx, sy);
			ImageCopy(num[((opt & (7 << (from * 3))) >> (from * 3))], res, bx, by);
			ImageCopy(res, paintImage, x, y);
			imshow("move", paintImage);
			waitKey(delay_t);
		}
		ImageCopy(draw::getImage(opt), paintImage, x, y);
		imshow("move", paintImage);
	}


	void showAnsMove(int opt, int from) {
		int emptyPlace = getEmpty(opt) & 0xf;
		int sx = from / 3 * image_size, sy = from % 3 * image_size;
		int dx = emptyPlace / 3 * image_size, dy = emptyPlace % 3 * image_size;

		int cx = 0, cy = 0;
		if (sx < dx) cx = -9;
		else if (sx > dx) cx = 9;
		if (sy < dy) cy = -9;
		else if (sy > dy) cy = 9;
		Mat res;
		for (int bx = dx, by = dy; bx != sx || by != sy; bx += cx, by += cy) {
			res = getImage(opt);
			ImageCopy(num[8], res, sx, sy);
			ImageCopy(num[((opt & (7 << (from * 3))) >> (from * 3))], res, bx, by);
			imshow("RESULT", res);
			waitKey(delay_t);
		}
	}


	Mat paintImageInit() {
		int row = paintImage.rows;
		int col = paintImage.cols * paintImage.channels();
		uchar * data = paintImage.data;
		int step = paintImage.step;

		for (int i = 0; i < row; ++i) {
			memset(data, 0xff, col);
			data += step;
		}

		return paintImage;
	}


	void drawBack() {

	}



	void drawOpt(int opt) {
		imshow("drawOpt", draw::getImage(opt));
		waitKey(1000);
	}
}

namespace search {

	typedef unsigned int uint;

	const int DNUM = 9;
	const int strategyNum = 4;

	const int strategy[strategyNum] = { -3, +3, -1, +1 };
	const uint invalid[strategyNum] = { 7, 448, 73, 292 };




	int getOpt(const int p[DNUM]) {
		int b[DNUM];
		memset(b, 0, sizeof(b));
		for (int i = 0; i < DNUM; ++i) {
			if (p[i] < -1 || p[i] > 7) return -1;
			if (b[p[i] + 1]) return -1;
			b[p[i] + 1] = 1;
		}
		int res = 0;
		for (int i = 0; i < DNUM; ++i) {
			if (p[i] != -1) {
				res |= (p[i] << (3 * i));
			}
			else {
				res |= (i << 27);
			}
		}
		return res;
	}

	void showOpt(const int opt) {
		int emptyPlace = getEmpty(opt) & 0xf;
		puts("+-+-+-+");
		for (int i = 0; i < 9; ++i) {
			if (i != emptyPlace) {
				printf("|%d", 1 + ((opt & (7 << (i * 3))) >> (i * 3)));
			}
			else {
				printf("| ");
			}
			if (i % 3 == 2) puts("|\n+-+-+-+");
		}
	}

	const int MAXOPT = 362881;

	priority_queue<pair<int, int> > que;
	int pre[MAXOPT];
	int cost[MAXOPT];
	int optR[MAXOPT];
	int black[MAXOPT];
	int TA;
	map<int, int> searchSet;

	int getArr(int opt, int x[9], int y[9]) {
		int k = getEmpty(opt);
		x[0] = 0;
		y[0] = 0;
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				if (i * 3 + j == k) continue;
				int n = getE(opt, i * 3 + j);
				x[n + 1] = i;
				y[n + 1] = j;
			}
		}
		return k;
	}

	int dif(const int opt_, const int targe_) {
		//return 0; 
		int res = 0;
		int initx[9], inity[9];
		int tarx[9], tary[9];

		getArr(opt_, initx, inity);
		getArr(targe_, tarx, tary);
		for (int i = 0; i < 9; ++i) {
			//res += (initx[i] == tarx[i]) + (inity[i] == tary[i]);
			res += abs(initx[i] - tarx[i]) + abs(inity[i] - tary[i]);
		}
		return res;
	}

	int dealNewNode(const int & opt_, const int & targe_, const int & preN, const int & cost_) {
		static int keyCount = 0;
		cout << dif(opt_, targe_) << endl;
		que.push(make_pair(-(cost_ + dif(opt_, targe_)), opt_));
		pre[keyCount] = preN;
		cost[keyCount] = cost_;
		optR[keyCount] = opt_;
		searchSet[opt_] = keyCount;
		return keyCount++;
	}

	int ans = 0;
	void showAns(int mark) {
		if (mark == -1) return;
		showAns(pre[mark]); 
		printf("第%d步状态为：\n", ans);
		++ans;
		int preEmpty = getEmpty(optR[pre[mark]]) & 0xf;
		//cout << preEmpty << endl;
		if (pre[mark] != -1) {
			draw::showAnsMove(optR[mark], preEmpty);
			waitKey(delay_t);
		}
		imshow("RESULT", draw::getImage(optR[mark]));
	}

	void showSearchList(list<int> L, int stdOpt) {
		vector<int> table;
		for (list<int>::iterator i = L.begin(); i != L.end(); ++i)
			table.push_back(*i);

		Mat LImage(81 * 5 + 40, 81, CV_8UC3);

		int l = 0;
		int p = 1, q = 81;
		if (table.size() <= 10) {
			p = 9;
			q = 9;
		}
		else if (table.size() <= 100) {
			p = 3;
			q = 27;
		}
		else {
			p = 1;
			q = 81;
		}
		if (table.size() == 0) return;
		int minId = table[0];
        int minV = 100;// (cost[searchSet[minId]] + dif(table[0], TA));
		for (int i = 0; i < table.size(); ++i) {
			if (l < 4) ++l;
			int rows = LImage.rows, cols = LImage.cols, ch = LImage.channels();
			for (int i = 0; i < rows; ++i)
				memset(LImage.data + i * cols * ch, 0xff, cols * ch);

			int id = table[i];
			int v = (cost[searchSet[id]] + dif(table[i], TA));
			if (v <= minV) {
				minV = v;
				minId = id;
                cout << v << ' ' << minV << endl;
			}
			if (id == stdOpt) {
				minId = stdOpt;
				minV = 0;
			}

			for (int k = i - l + 1; k <= i; ++k) {
				Mat pic = draw::getImage(table[k]), tmp;
				resize(pic, tmp, Size(81, 81), 0, 0, CV_INTER_LINEAR);
				draw::ImageCopy(tmp, LImage, (l - (i - k) - 1) * 91, 0);
			}
			Mat image_roi(81 * 3 + 20, 81, CV_8UC3);
			uchar * data = LImage.data;
			int size = LImage.cols * LImage.channels();
			for (int k = 0; k < p; ++k) {
				for (int j = 5 * 81 + 39; j >= q; --j) {
					memcpy(data + (j)* size, data + (j - q) * size, size);
				}

				//imshow("LIMAGE", LImage);

				for (int r = 0; r < 81 * 3 + 20; ++r) {
					memcpy(image_roi.data + r * (size), LImage.data + (r + 81) * (size), size);
				}

				//cout << k << endl;
				draw::ImageCopy(image_roi, draw::paintImage, 10, 625);
				imshow("move", draw::paintImage);

				waitKey(1);
			}
			//Mat pic = draw::getImage(*i);
			//Mat tmp;
			//resize(pic, tmp, Size(81, 81), 0, 0, CV_INTER_LINEAR);
			//draw::ImageCopy(tmp, draw::paintImage, i * tot++, 550);
			//waitKey(100);

			draw::ImageCopy(draw::getImage(minId), draw::paintImage, 50, 100);
			imshow("move", draw::paintImage);

			waitKey(1);


		}
	}



	void search(const int opt_, const int targe_) {
		dealNewNode(opt_, targe_, -1, 0);
		list<int> waitList;
		waitList.push_back(opt_);
		while (!que.empty()) {
			pair<int, int> ele = que.top();
			int opt = ele.second;
			cout << -ele.first << endl;
			draw::paintImageInit();
			showSearchList(waitList, opt);
			waitList.remove(opt);
			draw::ImageCopy(draw::getImage(opt), draw::paintImage, 50, 100);
			imshow("move", draw::paintImage);
			waitKey(delay_t);
			cout << waitList.size() << endl;
			que.pop();
			int preN = searchSet[opt];
			int emptyPlace = getEmpty(opt);
			for (int i = 0; i < 4; ++i) {
				if (((1 << emptyPlace) & invalid[i]) > 0) continue;
				int newEmptyPlace = emptyPlace + strategy[i];
				int element = getE(opt, newEmptyPlace);
				int optT = clearE(opt, newEmptyPlace);
				int newOpt = setEmpty(putE(optT, emptyPlace, element), newEmptyPlace);
				
				draw::showMove(newOpt, emptyPlace, 300, 500);
				imshow("move", draw::paintImage);
				int key = dealNewNode(newOpt, targe_, preN, cost[searchSet[opt]] + 1);
				Mat tmp;
				resize(draw::getImage(newOpt), tmp, Size(image_size * 1, image_size * 1), 0, 0, CV_INTER_LINEAR);
				draw::ImageCopy(tmp, draw::paintImage, 350, 50 + i * 95);
				imshow("move", draw::paintImage);
				waitList.push_back(newOpt);
				waitKey(delay_t * 10);

				if (searchSet.find(newOpt) != searchSet.end()) {
					int COST = cost[searchSet[opt]] + 1;
					int p = searchSet[newOpt];
					if (cost[p] > COST + 1) {
						cost[p] = COST + 1;
						waitList.push_back(newOpt);
						que.push(make_pair(-(cost[p] + dif(opt_, targe_)), opt_));
					}
					if (newOpt == targe_) {
						showAns(key);
						std::cout << key << endl;
						return;
					}

					continue;
				}
			}
		}
		std::cout << "NO SOLUTION" << endl;
	}

	int eight_digit_code(const int p[DNUM], const int t[DNUM]) {
		int opt = getOpt(p);
		if (opt == -1) return -1;
		int targe = getOpt(t);
		if (targe == -1) return -1;
		search(opt, TA = targe);
		return 0;
	}
}

int main() {
	draw::OptInit("");
	LARGE_INTEGER tick, fc_begin, fc_end;
	QueryPerformanceFrequency(&tick);	//获得时钟频率
	QueryPerformanceCounter(&fc_begin);	//获得初始硬件定时器计数

										//{2, 1, 3, 5, 4, 6, 0, 7, -1}
										//{3, 0, 1, -1, 4, 6, 2, 5, 7};
										//	int p[9] ={0,-1,1,2,3,4,5,6,7};
										//	int targe[9] = {1,0,-1,2,3,4,5,6,7};	 

	int p[9] = { 0, 3, 1, 2 , 4, 7, 5, -1, 6};//好数据
	//{ 3, 0, 1, -1, 4, 6, 2, 5, 7 };
									  // {2, 1, 3, 5, 4, 6, 0, 7, -1};
	int targe[9] = { -1, 0, 1, 2, 3, 4, 5, 6, 7};//{7, 3, 1, 5, -1, 6, 4, 2, 0};
	search::eight_digit_code(p, targe);
	waitKey(0);
	QueryPerformanceCounter(&fc_end);//获得终止硬件定时器计数
	std::cout << setiosflags(ios::fixed) << setprecision(3);
	std::cout << "时钟频率：" << double(tick.QuadPart) / 1024 / 1024 << "GHz" << endl;
	std::cout << setprecision(0);
	std::cout << "时钟计数：" << double(fc_end.QuadPart - fc_begin.QuadPart) << endl;
	std::cout << setprecision(6) << double(fc_end.QuadPart - fc_begin.QuadPart) / double(tick.QuadPart) << "秒" << endl;
	return 0;
}

