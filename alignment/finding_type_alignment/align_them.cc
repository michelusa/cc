#include <iostream>

auto main() -> int
{
	std::cout << "align(bool): " << alignof(bool);
	std::cout << '\n';

	std::cout << "align(char): " << alignof(char);
	std::cout << '\n';

	std::cout << "align(short): " << alignof(short);
	std::cout << '\n';

	std::cout << "align(long): " << alignof(long);
	std::cout << '\n';

	std::cout << "align(void*): " << alignof(void*);
	std::cout << '\n';
}

