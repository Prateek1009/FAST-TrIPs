/*-------------------------------------------------------
FAST-TrIPs: Flexible Assignment and Simulation Tool for Transit and Intermodal Passengers
Copyright (C) 2013 by Alireza Khani
Released under the GNU General Public License, version 2.
-------------------------------------------------------
Code primarily written by Alireza Khani
Under supervision of Mark Hickman

Contact:
    Alireza Khani:  akhani@utexas.edu or akhani@email.arizona.edu
    Mark Hickman:   m.hickman1@uq.edu.au
-------------------------------------------------------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------*/

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>
using namespace std;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class	taz{
protected:
	string					tazId;
	double					tazLat;
	double					tazLon;

	vector<string>			tazNodes;
	vector<string>			tazStops;
	vector<double>			tazAccessDistances;
	vector<double>			tazAccessTimes;

	//For TBSP
	vector<double>			tazLabels;
	vector<double>			tazArrivals;
	vector<double>			tazDepartures;
	vector<string>			tazArrivalModes;
	vector<string>			tazDepartureModes;
	vector<string>			tazPredecessors;
	vector<string>			tazSuccessors;

public:
	taz(){}
	~taz(){}
	void					initializeTaz(string _tmpIn);
	void					attachNode(string _tmpIn);
	void					attachStop(string _tmpIn);

	//For TBSP
	void					parallelize(int _numThreads);
	void					resetTaz(int _threadId);
	string					getTazId();
	int						getNumNodes();
	string					getNode(int _i);
	int						getNumStops();
	string					getStop(int _i);
	double					getAccessTime(int _i);
	void					forwardUpdate(double _label, double _arrival, string _predecessor, int _threadId);
	void					backwardUpdate(double _label, double _departure, string _successor, int _threadId);
	double					getLabel(int _threadId);
	double					getDeparture(int _threadId);
	double					getArrival(int _threadId);
	string					getPredecessor(int _threadId);
	string					getSuccessor(int _threadId);
	string					printPath(int _threadId);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
map<string,taz*>			tazSet;
list<taz*>					tazList;
map<string,double>			accessTimes;

int			readTAZs();
int			readAccessLinks();
int			parallelizeTazs(int _numThreads);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int			readTAZs(){
	int				tmpNumZones;
	string			tmpIn, buf, tmpTazId;
	vector<string>	tokens;
	taz*			tmpTazPntr;

	ifstream inFile;
	inFile.open("ft_input_zones.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_zones.dat";
		exit(1);
	}

	tmpNumZones = 0;
	buf.clear();
	tokens.clear();
	getline(inFile,tmpIn);
	while(!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss1(tmpIn);
		while (ss1 >> buf){
			tokens.push_back(buf);
		}

		tmpTazId = "t";
		tmpTazId.append(tokens[0]);
		if(tazSet.find(tmpTazId)==tazSet.end()){
			tmpTazPntr = NULL;
			tmpTazPntr = new taz;
			tazSet[tmpTazId] = tmpTazPntr;
			tazSet[tmpTazId]->initializeTaz(tmpIn);
			tazList.push_back(tmpTazPntr);
		}
		tmpNumZones++;
	}
	return tmpNumZones;
}
int			readAccessLinks(){
	int				numAccessLinks;
	string			tmpIn, buf, tmpTazId, tmpStopId;
	vector<string>	tokens;
	taz*			tmpTazPntr;
	stop*			tmpStopPntr;

	ifstream inFile;
	inFile.open("ft_input_accessLinks.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_accessLinks.dat";
		exit(1);
	}
	numAccessLinks=0;
	getline(inFile,tmpIn);
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		while (ss >> buf){
			tokens.push_back(buf);
		}
		tmpTazId = "t";
		tmpTazId.append(tokens[0]);
		tmpTazPntr = NULL;
		tmpTazPntr = tazSet[tmpTazId];
		tmpTazPntr->attachStop(tmpIn);
		tmpStopId = "s";
		tmpStopId.append(tokens[1]);
		tmpStopPntr = NULL;
		tmpStopPntr = stopSet[tmpStopId];
		tmpStopPntr->attachTaz(tmpIn);
		accessTimes[tmpTazId+","+tmpStopId] = atof(tokens[3].c_str());
		numAccessLinks++;
	}
	return numAccessLinks;
}
int			parallelizeTazs(int _numThreads){
	list<taz*>::iterator	tmpTazListIter;
	taz*					tmpTazPntr;

	for(tmpTazListIter=tazList.begin();tmpTazListIter!=tazList.end();tmpTazListIter++){
		tmpTazPntr = NULL;
		tmpTazPntr = *tmpTazListIter;
		tmpTazPntr->parallelize(_numThreads);
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void		taz::initializeTaz(string _tmpIn){
	string			buf, tmpStr, tmpNode;
	vector<string>	tokens;

	buf.clear();
	tokens.clear();
	stringstream	ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	tazId = "t";
	tazId.append(tokens[0]);
	tazLat = atof(tokens[1].c_str());
	tazLon = atof(tokens[2].c_str());
}
void		taz::attachNode(string _tmpIn){
	string			buf, tmpNode;
	vector<string>	tokens;

	buf.clear();
	tokens.clear();
	stringstream	ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	tmpNode = "n";
	tmpNode.append(tokens[0]);
	tazNodes.push_back(tmpNode);
}
void		taz::attachStop(string _tmpIn){
	string			buf, tmpStop;
	vector<string>	tokens;
	double			tmpAccessDist, tmpAccessTime;

	buf.clear();
	tokens.clear();
	stringstream	ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	tmpStop = "s";
	tmpStop.append(tokens[1]);
	tmpAccessDist = atof(tokens[2].c_str());
	tmpAccessTime = atof(tokens[3].c_str());
	tazStops.push_back(tmpStop);
	tazAccessDistances.push_back(tmpAccessDist);
	tazAccessTimes.push_back(tmpAccessTime);
}
void		taz::parallelize(int _numThreads){
	int		tmpCntr;
	for(tmpCntr=0;tmpCntr<_numThreads;tmpCntr++){
		tazLabels.push_back(0);
		tazArrivals.push_back(0);
		tazDepartures.push_back(0);
		tazArrivalModes.push_back("-101");
		tazDepartureModes.push_back("-101");
		tazPredecessors.push_back("-101");
		tazSuccessors.push_back("-101");
	}
}
void		taz::resetTaz(int _threadId){
	tazLabels[_threadId] = 999999;
	tazArrivals[_threadId] = 999999;
	tazDepartures[_threadId] = -999999;
	tazArrivalModes[_threadId] = "-101";
	tazDepartureModes[_threadId] = "-101";
	tazPredecessors[_threadId] = "-101";
	tazSuccessors[_threadId] = "-101";
}
string		taz::getTazId(){
	return this->tazId;
}
int			taz::getNumNodes(){
	return this->tazNodes.size();
}
string		taz::getNode(int _i){
	return this->tazNodes[_i];
}
int			taz::getNumStops(){
	return this->tazStops.size();
}
string		taz::getStop(int _i){
	return this->tazStops[_i];
}
double		taz::getAccessTime(int _i){
	return this->tazAccessTimes[_i];
}
void		taz::forwardUpdate(double _label, double _arrival, string _predecessor, int _threadId){
	tazLabels[_threadId] = _label;
	tazArrivals[_threadId] = _arrival;
	tazPredecessors[_threadId] = _predecessor;
}
void		taz::backwardUpdate(double _label, double _departure, string _successor, int _threadId){
	tazLabels[_threadId] = _label;
	tazDepartures[_threadId] = _departure;
	tazSuccessors[_threadId] = _successor;
}
double		taz::getLabel(int _threadId){
	return this->tazLabels[_threadId];
}
double		taz::getDeparture(int _threadId){
	return this->tazDepartures[_threadId];
}
double		taz::getArrival(int _threadId){
	return this->tazArrivals[_threadId];
}
string		taz::getPredecessor(int _threadId){
	return this->tazPredecessors[_threadId];
}
string		taz::getSuccessor(int _threadId){
	return this->tazSuccessors[_threadId];
}
string		taz::printPath(int _threadId){
	cout <<tazId<<"\t"<<tazLabels[_threadId]<<endl;
	cout <<"\t\t"<<tazDepartures[_threadId]<<"\t"<<tazSuccessors[_threadId]<<"\t"<<endl;
	return "";
}