
#include "CG_Implementation.h"
#include <iostream>
#include <stdexcept>
#include <functional>


int main(){
	CG_Implementation app{};
	try{
		app.run();
	}catch (const std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
		system("PAUSE");
		return EXIT_FAILURE;
	}
	return 0;
}
