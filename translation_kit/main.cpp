// 	Travis Smith
// 	EECS 1540 001

#include "Team.h"
#include "TeamSorter.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

int main( char **argv, int argc )
{
	ifstream file("./teams.txt");
	TeamSorter teams;

	assert(file);	
	while(!file.eof())
	{
		Team temp;
		file >> temp;
		teams.addTeam( temp );
	}

	file.close();
	return 0;
}
