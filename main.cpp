#include "tgaimage.h"
#include "model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor green   = TGAColor(0, 255, 0, 255);
Model *model = NULL;
int *zBuffer = NULL;

const int width  = 800;
const int height = 800;
const int depth  = 255;
Vec3f light_dir(0,0,-1);

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

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image, float intensity, int *zBuffer) {
	if (t0.y == t1.y && t0.y == t2.y) return;
	if (t1.y > t0.y) {
		std::swap(t1, t0);
		std::swap(uv1, uv0);
	}
	if (t2.y > t0.y) {
		std::swap(t2, t0);
		std::swap(uv2, uv0);
	}
	if (t2.y > t1.y) {
		std::swap(t2, t1);
		std::swap(uv2, uv1);
	}

	Vec3i v02 = t0 - t2;
	Vec3i v12 = t1 - t2;
	Vec3i v01 = t0 - t1;
	Vec2i uv02 = uv0 - uv2;
	Vec2i uv12 = uv1 - uv2;
	Vec2i uv01 = uv0 - uv1;

	int totalHeight = t0.y - t2.y;

	for (int h = 0; h <= totalHeight; ++h) {
		bool first_half = h < v12.y;
		float alpha = h / (float) totalHeight;
		Vec3i A = Vec3i(t2.x + v02.x * alpha, t2.y + h, t2.z + v02.z * alpha);
		Vec2i uvA = uv2 + uv02*alpha;

		Vec3i &buf_vector = first_half ? v12 : v01;
		Vec2i &uv_buf_vector = first_half ? uv12 : uv01;
		Vec3i &start_vector = first_half ? t2 : t1;
		Vec2i &uv_start_vector = first_half ? uv2 : uv1;
		int b_h = first_half ? h : h - v12.y;

		float beta = buf_vector.y == 0 ? 1 : b_h / (float) buf_vector.y;
		Vec3i B = Vec3i(start_vector.x + buf_vector.x * beta, t2.y + h, start_vector.z + buf_vector.z * beta);
		Vec2i uvB = uv_start_vector + uv_buf_vector * beta;

		if (A.x > B.x) {
			std::swap(A, B);
			std::swap(uvA, uvB);
		}
		for (int x = A.x; x <= B.x; ++x) {
			Vec3i d_x = B - A;
			float psi = d_x.x == 0 ? 1. : (x - A.x) / (float) d_x.x;
			Vec3i P = Vec3i(A.x + d_x.x * psi, A.y, A.z + d_x.z * psi);
			int idx = width * (P.y - 1) + P.x;
			if (P.z > zBuffer[idx]) {
				zBuffer[idx] = P.z;
				Vec2i uvP = uvA + ((uvB - uvA) * psi);
				TGAColor color = model->diffuse(uvP);
				image.set(P.x, P.y, TGAColor(color.r*intensity, color.g*intensity, color.b*intensity, color.a));
			}
		}
	}

}

int main(int argc, char** argv) {

  model = new Model("Projects/Other/tinyrenderer/obj/african_head/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);

	zBuffer = new int[width * height];
	for (int i=0; i<width*height; i++) {
    zBuffer[i] = std::numeric_limits<int>::min();
  }

  for (int i=0; i<model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec3i screen_coords[3];
      Vec3f world_coords[3];
      for (int j=0; j<3; j++) {
          Vec3f v = model->vert(face[j]);
          screen_coords[j] = Vec3i((v.x+1.)*width/2., (v.y+1.)*height/2., (v.z+1.)*depth/2.);
          world_coords[j]  = v;
      }
      Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
      n.normalize();
      float intensity = n*light_dir;
			if (intensity>0) {
          Vec2i uv[3];
          for (int k=0; k<3; k++) {
              uv[k] = model->uv(i, k);
          }
          triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], image, intensity, zBuffer);
      }
  }

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("Projects/Other/tinyrenderer/output.tga");
	delete model;
	return 0;
}
