
#include <windows.h>
#include <vector>
#include <thread>
#include <functional>

#include <jconfig.h>
#include <jpeglib.h>


void AddMsgFunc(std::function<int(int, int)> f);
bool doParseMoreData(FILE* inputStream, int index);

typedef struct ScreenBuffer {
	void *pixels;
	int w;
	int h;
}ScreenBuffer;


static std::vector<ScreenBuffer> screenBuffer;
static UINT_PTR timer = 0;

void out2rect(void *data, int w1, int h1, HDC &hdc, RECT  &rect) {

	BITMAPINFO lpBmpInfo;
	lpBmpInfo.bmiHeader.biBitCount = 32;
	lpBmpInfo.bmiHeader.biClrImportant = 0;
	lpBmpInfo.bmiHeader.biClrUsed = 0;
	lpBmpInfo.bmiHeader.biCompression = BI_RGB;
	lpBmpInfo.bmiHeader.biPlanes = 1;
	lpBmpInfo.bmiHeader.biXPelsPerMeter = 0;
	lpBmpInfo.bmiHeader.biYPelsPerMeter = 0;
	lpBmpInfo.bmiHeader.biSize = sizeof(lpBmpInfo.bmiHeader);
	lpBmpInfo.bmiHeader.biWidth = w1;
	lpBmpInfo.bmiHeader.biHeight = h1;
	lpBmpInfo.bmiHeader.biSizeImage = w1 * h1 * 4;


	
	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchDIBits(hdc, rect.left, (rect.bottom - rect.top), rect.right - rect.left, -(rect.bottom - rect.top), 0, 0, w1, h1,
		data, &lpBmpInfo, DIB_RGB_COLORS, SRCCOPY);
	//int error=GetLastError();
	
	//TextOut(hdc,xxx,0,ss.GetBuffer(),ss.GetLength());
/*
	for(int j=0; j<h1; j++)
	{
		for(int k=0; k<w1; k++)
		{
			int  b=((BYTE*)data)[j*w1*2+k*2];
			int  g=((BYTE*)data)[j*w1*2+k*2+1];
			int  r=((BYTE*)data)[j*w1*2+k*2+2];
			SetPixel(hdc,k,j,RGB(r,g,b));
		}
	}
	*/
}




void RefreshView(HWND hWnd, HDC hdc) {
	if (timer <= 0) {
		int cxScreen, cyScreen;
		cxScreen = GetSystemMetrics(SM_CXSCREEN);
		cyScreen = GetSystemMetrics(SM_CYSCREEN);
		int w = 720 / 10 * 5*3;
		int h = 1080 / 10 * 5;
		timer = SetTimer(hWnd, 1, 50, 0);
		SetWindowPos(hWnd, 0, cxScreen / 2-w/2, cyScreen / 2-h/2, w, h, SWP_NOZORDER);
	}
	//DrawBitmap(hdc, face->glyph->bitmap);
	RECT rect;
	GetClientRect(hWnd, &rect);
	int w = rect.right / (screenBuffer.size());

	for (int i = 0; i < screenBuffer.size(); i++) {
		rect.left = i * w;
		rect.right = rect.left + w;
		out2rect(screenBuffer[i].pixels, screenBuffer[i].w, screenBuffer[i].h, hdc, rect);
	}
}



METHODDEF(void) jpeg_mem_error_exit(j_common_ptr cinfo) {
	// 调用 format_message 生成错误信息
	char err_msg[JMSG_LENGTH_MAX];
	//(*cinfo->err->format_message) (cinfo, err_msg);
	// 抛出c++异常
	//throw jpeg_mem_exception(err_msg);
}

void prepareBuffer(int index,int w,int h) {
	ScreenBuffer& screen = screenBuffer[index];
	if ((screen.w != w || screen.h != h) && screen.pixels != nullptr) {
		free(screen.pixels);
		screen.pixels = nullptr;
	}

	if (screen.pixels == nullptr) {
		screen.pixels = malloc(w*h * 4);
	}
	// 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
	screen.w = w;
	screen.h = h;
}

