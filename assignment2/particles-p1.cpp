#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp" // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f rv(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName); // forward declaration

struct AlloApp : App {
  // add more GUI here
  Parameter pointSize{"/pointSize", "", 1.0, "", 0.0, 2.0};
  Parameter timeStep{"/timeStep", "", 0.01, "", 0.01, 1.9};
  Parameter drag{"/drag", "", 0.01, "", 0, 1};

  ControlGUI gui;

  ShaderProgram pointShader;
  Mesh mesh;     // vector<Vec3f> position is inside mesh
  Texture trail; // frame feedback for trails

  // typedef al::Vec<float, 3> Vec3f;
  // typedef std::vector<Vec3f> Vertices;

  //  simulation state
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;
  const double GM = 398600.4418; // Geocentric gravitational constant (gravity
                                 // per earth mass) in km^3/s^2

  void onCreate() override {
    // add more GUI here
    gui << pointSize << timeStep << drag;
    gui.init();
    navControl().useMouse(false);

    trail.filter(Texture::LINEAR);

    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    // set initial conditions of the simulation
    //

    // c++11 "lambda" function
    auto rc = []() { return HSV(rnd::uniform(), 1.0f, 1.0f); };

    mesh.primitive(Mesh::POINTS);
    // does 1000 work on your system? how many can you make before you get a low
    // frame rate? do you need to use <1000?
    for (int _ = 0; _ < 1000; _++) {
      mesh.vertex(rv(5));
      mesh.color(rc());

      // float m = rnd::uniform(3.0, 0.5);
      //   float m = 3 + rnd::normal() / 2;
      //   if (m < 0.5) m = 0.5;
      float m = 1; //
      mass.push_back(m);

      // using a simplified volume/size relationship
      mesh.texCoord(pow(m, 1.0f / 3), 0); // s, t

      // separate state arrays
      velocity.push_back(rv(0.1));
      acceleration.push_back(rv(1));
    }

    nav().pos(0, 0, 10);
  }

  bool freeze = false;
  void onAnimate(double dt) override {
    if (freeze)
      return;

    // ignore the real dt and set the time step;
    dt = timeStep;

    // Calculate forces
    for (int i = 0; i < velocity.size(); i++) {
      // drag
      acceleration[i] -= velocity[i] * drag;
      // Inter-particle gravitation
        for (int j = i + 1; j < velocity.size(); j++) {
          Vec3f r = mesh.vertices()[i] - mesh.vertices()[j];
          Vec3d rhat = r.normalize();
          double distsqr = r.magSqr() * 1000000000;
          acceleration[i] -= ((GM) / distsqr) * rhat;
          acceleration[j] += ((GM) / distsqr) * rhat;
        }
    }
    // Integration
    //
    vector<Vec3f> &position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      // "backward" Euler integration
      velocity[i] += acceleration[i] / mass[i] * dt;
      position[i] += velocity[i] * dt;

      // Explicit (or "forward") Euler integration would look like this:
      // position[i] += velocity[i] * dt;
      // velocity[i] += acceleration[i] / mass[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration)
      a.zero();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') {
      // introduce some "random" forces
      for (int i = 0; i < velocity.size(); i++) {
        // F = ma
        acceleration[i] = rv(1) / mass[i];
      }
    }

    return true;
  }

  void onDraw(Graphics &g) override {
    g.clear(0);

    // 1. Match texture dimensions to window
    trail.resize(fbWidth(), fbHeight());
    // 2. Draw feedback texture.
    g.tint(0.96);

    g.quadViewport(trail);
    g.tint(1); // set tint back to 1

    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.blending(true);
    g.blendModeTrans();
    g.depthTesting(true);

    g.draw(mesh);
    g.depthTesting(false); // important - turn off depth testing so the fbo doesn't occlude the mesh

    trail.copyFrameBuffer(); // 3. Copy current (read) frame buffer to texture

    imguiBeginFrame();
    ParameterGUI::beginPanel("Control");
    ParameterGUI::drawParameter(&pointSize);
    ParameterGUI::drawParameter(&timeStep);
    ParameterGUI::drawParameter(&drag);
    ImGui::Text("Framerate %.3f", ImGui::GetIO().Framerate);
    ParameterGUI::endPanel();
    imguiEndFrame();
    imguiDraw();
  }
};

int main() { AlloApp().start(); }

string slurp(string fileName) {
  fstream file(fileName);
  string returnValue = "";
  while (file.good()) {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}