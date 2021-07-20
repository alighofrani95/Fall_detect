#include "rgb565_to_bmp.h"
#include <string.h>

int Rgb565ConvertBmp(char *buf,int width,int height, FALL_BMP_File *file)
{
	BitMapFileHeader bmfHdr; //定义文件头
	BitMapInfoHeader bmiHdr; //定义信息头
	RgbQuad bmiClr[3]; //定义调色板

	bmiHdr.biSize = sizeof(BitMapInfoHeader);
	bmiHdr.biWidth = width;//指定图像的宽度，单位是像素
	bmiHdr.biHeight = height;//指定图像的高度，单位是像素
	bmiHdr.biPlanes = 1;//目标设备的级别，必须是1
	bmiHdr.biBitCount = 16;//表示用到颜色时用到的位数 16位表示高彩色图
	bmiHdr.biCompression = BI_BITFIELDS;//BI_RGB仅有RGB555格式
	bmiHdr.biSizeImage = (width * height * 2);//指定实际位图所占字节数
	bmiHdr.biXPelsPerMeter = 0;//水平分辨率，单位长度内的像素数
	bmiHdr.biYPelsPerMeter = 0;//垂直分辨率，单位长度内的像素数
	bmiHdr.biClrUsed = 0;//位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）
	bmiHdr.biClrImportant = 0;//说明对图象显示有重要影响的颜色索引的数目，0表示所有颜色都重要

	//RGB565格式掩码
	bmiClr[0].rgbBlue = 0;
	bmiClr[0].rgbGreen = 0xF8;
	bmiClr[0].rgbRed = 0;
	bmiClr[0].rgbReserved = 0;

	bmiClr[1].rgbBlue = 0xE0;
	bmiClr[1].rgbGreen = 0x07;
	bmiClr[1].rgbRed = 0;
	bmiClr[1].rgbReserved = 0;

	bmiClr[2].rgbBlue = 0x1F;
	bmiClr[2].rgbGreen = 0;
	bmiClr[2].rgbRed = 0;
	bmiClr[2].rgbReserved = 0;


	bmfHdr.bfType = (WORD)0x4D42;//文件类型，0x4D42也就是字符'BM'
	bmfHdr.bfSize = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + sizeof(RgbQuad) * 3 + bmiHdr.biSizeImage);//文件大小
	bmfHdr.bfReserved1 = 0;//保留，必须为0
	bmfHdr.bfReserved2 = 0;//保留，必须为0
	bmfHdr.bfOffBits = (DWORD)(sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader)+ sizeof(RgbQuad) * 3);//实际图像数据偏移量

	memcpy(&(file->BMPHead), &bmfHdr, sizeof(bmfHdr));
	memcpy(&(file->BMIHead), &bmiHdr, sizeof(bmiHdr));
	memcpy(&(file->RgbQuadClr), &bmiClr, 3*sizeof(RgbQuad));
	uint16_t *f_buf = (uint16_t *)buf;
	for(int i=0, j = 0; i<height; i++, j++) {
		// file->frame_buf[j] = f_buf[width*(height-i-1)];
		// fwrite(buf+(width*(height-i-1)*2), 2, width, fp);
		memcpy(&file->frame_buf[width*(height-i-1)], buf+(width*(height-i-1)), width);
	}
// 	if (!(fp = fopen(filename, "wb"))){
// 		return -1;
// 	} else {
// 		printf("file %s open success\n",filename);
// 	}

// 	fwrite(&bmfHdr, 1, sizeof(BitMapFileHeader), fp); 
// 	fwrite(&bmiHdr, 1, sizeof(BitMapInfoHeader), fp); 
// 	fwrite(&bmiClr, 1, 3*sizeof(RgbQuad), fp);
 
// //	fwrite(buf, 1, bmiHdr.biSizeImage, fp);	//mirror
// 	for(int i=0; i<height; i++){
// 		fwrite(buf+(width*(height-i-1)*2), 2, width, fp);
// 	}
 
// 	printf("Image size=%d, file size=%d, width=%d, height=%d\n", bmiHdr.biSizeImage, bmfHdr.bfSize, width, height);
// 	printf("%s over\n", __FUNCTION__);
// 	fclose(fp);
 
	return 0;
}
