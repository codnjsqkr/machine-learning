#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>
#define SIZE 3
void BrightnessControl(BYTE * In, BYTE * Out, int Val, int W, int H)
{
	int Size = W * H;
	for (int i = 0; i < Size; i++)
	{
		if (In[i] + Val > 255) Out[i] = 255;
		else if (In[i] + Val < 0) Out[i] = 0;
		else Out[i] = In[i] + Val;
	}
}
void ContrastControl(BYTE * In, BYTE * Out, double Val, int W, int H)
{
	int Size = W * H;
	for (int i = 0; i < Size; i++)
	{
		In[i] * Val > 255.0 ? Out[i] = 255 : Out[i] = (BYTE)(In[i] * Val);
	}
}
void ObtainHisto(BYTE * Img, int * Histo, int W, int H)
{
	int Size = W * H;
	for (int i = 0; i < Size; i++)
		Histo[Img[i]]++;
	FILE * fp3 = fopen("Histo.txt", "wt");
	for (int i = 0; i < 256; i++)
		fprintf(fp3, "%d\t%d\n", i, Histo[i]);
	fclose(fp3);
}
void Binarization(BYTE * In, BYTE * Out, int Th, int W, int H)
{
	int Size = W * H;
	for (int i = 0; i < Size; i++)
	{
		if (In[i] > Th) Out[i] = 255;
		else Out[i] = 0;
	}
}
void HistoStretching(BYTE * Image, BYTE * Output, int W, int H, int * Histo, double p)
{
	int High, Low;
	int ImgSize = W*H;
	int Limit = ImgSize * p;
	int cnt = 0;
	for (int i = 0; i < 256; i++) {
		cnt += Histo[i];
		if (cnt > Limit) {
			Low = i;
			break;
		}
	}
	cnt = 0;
	for (int i = 255; i >= 0; i--) {
		cnt += Histo[i];
		if (cnt > Limit) {
			High = i;
			break;
		}
	}
	for (int i = 0; i < ImgSize; i++)
	{
		if (Image[i] < Low) Output[i] = 0;
		else if (Image[i] > High) Output[i] = 255;
		else 	Output[i] = (BYTE)(((Image[i] - Low) / (double)(High - Low)) * 255);
	}
}
void ObtainAccHisto(int * Histo, int * AHisto)
{
	AHisto[0] = Histo[0];
	for (int i = 1; i < 256; i++)
		AHisto[i] = AHisto[i - 1] + Histo[i];
}
void HistoEqualization(BYTE * Image, BYTE * Output, int W, int H, int *AHisto,
	int Gmax)
{
	int ImgSize = W*H;
	int Nt = ImgSize;
	double NR = Gmax / (double)Nt;
	int NSum[256];
	for (int i = 0; i < 256; i++)
		NSum[i] = (int)(NR * AHisto[i]);
	for (int i = 0; i < ImgSize; i++)
		Output[i] = NSum[Image[i]];
}
void LowPassFilter(BYTE * Image, BYTE * Output, double* m,
	const int S, int W, int H)
{
	//const int Size = S;
	int Margin = SIZE / 2;
	double Mask[SIZE][SIZE];
	for (int i = 0; i < SIZE*SIZE; i++)
		Mask[i / S][i%S] = m[i];
	double temp = 0.0;
	for (int i = Margin; i < H - Margin; i++) { // 마스크 중앙의 세로방향 이동
		for (int j = Margin; j < W - Margin; j++) { // 마스크 중앙의 가로방향 이동
			for (int m = -Margin; m <= Margin; m++) { // 마스크 중앙 기준 세로방향 주변화소 접근
				for (int n = -Margin; n <= Margin; n++) { // 마스크 중앙 기준 가로방향 주변화소 접근
					temp += (Image[(i + m)*W + (j + n)] * Mask[m + Margin][n + Margin]);
				}
			}
			Output[i*W + j] = (BYTE)temp;
			temp = 0.0;
		}
	}
}
void HighPassFilter(BYTE * Image, BYTE * Output, int* m,
	const int S, int W, int H)
{
	int Margin = SIZE / 2;
	double Mask[SIZE][SIZE];
	for (int i = 0; i < SIZE*SIZE; i++)
		Mask[i / SIZE][i%SIZE] = m[i];
	int temp = 0;
	for (int i = Margin; i < H - Margin; i++) { // 마스크 중앙의 세로방향 이동
		for (int j = Margin; j < W - Margin; j++) { // 마스크 중앙의 가로방향 이동
			for (int m = -Margin; m <= Margin; m++) { // 마스크 중앙 기준 세로방향 주변화소 접근
				for (int n = -Margin; n <= Margin; n++) { // 마스크 중앙 기준 가로방향 주변화소 접근
					temp += (Image[(i + m)*W + (j + n)] * Mask[m + Margin][n + Margin]);
				}
			}
			Output[i*W + j] = temp;
			temp = 0;
		}
	}
}

