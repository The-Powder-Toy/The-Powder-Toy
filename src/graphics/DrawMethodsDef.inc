	int drawtext(int x, int y, const char *s, int r, int g, int b, int a);
	int drawtext(int x, int y, std::string s, int r, int g, int b, int a);
	int drawchar(int x, int y, int c, int r, int g, int b, int a);
	int addchar(int x, int y, int c, int r, int g, int b, int a);

	void xor_pixel(int x, int y);
	void xor_line(int x, int y, int x2, int y2);
	void xor_rect(int x, int y, int width, int height);
	void xor_bitmap(unsigned char * bitmap, int x, int y, int w, int h);

	void draw_line(int x, int y, int x2, int y2, int r, int g, int b, int a);
	void drawrect(int x, int y, int width, int height, int r, int g, int b, int a);
	void fillrect(int x, int y, int width, int height, int r, int g, int b, int a);
	void clearrect(int x, int y, int width, int height);
	void gradientrect(int x, int y, int width, int height, int r, int g, int b, int a, int r2, int g2, int b2, int a2);

	void draw_image(pixel *img, int x, int y, int w, int h, int a);
