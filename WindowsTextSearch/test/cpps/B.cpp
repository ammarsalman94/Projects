
#include "B.h"
#include "A.h"

namespace A{
	Bee::Bee(){
		
	}
	
	Bee::~Bee(){
		
	}
	
}

#ifdef TEST_B
int main(){
	using namespace A;
	Ammar a;
	Something i = 10;
	Bee b;
	return 0;
}
#endif