

#include "A.h"
#include "B.h"

namespace A{
	Ammar::Ammar(){
		
	}
	
	Ammar::~Ammar(){
		
	}
	
	void Ammar::AFunc(){
		B::AFunction();
		B::AFunction(10);
	}
	
}

#ifdef TEST_A
int main(){
	using namespace A;
	
	Ammar a;
	a.AFunc();
	
	return 0;
	
}
#endif