void HighPassFilter2(BYTE * Image, BYTE * Output, int* m,
	const int S, int W, int H) // 마스크 합이 1인 경우 사용할 함수
{
	int * Temp = (int *)malloc(W*H * sizeof(int));
	int Margin = SIZE / 2;
	double Mask[SIZE][SIZE];
	for (int i = 0; i < SIZE*SIZE; i++)
		Mask[i / SIZE][i%SIZE] = m[i];
	int Max = -99999, Min = 99999;
	int temp = 0;
	for (int i = Margin; i < H - Margin; i++) { // 마스크 중앙의 세로방향 이동
		for (int j = Margin; j < W - Margin; j++) { // 마스크 중앙의 가로방향 이동
			for (int m = -Margin; m <= Margin; m++) { // 마스크 중앙 기준 세로방향 주변화소 접근
				for (int n = -Margin; n <= Margin; n++) { // 마스크 중앙 기준 가로방향 주변화소 접근
					temp += (Image[(i + m)*W + (j + n)] * Mask[m + Margin][n + Margin]);
				}
			}
			if (temp > Max) Max = temp;
			if (temp < Min) Min = temp;
			Temp[i*W + j] = temp;
			temp = 0;
		}
	}
	for (int i = 0; i < W*H; i++)
		Output[i] = (Temp[i] - Min) / (double)(Max - Min) * 255;

	free(Temp);
}
void swap(BYTE * a, BYTE * b) {
	BYTE temp;
	temp = *a;
	*a = *b;
	*b = temp;
}
void Sorting(BYTE * Arr) {
	for (int i = 0; i < 8; i++) { // Pivot index
		for (int j = i + 1; j < 9; j++) { // 비교대상 index
			if (Arr[i] > Arr[j]) swap(&Arr[i], &Arr[j]);
		}
	}
}
BYTE Median(BYTE * arr) {
	Sorting(arr);
	return arr[4];
}
int push(short *stackx, short *stacky, int arr_size, short vx, short vy, int *top)
{
	if (*top >= arr_size) return(-1);
	(*top)++;
	stackx[*top] = vx;
	stacky[*top] = vy;
	return(1);
}

int pop(short *stackx, short *stacky, short *vx, short *vy, int *top)
{
	if (*top == 0) return(-1);
	*vx = stackx[*top];
	*vy = stacky[*top];
	(*top)--;
	return(1);
}
// GlassFire 알고리즘을 이용한 라벨링 함수
void m_BlobColoring(BYTE *Image, BYTE* CutImage, int height, int width)
{
	int i, j, m, n, top, area, Out_Area, index, BlobArea[1000];
	int cnt = 0, cogX = 0, cogY = 0; // 무게중심의 좌표
	long k;
	short curColor = 0, r, c;
	//	BYTE** CutImage2;
	Out_Area = 1;


	// 스택으로 사용할 메모리 할당
	short* stackx = new short[height*width];
	short* stacky = new short[height*width];
	short* coloring = new short[height*width];

	int arr_size = height * width;

	// 라벨링된 픽셀을 저장하기 위해 메모리 할당

	for (k = 0; k < height*width; k++) coloring[k] = 0;  // 메모리 초기화

	for (i = 0; i < height; i++)
	{
		index = i*width;
		for (j = 0; j < width; j++)
		{
			// 이미 방문한 점이거나 픽셀값이 255가 아니라면 처리 안함
			if (coloring[index + j] != 0 || CutImage[index + j] != 255) continue;
			r = i; c = j; top = 0; area = 1;
			curColor++;

			while (1)
			{
			GRASSFIRE:
				for (m = r - 1; m <= r + 1; m++)
				{
					index = m*width;
					for (n = c - 1; n <= c + 1; n++)
					{
						//관심 픽셀이 영상경계를 벗어나면 처리 안함
						if (m < 0 || m >= height || n < 0 || n >= width) continue;

						if ((int)CutImage[index + n] == 255 && coloring[index + n] == 0)
						{
							coloring[index + n] = curColor; // 현재 라벨로 마크
							if (push(stackx, stacky, arr_size, (short)m, (short)n, &top) == -1) continue;
							r = m; c = n; area++;
							goto GRASSFIRE;
						}
					}
				}
				if (pop(stackx, stacky, &r, &c, &top) == -1) break;
			}
			if (curColor < 1000) BlobArea[curColor] = area;
		}
	}

	float grayGap = 255.0f / (float)curColor;

	// 가장 면적이 넓은 영역을 찾아내기 위함 
	for (i = 1; i <= curColor; i++)
	{
		if (BlobArea[i] >= BlobArea[Out_Area]) Out_Area = i;
	}
	// CutImage 배열 클리어~
	//for (k = 0; k < width*height; k++) CutImage[k] = 255;

	// coloring에 저장된 라벨링 결과중 (Out_Area에 저장된) 영역이 가장 큰 것만 CutImage에 저장
	for (k = 0; k < width*height; k++)
	{
		if (coloring[k] == Out_Area) {
			CutImage[k] = 255;  // 가장 큰 것만 저장
			cnt++;// 눈동자 영역 픽셀 수
			cogY += k / width;
			cogX += k % width;
			//CutImage[k] = (unsigned char)(coloring[k] * grayGap);
		}

		//delete[] coloring;
		//delete[] stackx;
		//delete[] stacky;
	}
	// 라벨링 후 가장 넓은 영역에 대해서만 뽑아내는 코드 포함
	for (k = 0; k < width*height; k++)
	{
			CutImage[k] = Image[k]; // 원이미지로 변환
		}

		//delete[] coloring;
		//delete[] stackx;
		//delete[] stacky;
	cogX = cogX / cnt;
	cogY = cogY / cnt;
	for (int i = 0; i < height; i++) { //세로 그리기
		CutImage[i*width + cogX] = 255;
	}
	for (int i = 0; i < width; i++) { //가로 그리기
		CutImage[cogY*width+i] = 255;
	}

}