int testProgressive(int index, unsigned char* data, int size) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	int row_stride;
	unsigned char *buffer;

	cinfo.err = jpeg_std_error(&jerr);

	jerr.error_exit = jpeg_mem_error_exit;

	jpeg_create_decompress(&cinfo);


	//jpeg_stdio_src(&cinfo, infile);
	jpeg_mem_src(&cinfo, data, size);

	// 用jpeg_read_header获得jpg信息
	(void)jpeg_read_header(&cinfo, true);
	if (cinfo.err->msg_code != 0) {
		//jpeg_finish_decompress(&cinfo);
		//jpeg_destroy_decompress(&cinfo);
		//return 0;
	}
	/* 源信息 */
	printf("image_width = %d\n", cinfo.image_width);
	printf("image_height = %d\n", cinfo.image_height);
	printf("num_components = %d\n", cinfo.num_components);

	// 设置解压参数,比如放大、缩小
	//printf("enter scale M/N:\n");
	//scanf("%d/%d", &cinfo.scale_num, &cinfo.scale_denom);
	//printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);

	// 启动解压：jpeg_start_decompress   
	jpeg_start_decompress(&cinfo);

	/* 输出的图象的信息 */
	printf("output_width = %d\n", cinfo.output_width);
	printf("output_height = %d\n", cinfo.output_height);
	printf("output_components = %d\n", cinfo.output_components);//解压的是rgb，故为3元素

	// 一行的数据长度
	row_stride = cinfo.output_width * cinfo.output_components;

	
	buffer = (unsigned char*)malloc(row_stride);//分配空间用来存储一行数据
	prepareBuffer(index, cinfo.output_width , cinfo.output_height);
	ScreenBuffer& screen = screenBuffer[index];
	void *pixels = screen.pixels;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		int y = cinfo.output_scanline;
		(void)jpeg_read_scanlines(&cinfo, &buffer, 1);
		for (int i = 0; i < cinfo.output_width; i++) {
			int * p = (((int *)pixels) + (cinfo.output_width*y) + i);
			int strip = cinfo.output_components;
			unsigned char r = buffer[i*strip];
			unsigned char g = buffer[i*strip + 1];
			unsigned char b = buffer[i*strip + 2];
			*p = ((r << 16) & 0xff0000) | ((g << 8) & 0xff00) | ((b) & 0xff) | 0xff000000;
			//*p = 0xffff0000;
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	free(buffer);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
}



int testFile(int index,const char * arg) {
	FILE * infile;
	if ((infile = fopen(arg, "rb")) == NULL) {
		//fprintf(stderr, "can't open %s\n", argv[1]);
		return -1;
	}
	//std::this_thread::sleep_for(std::chrono::seconds(5));
	doParseMoreData(infile, index);
	fclose(infile);
}

int Again(int wp, int lp) {
	switch (wp) {
	case VK_SPACE:
		std::thread th = std::thread(testFile,0, "Readme/semi_progressive.jpg"); //defaultProgressive.jpg
		th.detach();
		std::thread th1 = std::thread(testFile,1, "Readme/YCBProgressive.jpg");
		th1.detach();
		std::thread th2 = std::thread(testFile,2, "Readme/steep_progressive.jpg");
		th2.detach();
		break;
	}

	return 0;
}



int main(int argc, const char* argv[]) {
	HWND hwnd = ::GetConsoleWindow();
	HINSTANCE instance = GetModuleHandle(NULL);
	//ShowWindow(hwnd, 0);

	std::function<int(int, int)> f = Again;
	AddMsgFunc(Again);


	ScreenBuffer buff1;
	ScreenBuffer buff2;
	ScreenBuffer buff3;
	screenBuffer.push_back(buff1);
	screenBuffer.push_back(buff2);
	screenBuffer.push_back(buff3);
	for (int i = 0; i < screenBuffer.size(); i++) {
		screenBuffer[i].pixels = nullptr;
	}

	Again(VK_SPACE,0);

	wWinMain(instance, NULL, NULL, 1);

	ShowWindow(hwnd, 1);
}