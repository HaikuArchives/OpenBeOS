// Team.cpp: implementation of the Team class.
//
//////////////////////////////////////////////////////////////////////

#include "Team.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Team::Team( string name, 
			string nickname,
			string division, 
			string conference )
{
	_szName = name;
	_szNickname = nickname;
	_szConference = conference;
	_szDivision = division;

	_ZeroScores();

	return;

}

Team::Team( istream & ios )
{
	getline( ios, _szCoach );
	getline( ios, _szNickname );
	getline( ios, _szName );
	getline( ios, _szConference );
	getline( ios, _szDivision );

	ios >> _iConferenceWins;
	ios >> _iConferenceLosses;
	ios >> _iConferenceTies;
	ios >> _iOtherWins;
	ios >> _iOtherLosses;
	ios >> _iOtherTies;

	return;
} 

Team::Team( const Team & source )
{
	_szName = source.getName();
	_szNickname = source.getNickname();
	_szCoach = source.getCoach();
	_szConference = source.getConference();
	_szDivision = source.getDivision();

	_iConferenceLosses = source.getConferenceLosses();
	_iConferenceTies = source.getConferenceTies();
	_iConferenceWins = source.getConferenceWins();
	_iOtherLosses = source.getAllLosses();
	_iOtherTies = source.getAllTies();
	_iOtherWins = source.getAllWins();

	return;
}	// end team( team & )

Team::~Team()
{

}

void Team::_ZeroScores()
{
	_iConferenceLosses = 0;
	_iConferenceTies = 0;
	_iConferenceWins = 0;
	_iOtherLosses = 0;
	_iOtherTies = 0;
	_iOtherWins = 0;
}

string Team::getName() const
{
	return _szName;
}

string Team::getCoach() const
{
	return _szCoach;
}

string Team::getConference() const
{	
	return _szConference;
}

string Team::getDivision() const
{
	return _szDivision;
}

string Team::getNickname() const
{
	return _szNickname;
}

int Team::getConferenceLosses() const
{
	return _iConferenceLosses;
}

int Team::getConferenceTies() const
{
	return _iConferenceTies;
}

int Team::getConferenceWins() const
{
	return _iConferenceWins;
}

int Team::getAllWins() const
{
	return _iOtherWins;
}

int Team::getAllLosses() const
{
	return _iOtherLosses;
}

int Team::getAllTies() const
{
	return _iOtherTies;
}

void Team::addNonConferenceWin()
{
	_iOtherWins++;
}

void Team::addConferenceWin()
{
	_iOtherWins++;
	_iConferenceWins++;
}

void Team::addNonConferenceLoss()
{
	_iOtherLosses++;
}

void Team::addConferenceLoss()
{
	_iOtherLosses++;
	_iConferenceLosses++;
}

void Team::addNonConferenceTie()
{
	_iOtherTies++;
}

void Team::addConferenceTie()
{
	_iOtherTies++;
	_iConferenceTies++;
}

void Team::changeCoach(string name)
{
	_szCoach = name;
}

istream & operator >> ( istream & ins, class Team & target )
{
	class Team temp( ins );
	target = temp;
	return ins;
}

ostream & operator << ( ostream & outs, class Team & s )
{
	outs << s.getName() << endl;
	outs << s.getNickname() << endl;
	outs << s.getCoach() << endl;
	outs << s.getConference() << endl;
	outs << s.getDivision() << endl;

	outs << s.getConferenceLosses();
	outs << s.getConferenceTies();
	outs << s.getConferenceWins();
	outs << s.getAllLosses();
	outs << s.getAllTies();
	outs << s.getAllWins();

	return outs;
}
