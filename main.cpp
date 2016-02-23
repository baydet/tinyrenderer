#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor green   = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool reversed = false;
	if (std::abs(x1-x0) < std::abs(y1-y0)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		reversed = true;
	}
	if (x0>x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = std::abs(y1 - y0);
	int derr = dy * 2;
	int err = 0;
	int y = y0;
	for (int x = x0; x <= x1; ++x) {
		if (reversed) {
			image.set(y, x, color);
		} else {
			image.set(x, y, color);
		}
		err += derr;

		if (err > dx) {
			y += y1 > y0 ? 1 : -1;
			err -= 2 * dx;
		}

	}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
	if (t1.y > t0.y) std::swap(t1, t0);
	if (t2.y > t0.y) std::swap(t2, t0);
	if (t2.y > t1.y) std::swap(t2, t1);

	//todo remove shitcode
	if (t1.y == t2.y) return;
	if (t0.y == t2.y) return;
	if (t0.y == t1.y) return;

	float a02 = (t0.x - t2.x) / (float) (t0.y - t2.y);
	float a12 = (t1.x - t2.x) / (float) (t1.y - t2.y);
	float a01 = (t0.x - t1.x) / (float) (t0.y - t1.y);

	for (int y = t2.y; y <= t0.y; ++y) {
		int x0 = a02 * (y - t2.y) + t2.x;
		int x1 = y < t1.y ? a12 * (y - t2.y) + t2.x : a01 * (y - t1.y) + t1.x;
		if (x0>x1) std::swap(x0, x1);
		for (int x = x0; x < x1; ++x) {
			image.set(x, y, color);
		}
	}

}

int main(int argc, char** argv) {

  model = new Model("Projects/Other/tinyrenderer/obj/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);

	for (int i=0; i<model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec2i screen_coords[3];
      for (int j=0; j<3; j++) {
          Vec3f world_coords = model->vert(face[j]);
          screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.);
      }
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
  }

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("Projects/Other/tinyrenderer/output.tga");
	delete model;
	return 0;
}
