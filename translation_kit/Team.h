// Team.h: interface for the Team class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEAM_H__C91C35E7_773D_410E_851E_259AC2DC8BBA__INCLUDED_)
#define AFX_TEAM_H__C91C35E7_773D_410E_851E_259AC2DC8BBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <iostream>

using std::string;
using std::istream;
using std::ostream;
using std::getline;
using std::endl;

class Team  
{
public:
	void changeCoach( string name );
	void addConferenceTie();
	void addNonConferenceTie();
	void addConferenceLoss();
	void addNonConferenceLoss();
	void addConferenceWin();
	void addNonConferenceWin();
	int getAllTies() const;
	int getAllLosses() const;
	int getAllWins() const;
	int getConferenceWins() const;
	int getConferenceTies() const;
	int getConferenceLosses() const;
	string getNickname() const;
	string getDivision() const;
	string getConference() const;
	string getCoach() const;
	string getName() const;
	Team(string name = "none", string nickname = "none", string division = "none", string conference = "none");
	Team(istream & ios);
	Team(const Team & source);
	virtual ~Team();

	friend istream & operator >> ( istream & ins, class Team & target );
	friend ostream & operator << ( ostream & outs, class Team & source );


private:
	int _iConferenceWins;
	string _szCoach;
	string _szDivision;
	string _szConference;
	string _szNickname;
	string _szName;
	void _ZeroScores();
	int _iOtherTies;
	int _iOtherWins;
	int _iOtherLosses;
	int _iConferenceTies;
	int _iConferenceLosses;
};

#endif // !defined(AFX_TEAM_H__C91C35E7_773D_410E_851E_259AC2DC8BBA__INCLUDED_)
