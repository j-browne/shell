#ifndef SLATER_H
#define SLATER_H

#include <iostream>
#include <vector>
#include "Single.h"
using namespace std;

extern int N;                  // number of particles
extern vector<Single> singles; // single particle states

// vector<bool> is optimized for memory usage
class Slater : public vector<bool>
{
	private:
		double coeff;
	public:
		Slater ();
		bool operator== (const Slater&) const;
		void setcoeff (double c);
		double getcoeff () const;
};

ostream& operator<< (ostream& out, const Slater& s);
Slater create(int i, const Slater& s);
Slater annihilate(int i, const Slater& s);

#endif //SLATER_H
