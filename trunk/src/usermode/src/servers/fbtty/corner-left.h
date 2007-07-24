/*  GIMP header image file format (INDEXED): /home/oleg/tmp/fos-console/corner1.h  */

static unsigned int corner_width = 28;
static unsigned int corner_height = 28;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define CORNER_HEADER_PIXEL(data,pixel) {\
  pixel[0] = corner_header_data_cmap[(unsigned char)data[0]][0]; \
  pixel[1] = corner_header_data_cmap[(unsigned char)data[0]][1]; \
  pixel[2] = corner_header_data_cmap[(unsigned char)data[0]][2]; \
  data ++; }

static char corner_header_data_cmap[256][3] = {
	{  0,  0,  1},
	{  0,  0,  1},
	{  0,  1,  1},
	{  1,  1,  1},
	{  1,  1,  2},
	{  1,  1,  3},
	{  2,  2,  2},
	{  2,  3,  3},
	{  2,  3,  4},
	{  3,  4,  4},
	{  3,  4,  5},
	{  4,  4,  5},
	{  4,  4,  6},
	{  4,  5,  6},
	{  5,  5,  7},
	{  5,  6,  7},
	{  5,  6,  8},
	{  6,  7,  9},
	{  7,  8, 10},
	{  7,  8, 11},
	{  8,  9, 12},
	{  8, 10, 12},
	{ 10, 11, 14},
	{ 10, 12, 14},
	{ 11, 13, 16},
	{ 11, 13, 17},
	{ 12, 14, 18},
	{ 13, 15, 19},
	{ 14, 16, 20},
	{ 15, 18, 22},
	{ 16, 18, 24},
	{ 16, 19, 24},
	{ 17, 20, 24},
	{ 17, 20, 25},
	{ 18, 21, 26},
	{ 19, 23, 28},
	{ 20, 23, 29},
	{ 20, 24, 30},
	{ 22, 26, 32},
	{ 22, 26, 33},
	{ 25, 29, 37},
	{ 25, 30, 37},
	{ 26, 31, 38},
	{ 26, 31, 39},
	{ 27, 32, 39},
	{ 27, 32, 40},
	{ 27, 33, 41},
	{ 29, 35, 44},
	{ 30, 35, 44},
	{ 32, 37, 46},
	{ 32, 38, 47},
	{ 33, 38, 47},
	{ 33, 39, 48},
	{ 33, 39, 49},
	{ 34, 40, 50},
	{ 35, 41, 51},
	{ 35, 42, 52},
	{ 36, 42, 52},
	{ 37, 43, 54},
	{ 38, 44, 55},
	{ 38, 45, 56},
	{ 39, 45, 56},
	{ 39, 45, 57},
	{ 40, 47, 57},
	{ 40, 48, 59},
	{ 42, 49, 61},
	{ 43, 50, 63},
	{ 47, 55, 68},
	{ 48, 56, 70},
	{ 48, 57, 71},
	{ 51, 59, 73},
	{ 51, 60, 74},
	{ 53, 62, 77},
	{ 54, 63, 78},
	{ 54, 64, 80},
	{ 55, 64, 80},
	{ 55, 65, 80},
	{ 56, 66, 81},
	{ 57, 66, 82},
	{ 57, 67, 82},
	{ 57, 67, 83},
	{ 58, 68, 84},
	{ 58, 68, 85},
	{ 59, 69, 86},
	{ 60, 70, 87},
	{ 61, 71, 89},
	{ 62, 73, 91},
	{ 63, 73, 91},
	{ 63, 74, 92},
	{ 64, 75, 93},
	{ 65, 76, 94},
	{ 65, 76, 95},
	{ 66, 77, 95},
	{ 66, 77, 96},
	{ 66, 78, 96},
	{ 67, 78, 97},
	{ 67, 78, 98},
	{ 67, 79, 98},
	{ 68, 79, 98},
	{ 68, 80, 99},
	{ 69, 81,100},
	{ 69, 81,101},
	{ 70, 82,101},
	{ 70, 82,102},
	{ 71, 82,102},
	{  0,  0,  0},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255}
	};
static char corner_header_data[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,3,8,16,21,27,34,38,42,48,51,53,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,
	14,23,32,41,54,66,70,75,82,85,87,88,
	0,0,0,0,0,0,0,0,0,0,0,0,0,4,10,22,
	36,52,67,78,88,97,101,103,105,105,105,105,
	0,0,0,0,0,0,0,0,0,0,0,2,12,24,38,59,
	74,91,100,103,105,105,105,105,105,105,105,105,
	0,0,0,0,0,0,0,0,0,0,5,19,35,55,72,92,
	104,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,0,0,0,0,0,1,9,25,46,68,88,103,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,0,0,0,0,3,13,28,50,73,95,103,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,0,0,0,2,15,31,57,81,100,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,0,0,0,13,30,60,84,102,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,0,0,10,33,63,86,103,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,0,7,25,56,83,102,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,0,3,20,45,80,101,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,3,13,37,68,99,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,0,7,24,58,89,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,2,18,40,79,103,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	0,8,29,65,94,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	2,17,43,77,102,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	6,26,62,90,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	11,37,71,101,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	18,47,84,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	26,64,93,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	33,69,100,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	39,76,103,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	44,84,105,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	49,88,105,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	57,93,105,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	61,96,105,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105,
	61,98,105,105,105,105,105,105,105,105,105,105,105,105,105,105,
	105,105,105,105,105,105,105,105,105,105,105,105
	};
