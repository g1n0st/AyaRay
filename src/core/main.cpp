#include "scene.h"
#include "parser.h"

void ayaInit() {
	Aya::SampledSpectrum::init();
}
void ayaCleanUp() {

}
int main(int argc, char **argv) {
	if (argc != 3) {
		printf("the argv is error.\n");
		exit(1);
	}

	ayaInit();

	Aya::Scene *scene = new Aya::Scene();
	Aya::Parser parser;
	parser.load(argv[1]);
	parser.run(scene);
	scene->render(argv[2]);

	ayaCleanUp();

	system("PAUSE");
	return 0;
}