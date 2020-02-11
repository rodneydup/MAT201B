#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName);  // forward declaration

// suggested structure for Agent
//
struct Agent : Pose {
  Vec3f flockHeading, flockCenter;  // of the local flock! averages
  unsigned flockCount{1};
};

struct AlloApp : App {
  // add more GUI here
  Parameter Separation{"Separation", "", 0.04, "", 0.0, 0.05};
  Parameter Alignment{"Alignment", "", 0.045, "", 0.0, 0.05};
  Parameter Cohesion{"Cohesion", "", 0.005, "", 0.0, 0.05};
  Parameter moveRate{"Move Rate", "", 1.0, "", 0.0, 2.0};
  Parameter localRadius{"Local Radius", "", 0.16, "", 0.01, 0.9};
  Parameter size{"Size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"Ratio", "", 1.0, "", 0.0, 2.0};
  ParameterString frameRate{"Frame Rate","", "60", ""};
  ControlGUI gui;

  ShaderProgram shader;
  Mesh mesh;

  vector<Agent> agent;

  void onCreate() override {
    // add more GUI here
    gui << Separation << Alignment << Cohesion << moveRate << localRadius << size << ratio << frameRate;
    gui.init();
    navControl().useMouse(false);

    // compile shaders
    shader.compile(slurp("../tetrahedron-vertex.glsl"),
                   slurp("../tetrahedron-fragment.glsl"),
                   slurp("../tetrahedron-geometry.glsl"));

    mesh.primitive(Mesh::POINTS);

    for (int _ = 0; _ < 500; _++) {
      Agent a;
      a.pos(rv(1.5));
      a.faceToward(rv());
      agent.push_back(a);
      //
      mesh.vertex(a.pos());
      mesh.normal(a.uf());
      const Vec3f& up(a.uu());
      mesh.color(up.x, up.y, up.z);
    }

    nav().pos(0, 0, 5);
  }

  void onAnimate(double dt) override {
    // for each pair of agents
    //
    int N = agent.size();
    for (unsigned i = 0; i < N; i++)
      for (unsigned j = 1 + i; j < N; j++) {
        float distance = (agent[j].pos() - agent[i].pos()).mag();
        if (distance < localRadius) {
          // add to flock count
          agent[i].flockCount += 1;
          agent[j].flockCount += 1;
          // add positions to each other's Flock Center for averaging later (Cohesion)
          agent[i].flockCenter += agent[j].pos();
          agent[j].flockCenter += agent[i].pos();
          // add headings to each other's Flock headings for averaging later (Alignment)
          agent[i].flockHeading += agent[j].uf();
          agent[j].flockHeading += agent[i].uf();
          // Separation
          distance < 2 ? distance = 2 : NULL;
          agent[i].faceToward(agent[j].pos(), (-1/distance) * Separation);
          agent[j].faceToward(agent[i].pos(), (-1/distance) * Separation);
        }
      }

    // only once the above loop is done do we have good data on average headings
    // and centers

    //
    // put code here
    //
    //
    for (unsigned i = 0; i < N; i++) {
      if (agent[i].flockCount > 0) {
      // average flock positions and steer toward that (Cohesion)
      agent[i].flockCenter /= agent[i].flockCount;
      agent[i].faceToward(agent[i].flockCenter, Cohesion);
      // average flock headings and steer toward that (Alignment)
      agent[i].flockHeading /= agent[i].flockCount;
      agent[i].faceToward(agent[i].flockHeading, Alignment);
      // move the agent
      agent[i].pos() += agent[i].uf() * moveRate * 0.002 * agent[i].flockCount;
      }
      agent[i].pos() += agent[i].uf() * moveRate * 0.001;
      // reset stuff
      agent[i].flockCenter = Vec3f(0,0,0);
      agent[i].flockHeading = Vec3f(0,0,0);
      agent[i].flockCount = 0;
    }

    // TURN agents if they go too far
    //
    for (unsigned i = 0; i < N; i++) {
      if (agent[i].pos().mag() > 5) {
        agent[i].faceToward((0,0,0),0.1);
      }
    }

    // visualize the agents
    //
    vector<Vec3f>& v(mesh.vertices());
    vector<Vec3f>& n(mesh.normals());
    vector<Color>& c(mesh.colors());
    for (unsigned i = 0; i < N; i++) {
      v[i] = agent[i].pos();
      n[i] = agent[i].uf();
      const Vec3d& up(agent[i].uu());
      c[i].set(up.x, up.y, up.z);
    }
  }

  void onDraw(Graphics& g) override {
    g.clear(0.1, 0.1, 0.1);
    // gl::depthTesting(true); // or g.depthTesting(true);
    // gl::blending(true); // or g.blending(true);
    // gl::blendTrans(); // or g.blendModeTrans();
    g.shader(shader);
    g.shader().uniform("size", size * 0.03);
    g.shader().uniform("ratio", ratio * 0.2);
    g.draw(mesh);
    frameRate.set(toString(ImGui::GetIO().Framerate));
    gui.draw(g);
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
