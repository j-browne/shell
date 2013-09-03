#include <iostream>
#include "Slater.h"
using namespace std;

Slater::Slater (): coeff(1), vector<bool>(singles.size(),0)
{
}

ostream& operator<< (ostream& out, const Slater& s)
{
	out << "|";
	for (int i = 0; i < s.size(); ++i)
	{
		out << s[i];
	}
	out << ">";
	return out;
}

bool Slater::operator== (const Slater& b) const
{
	if (this->size() != b.size())
	{
		return 0;
	}
	for (int i = 0; i < this->size(); ++i)
	{
		if ((*this)[i] != b[i])
		{
			return 0;
		}
	}
	return 1;
}

void Slater::setcoeff (double c)
{
	coeff = c;
	return;
}

double Slater::getcoeff () const
{
	return coeff;
}

Slater create(int i, const Slater& s)
{
	Slater s2 (s);
	int count = 0;

	// You can't add a second one
	if (s2[i])
	{
		s2.setcoeff (0);
	}

	s2[i] = 1;

	// Keep track of parity flipping
	for (int j = 0; j < i; ++j)
	{
		if (s2[j])
		{
			++count;
		}
	}

	if (count%2)
	{
		s2.setcoeff(-s2.getcoeff());
	}

	return s2;
}

Slater annihilate(int i, const Slater& s)
{
	Slater s2 (s);
	int count = 0;

	// You can't subtract when there is not one
	if (!s2[i])
	{
		s2.setcoeff (0);
	}

	s2[i] = 0;

	// Keep track of parity flipping
	for (int j = 0; j < i; ++j)
	{
		if (s2[j])
		{
			++count;
		}
	}

	if (count%2)
	{
		s2.setcoeff(-s2.getcoeff());
	}

	return s2;
}
