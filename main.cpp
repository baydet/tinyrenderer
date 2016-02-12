#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool reversed = false;
	if (std::abs(x1-x0) < std::abs(y1-y0)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		reversed = true;
	}
	if (x0 > x1) std::swap(x0, x1);
	for (int x = x0; x <= x1; ++x) {
		int y = ((x - x0) * (y1 - y0)) / (float)(x1-x0) + y0;
		if (reversed) {
			image.set(y, x, color);
		} else {
			image.set(x, y, color);
		}
	}
}

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	line(13, 20, 80, 40, image, white);
	line(20, 13, 40, 80, image, red);
	line(80, 40, 13, 20, image, red);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("Projects/Other/tinyrenderer/test.tga");
	return 0;
}
