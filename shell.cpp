/*
 * Usage: ./shell < input_file
 ***************************************************************
 * INPUT
 ***************************************************************
 ** input_file:
 ** four lines, read from stdin, lines starting with '#' are ignored
 ** line 1: states_file
 ** line 2: number of particles
 ** line 3: maximum number of broken pairs in a slater determinant
 ** line 4: value of g (pairing strength)
 ** line 5: output_path (folder to store output files)
 ***************************************************************
 ** states_file:
 ** a file with the single particle states
 ** lines starting with '#' are ignored
 ** one line per state
 ** two columns per line, first column is p, second column is sigma
 ***************************************************************
 * OUTPUT
 ***************************************************************
 ** output_path/sps.txt: single particle states
 ** output_path/nps.txt: N-particle states
 ** output_path/H.txt: Hamiltonian matrix
 ** output_path/eigen.txt: eigenvalues
 ** output_path/eigen_less.txt: condensed eigenvalues
 ***************************************************************
 * Known Issues:
 * states_file and output folder do not accept whitespace
 */

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <armadillo>
#include <boost/filesystem.hpp>
#include "Single.h"
#include "Slater.h"
using namespace std;
using namespace arma;

/*
 * Globals, just because I don't want to be passing a million arguments
 */
double g;               // strength of the pairing term
vector<Single> singles; // single particle states
vector<Slater> sd;      // slater determinants
int N;                  // number of particles
int max_broken_pairs;   // maximum number of broken pairs in a slater determinant

void create_slaters ();
void create_slaters (int depth, int begin, Slater s);
bool valid_slater (const Slater& s);
double Hamiltonian (Slater a, Slater b);
int broken_pairs (const Slater& s);
int total_spin (const Slater& s);

int main()
{
	ifstream states_file;   // file with single particle states
	string states_filename; // filename for states_file
	string output_path;     // path for output files
	mat H;                  // Hamiltonian matrix
	vec eigenval;           // eigenvalues of the Hamiltonian

	// Read in from stdin
	// line 1: states_file
	// line 2: number of particles
	// line 3: maximum number of broken pairs in a slater determinant
	// line 4: value of g (pairing strength)
	// line 5: folder to store output files
	// FIXME: Allow for whitespace in filename/paths
	for (int i = 0; i < 5;)
	{
		if (!cin.good() || cin.peek() == EOF)
		{
			cerr << "Bad input" << endl;
			return 1;
		}
		if (cin.peek() == '#')
		{
			cin.ignore(256, '\n');
			continue;
		}
		else if (i == 0)
		{
			cin >> states_filename;
			cin.ignore(256, '\n');
			++i;
		}
		else if (i == 1)
		{
			cin >> N;
			cin.ignore(256, '\n');
			++i;
		}
		else if (i == 2)
		{
			cin >> max_broken_pairs;
			cin.ignore(256, '\n');
			++i;
		}
		else if (i == 3)
		{
			cin >> g;
			cin.ignore(256, '\n');
			++i;
		}
		else if (i == 4)
		{
			cin >> output_path;
			cin.ignore(256, '\n');
			++i;
		}
	}
	cin.ignore(1024);

	// Read in from states_file
	// lines starting with '#' are ignored
	// one state per line, with columns: p, sigma
	states_file.open(states_filename.c_str());
	if (!states_file.good())
	{
		cerr << "Error opening file " << states_filename << endl;
		return 1;
	}
	while(states_file.good() && states_file.peek() != EOF)
	{
		int p;
		int s;

		if (states_file.peek() == '#')
		{
			states_file.ignore(256, '\n');
		}
		else
		{
			states_file >> p >> s;
			states_file.ignore(256, '\n');
			singles.push_back(Single(p, s));
		}
	}
	states_file.close();

	// Sanity Check
	// Make sure the number of states is greater than the number of particles
	if (singles.size() < N)
	{
		cerr << "The number of single particle states is less than the number of particles" << endl;
		return 1;
	}

	// Create the Slater determinants
	create_slaters ();
	cout << sd.size() << " Slater determinants" << endl;


	// Create the Hamiltonian
	{
		H.set_size(sd.size(),sd.size());
		int i = 0;
		int j = 0;
		for (vector<Slater>::iterator a = sd.begin(); a != sd.end(); ++a, ++i)
		{
			j=0;
			for (vector<Slater>::iterator b = sd.begin(); b != sd.end(); ++b, ++j)
			{
				H.at(i,j) = Hamiltonian(*a, *b);
			}
		}
	}

	// Calculate Eigenvalues
	eig_sym (eigenval, H);

	// Output
	boost::filesystem::create_directories(output_path);
	ofstream outfile;

	outfile.open((output_path+"/sps.txt").c_str());
	if (!outfile.good())
	{
		cerr << "Error opening file " << output_path+"/sps.txt" << endl;
		return 1;
	}
	outfile << "# 1-particle states" << endl;
	for (vector<Single>::const_iterator it = singles.begin(); it != singles.end(); ++it)
	{
		outfile << *it << endl;
	}
	outfile.close();

	outfile.open((output_path+"/nps.txt").c_str());
	if (!outfile.good())
	{
		cerr << "Error opening file " << output_path+"/nps.txt" << endl;
		return 1;
	}
	outfile << "# " << N << "-particle states" << endl;
	for (vector<Slater>::iterator it = sd.begin(); it != sd.end(); ++it)
	{
		outfile << *it << endl;
	}
	outfile.close();

	outfile.open((output_path+"/H.txt").c_str());
	if (!outfile.good())
	{
		cerr << "Error opening file " << output_path+"/H.txt" << endl;
		return 1;
	}
	outfile << "# Hamiltonian" << endl << H << endl;
	outfile.close();

	outfile.open((output_path+"/eigen.txt").c_str());
	if (!outfile.good())
	{
		cerr << "Error opening file " << output_path+"/eigen.txt" << endl;
		return 1;
	}
	outfile <<  "# eigenvalues" << endl << eigenval << endl;
	outfile.close();

	outfile.open((output_path+"/eigen_less.txt").c_str());
	if (!outfile.good())
	{
		cerr << "Error opening file " << output_path+"/eigen_less.txt" << endl;
		return 1;
	}
	outfile <<  "# eigenvalues" << endl;
	outfile << eigenval[0] << endl;
	for (int i = 1; i < eigenval.size(); ++i)
	{
		if (abs(eigenval[i] - eigenval[i-1]) > .0001*eigenval[i-1])
		{
			outfile << eigenval[i] << endl;
		}
	}
	outfile.close();

	return 0;
}

