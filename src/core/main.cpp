#include "scene.h"
#include "parser.h"

int main() {
	Aya::Scene *scene = new Aya::Scene();
	Aya::Parser parser;
	parser.load("test.json");
	parser.run(scene);
	scene->render("parser.ppm");
	return 0;
}