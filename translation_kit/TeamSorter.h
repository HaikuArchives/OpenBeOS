// TeamSorter.h: interface for the TeamSorter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEAMSORTER_H__2B727420_2397_423B_B544_4DC00A3514FB__INCLUDED_)
#define AFX_TEAMSORTER_H__2B727420_2397_423B_B544_4DC00A3514FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <strstream>
#include <vector>
#include "Team.h"

using std::vector;

class TeamSorter  
{
public:
	string * getSortedString();
	void sortByConference();
	void sortByAllRecord();
	void sortByConferenceRecord();
	void sortByName();
	int getNumTeams();
	class Team getTeams(int k);
	void addTeam(class Team & t);
	TeamSorter();
	virtual ~TeamSorter();

private:
	vector<Team> _pTeams;
};

#endif // !defined(AFX_TEAMSORTER_H__2B727420_2397_423B_B544_4DC00A3514FB__INCLUDED_)