/*
 * Creates the slater determinants from a vector of single-particle states
 * It is recursive so it handles an arbitrary number of particles.
 */
void create_slaters ()
{
	int depth = N-1;
	int begin = 0;
	Slater s;

	for (int i = begin; i < singles.size(); ++i)
	{
		s[i] = 1;
		if (depth == 0)
		{
			if (valid_slater(s))
			{
				sd.push_back(s);
			}
		}
		else
		{
			create_slaters (depth-1, i+1, s);
		}
		s[i] = 0;
	}
	return;
}

void create_slaters (int depth, int begin, Slater s)
{
	for (int i = begin; i < singles.size(); ++i)
	{
		s[i] = 1;
		if (depth == 0)
		{
			if (valid_slater(s))
			{
				sd.push_back(s);
			}
		}
		else
		{
			create_slaters (depth-1, i+1, s);
		}
		s[i] = 0;
	}
	return;
}

/*
 * This determines the criteria for a slater determinant to be included.
 * A max of two broken pairs are allowed
 */
bool valid_slater (const Slater& s)
{
	if (broken_pairs (s) > max_broken_pairs)
	{
		return 0;
	}
	if (total_spin (s) != 0)
	{
		return 0;
	}
	return 1;
}

int broken_pairs (const Slater& s)
{
	int broken = 0;
	map<pair<int,int>, int> sums;
	for (int i = 0; i < s.size(); ++i)
	{
		if (s[i])
		{
			sums[pair<int, int>(singles[i].getp(), abs(singles[i].getsigma()))] += ((singles[i].getsigma() > 0)?(1):(-1));
		}
	}
	for (map<pair<int,int>, int>::iterator it = sums.begin(); it != sums.end(); ++it)
	{
		if (it->second != 0)
		{
			++broken;
		}
	}

	// Account for double counting
	broken /= 2;
	return broken;
}

int total_spin (const Slater& s)
{
	int m = 0;
	for (int i = 0; i < s.size(); ++i)
	{
		if (s[i])
		{
			m += singles[i].getsigma();
		}
	}

	return m;
}

/*
 * This computes the Hamiltonian matrix element <a|H|b>
 */
double Hamiltonian (Slater a, Slater b)
{
	double E=0;

	// Check to make sure this is even possible
	if (a.size() != b.size())
	{
		return 0;
	}

	// <a|(p-1)N_p|b>
	if (a==b)
	{
		for (int i = 0; i < singles.size(); ++i)
		{
			if (b[i])
			{
				E += (singles[i].getp())-1;
			}
		}
	}

	// -g/2 <a|P_p'^+ P_p^-|b>
	for (int i = 0; i < singles.size(); ++i)
	{
		// i is positive
		if (singles[i].getsigma() > 0)
		{
			continue;
		}
		for (int j = 0; j < singles.size(); ++j)
		{
			// i and j are a pair
			if (singles[i].getp() != singles[j].getp())
			{
				continue;
			}
			// j is negative
			if (singles[j].getsigma() < 0)
			{
				continue;
			}
			for (int k = 0; k < singles.size(); ++k)
			{
				// k is positive
				if (singles[k].getsigma() > 0)
				{
					continue;
				}
				for (int l = 0; l < singles.size(); ++l)
				{
					// k and l are a pair
					if (singles[k].getp() != singles[l].getp())
					{
						continue;
					}
					// l is negative
					if (singles[l].getsigma() < 0)
					{
						continue;
					}

					// Don't do the work if it's going to be zero
					if (a[i] && a[j] && b[k] && b[l])
					{
						Slater s1 = annihilate (j, annihilate(i, a));
						Slater s2 = annihilate (l, annihilate(k, b));

						if (s1 == s2)
						{
							E -= s1.getcoeff() * s2.getcoeff() * g/2;
						}
					}
				}
			}
		}
	}

	return E;
}
