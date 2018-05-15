#include <iostream>
#include <chrono>

void foo_constref(const std::string &s){
	s;
}
void foo_move(const std::string &&s){
	s;
}

auto main() -> int
{
	const auto MAX_REP {1000000};
	auto start = std::chrono::system_clock::now();
	for(size_t rep = 0; rep < MAX_REP; ++rep){
		std::string gp {"qwertyuiopasdfghjklzxcvbnm"};
		foo_constref(gp);
	}
	auto end   = std::chrono::system_clock::now();
	const std::chrono::duration<double> elapsed_seconds1 = end - start;

	start = std::chrono::system_clock::now();
	for(size_t rep = 0; rep < MAX_REP; ++rep){
		std::string gp {"qwertyuiopasdfghjklzxcvbnm"};
		foo_move(std::move(gp));
	}

	end   = std::chrono::system_clock::now();
	const std::chrono::duration<double> elapsed_seconds2 = end - start;
	
	
	
	std::cout <<"elapsed1 time: " << elapsed_seconds1.count() << '\n';
	std::cout <<"elapsed2 time: " << elapsed_seconds2.count() << '\n';


}
