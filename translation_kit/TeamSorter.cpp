// TeamSorter.cpp: implementation of the TeamSorter class.
//
//////////////////////////////////////////////////////////////////////

#include "TeamSorter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TeamSorter::TeamSorter()
{
}

TeamSorter::~TeamSorter()
{
}

void TeamSorter::addTeam(Team &t)
{
	_pTeams.insert(0,t);
}

class Team TeamSorter::getTeams(int k)
{
	return _pTeams[k];
}

int TeamSorter::getNumTeams()
{
	return _pTeams.size();
}

void TeamSorter::sortByName()
{
	int len = _pTeams.size();
	vector<Team> temp;
	for(int i = 0; i < len; i++)
	{
		int max = 0;
		for(int j = 0 ; j < (int) _pTeams.size(); j++)
		{
			if(_pTeams[max].getName() > _pTeams[j].getName())
			{
				max = j;
			}
		}
		temp.insert(0, _pTeams[max]);
		_pTeams.erase(&temp[0]);
	}

	_pTeams = temp;
}

void TeamSorter::sortByConferenceRecord()
{
	int len = _pTeams.size();
	vector<Team> temp;
	for(int i = 0; i < len; i++)
	{
		int max = 0;
		for(int j = 0 ; j < (int) _pTeams.size(); j++)
		{
			if(_pTeams[max].getConferenceWins() > _pTeams[j].getConferenceWins())
			{
				max = j;
			}
		}
		temp.insert(0, _pTeams[max]);
		_pTeams.erase(&temp[0]);
	}

	_pTeams = temp;
}

void TeamSorter::sortByAllRecord()
{
	int len = _pTeams.size();
	vector<Team> temp;
	for(int i = 0; i < len; i++)
	{
		int max = 0;
		for(int j = 0 ; j < (int) _pTeams.size(); j++)
		{
			if(_pTeams[max].getAllWins() > _pTeams[j].getAllWins())
			{
				max = j;
			}
		}
		temp.insert(0, _pTeams[max]);
		_pTeams.erase(&temp[0]);
	}

	_pTeams = temp;
}

void TeamSorter::sortByConference()
{
	int len = _pTeams.size();
	vector<Team> temp;
	for(int i = 0; i < len; i++)
	{
		int max = 0;
		for(int j = 0 ; j < (int) _pTeams.size(); j++)
		{
			if(_pTeams[max].getConference() > _pTeams[j].getConference())
			{
				max = j;
			}
		}
		temp.insert(0, _pTeams[max]);
		_pTeams.erase(&temp[0]);
	}

	_pTeams = temp;

}

string * TeamSorter::getSortedString()
{
	return NULL;
}
