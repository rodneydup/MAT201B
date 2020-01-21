#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp" // al::Image
#include "al/math/al_Random.hpp"
using namespace al;

struct MyApp : App {
  Mesh mesh;
  int pixbehaviour = 0;
  std::vector<Vec3f> poseDefault;
  std::vector<Vec3f> poseRGB;
  std::vector<Vec3f> poseHSV;
  std::vector<float> explode;
  int counter;
  float gravity;

  void onCreate() override {
    mesh.primitive(Mesh::POINTS);

    const char *filename = "../CMS_Higgs-event.jpg";
    auto imageData = Image(filename);
    if (imageData.array().size() == 0) {
      std::cout << "failed to load image" << std::endl;
    }

    Image::RGBAPix pixel;

    for (int c = 0; c < imageData.width(); c++)
      for (int r = 0; r < imageData.height(); r++) {
        imageData.read(pixel, c, r);
        poseDefault.push_back(Vec3f(float(c) / imageData.width() * 2 - 1,
                                    float(r) / imageData.height() * 2 - 1, 0));
        mesh.vertex(float(c) / imageData.width() * 2 - 1,
                    float(r) / imageData.height() * 2 - 1, 0);
        mesh.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
        poseRGB.push_back(Vec3f(pixel.r / 127.5 - 1, pixel.g / 127.5 - 1,
                                pixel.b / 127.5 - 1));
        HSV HSVpixel =
            HSV(RGB(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0));
        poseHSV.push_back(Vec3f(HSVpixel.s * cos(HSVpixel.h * M_PI * 2),
                                HSVpixel.v * 2 - 1,
                                HSVpixel.s * sin(HSVpixel.h * M_PI * 2)));
        explode.push_back(HSVpixel.h);
      }
    nav().pos(0, 0, 6);
  }

  void onAnimate(double dt) override {
    if (pixbehaviour == 1) {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i].lerp(poseDefault[i], 0.02);
      }
    } else if (pixbehaviour == 2) {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i].lerp(poseRGB[i], 0.02);
      }
    } else if (pixbehaviour == 3) {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i].lerp(poseHSV[i], 0.02);
      }
    } else if (pixbehaviour == 4) {
      if (counter > -60)
        counter -= 1;
      if (gravity > -2)
        gravity -= 0.01;
      for (int i = 0; i < mesh.vertices().size(); i++) {
        rnd::Random<> rng;
        float r = rng.gaussian();
        float r2 = rng.gaussian();
        float dist = mesh.vertices()[i].sumAbs();
        if (counter > 0) {
          mesh.vertices()[i].lerp(mesh.vertices()[i].rotate(0.02 / dist * r),
                                  0.1);
          mesh.vertices()[i].lerp(0, 0.02);
        }
        if (counter < 0 && counter > -60) {
          gravity = 1;
          mesh.vertices()[i].lerp(
              Vec3f((explode[i] * 2 - 1) + (r * 4), gravity + r2, 0), 0.1);
        }
        if (counter == -60) {
          mesh.vertices()[i].lerp(
              Vec3f((explode[i] * 2 - 1) + (r * 4), gravity + r2, 0), 0.0005);
        }
      }
    } else if (pixbehaviour == 5) {

      for (int i = 0; i < mesh.vertices().size(); i++) {
        rnd::Random<> rng;
        float r = rng.uniform();
        float dist = mesh.vertices()[i].sumAbs();
        mesh.vertices()[i].lerp(mesh.vertices()[i].rotate(0.02 / dist * r),
                                0.1);
      }
    } else if (pixbehaviour == 6) {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        rnd::Random<> rng;
        float r = rng.uniform();
        float dist = mesh.vertices()[i].sumAbs();
        mesh.vertices()[i].lerp(
            Vec3f((mesh.vertices()[i].x * cos(M_PI / dist + r)) -
                      (mesh.vertices()[i].y * sin(M_PI / dist + r)),
                  (mesh.vertices()[i].x * sin(M_PI / dist + r)) -
                      (mesh.vertices()[i].y * cos(M_PI / dist + r)),
                  0),
            0.01);
      }
    } else if (pixbehaviour == 7) {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        rnd::Random<> rng;
        float r = rng.uniform();
        float dist = mesh.vertices()[i].sum();
        mesh.vertices()[i].lerp(
            Vec3f((mesh.vertices()[i].x * cos(M_PI / dist + r)) -
                      (mesh.vertices()[i].y * sin(M_PI / dist + r)),
                  (mesh.vertices()[i].x * sin(M_PI / dist + r)) -
                      (mesh.vertices()[i].y * cos(M_PI / dist + r)),
                  0),
            0.01);
      }
    } else if (pixbehaviour == 8) {
      for (int i = 0; i < mesh.vertices().size(); i++) {
        rnd::Random<> rng;
        float r = rng.uniform();
        float dist = mesh.vertices()[i].sumAbs();
        mesh.vertices()[i].lerp(
            Vec3f((mesh.vertices()[i].x * cos(M_PI * dist + r)) -
                      (mesh.vertices()[i].y * sin(M_PI * dist + r)),
                  (mesh.vertices()[i].x * sin(M_PI * dist + r)) -
                      (mesh.vertices()[i].y * cos(M_PI * dist + r)),
                  0),
            0.01);
      }
    }
    nav().faceToward(0);
  }

  void onDraw(Graphics &g) override {
    g.clear(0.1);
    g.meshColor();
    g.draw(mesh);
  }

  bool onKeyDown(const Keyboard &k) override {
    //
    switch (k.key()) {
    case '1':
      pixbehaviour = 1;
      break;
    case '2':
      pixbehaviour = 2;
      break;
    case '3':
      pixbehaviour = 3;
      break;
    case '4':
      counter = 180;
      pixbehaviour = 4;
      break;
    case '5':
      pixbehaviour = 5;
      break;
    case '6':
      pixbehaviour = 6;
      break;
    case '7':
      pixbehaviour = 7;
      break;
    case '8':
      pixbehaviour = 8;
      break;
    default:
      break;
    }
    return false;
  }
};

int main() {
  MyApp app;
  app.start();
}
