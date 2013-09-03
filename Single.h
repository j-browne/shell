#ifndef SINGLE_H
#define SINGLE_H

#include <iostream>
using namespace std;

class Single
{
	private:
		int p;
		int sigma;
	public:
		Single(const int, const int);
		int getp() const;
		int getsigma() const;
		bool operator== (const Single&) const;
};

ostream& operator<< (ostream& out, const Single& s);

#endif //SINGLE_H
