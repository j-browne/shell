#include <iostream>
#include "Single.h"
using namespace std;

int Single::getp() const
{
	return p;
}

int Single::getsigma() const
{
	return sigma;
}

Single::Single(const int a, const int b)
{
	p = a;
	sigma = b;
}

bool Single::operator== (const Single& b) const
{
	if (this->p == b.p && this->sigma == b.sigma)
	{
		return 1;
	}
}

ostream& operator<< (ostream& out, const Single& s)
{
	out << "|" << s.getp() << " " << s.getsigma() << ">";
	return out;
}