void main()
{
	BITMAPFILEHEADER hf; // BMP 파일헤더 14Bytes
	BITMAPINFOHEADER hInfo; // BMP 인포헤더 40Bytes
	RGBQUAD hRGB[256]; // 팔레트 (256 * 4Bytes)
	FILE *fp;
	fp = fopen("pupil2.bmp", "rb");
	if (fp == NULL) return;
	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	BYTE * Image = (BYTE *)malloc(ImgSize);
	BYTE * Output = (BYTE *)malloc(ImgSize);
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	int W = hInfo.biWidth, H = hInfo.biHeight;
	/* 영상처리 */
	//const int Size = 5;
	/*double Mask[SIZE*SIZE];
	for(int i=0; i<SIZE*SIZE; i++) Mask[i] = 1.0 / (SIZE*SIZE);
	LowPassFilter(Image, Output, Mask, SIZE, hInfo.biWidth, hInfo.biHeight);*/
	/*int m[SIZE*SIZE] = { -1, -1, -1,
	-1, 9, -1,
	-1, -1, -1 };
	HighPassFilter2(Image, Output, m, SIZE, hInfo.biWidth, hInfo.biHeight);*/
	//BYTE temp[9];
	//int W = hInfo.biWidth, H = hInfo.biHeight;
	//int Margin = SIZE / 2;
	//for (int i = Margin; i < H - Margin; i++) {
	//	for (int j = Margin; j < W - Margin; j++) {
	//		temp[0] = Image[(i - 1)*W + j-1];
	//		temp[1] = Image[(i - 1)*W + j];
	//		temp[2] = Image[(i - 1)*W + j+1];
	//		temp[3] = Image[i*W + j-1];
	//		temp[4] = Image[i*W + j]; // center
	//		temp[5] = Image[i*W + j+1];
	//		temp[6] = Image[(i + 1)*W + j-1];
	//		temp[7] = Image[(i + 1)*W + j];
	//		temp[8] = Image[(i + 1)*W + j+1];
	//		Output[i*W + j] = Median(temp);
	//	}
	//}
	Binarization(Image, Output, 30, W, H);
	for (int i = 0; i < ImgSize; i++) Output[i] = 255 - Output[i];
	m_BlobColoring(Image, Output, H, W);


	// 이진 영상의 경계검출
	//for (int i = 1; i < H - 1; i++) {
	//	for (int j = 1; j < W - 1; j++) {
	//		if (Output[i*W + j] == 255) { // 전경 화소라면
	//			if (!(Output[(i + 1)*W + j] == 255 &&
	//				Output[(i - 1)*W + j] == 255 &&
	//				Output[i*W + (j + 1)] == 255 &&
	//				Output[i*W + (j - 1)] == 255)) {
	//				Image[i*W + j] = 255;
	//			}
	//		}
	//	}
	//}






	/* 영상처리 */
	fp = fopen("output2.bmp", "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	free(Image);
	free(Output);
}