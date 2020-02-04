#include "scene.h"
#include "parser.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("the argv is error.\n");
		exit(1);
	}

	Aya::Scene *scene = new Aya::Scene();
	Aya::Parser parser;
	parser.load(argv[1]);
	parser.run(scene);
	scene->render(argv[2]);
	return 0;
}