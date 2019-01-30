#pragma once
class ColorHelp
{
public:
	//�ı���ɫ����
	static COLORREF ChangeLuminance(const COLORREF& color,const char& plus);

	static void RGBToYUV(const unsigned char& r, const unsigned char& g, const unsigned char& b
	             , char& y, char& u, char& v);
	static void YUVToRGB(const char& y, const char& u, const char& v
	             , unsigned char& r, unsigned char& g, unsigned char& b);

	//��COLORREFת��ΪRGB
	static inline void ExtractRGB(const COLORREF& color, unsigned char& r, unsigned char& g, unsigned char& b) {
		r = static_cast<unsigned char>(color & 0x000000FF);
		g = static_cast<unsigned char>((color & 0x0000FF00) >> 8);
		b = static_cast<unsigned char>((color & 0x00FF0000) >> 16);
	}

	//��ɫȡ��
	static COLORREF NegtiveColor(const COLORREF& color);
};

