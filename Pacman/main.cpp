#include "Pacman.h"
#include <iostream>




//Entry Point for Application
int main(int argc, char* argv[]) {

	std::cout << "Which level would you like to play? There are currently 2 levels: ";
	int level;
	std::cin >> level;
	
	Pacman* game = new Pacman(argc, argv, level);
}