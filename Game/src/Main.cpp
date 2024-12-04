#include <Engine.h>

int add(int a, int b) {
	return a + b;
}

int main()
{
	Engine::Hello();
	int result = add(1, 1);
	std::cout << "hello world, 1+1 is " << result << std::endl;
	return 0;
}