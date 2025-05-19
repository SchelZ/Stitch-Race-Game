#include "game.h"


int main(int argc, char* argv[]) {
	Game game(argc, argv);
	if (!game.initialize()) return EXIT_FAILURE;
	game.run();
	return EXIT_SUCCESS;
}
