//Modified to add target inputs, 9/10/2012 AW

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "DataWriter.h"


using namespace std;

DataWriter::DataWriter(TargetFrame Target, const char* filename)
{

	//record current date/time
	time_t current_time = time(0);
	tm* ltm = localtime(&current_time);
	stringstream ss1, ss2, ss3;
	
	iswriting = 0;

	ss1 << setw(4) << setfill('0') << ltm->tm_year + 1900;
	ss1 << setw(2) << setfill('0') << ltm->tm_mon + 1;
	ss1 << setw(2) << setfill('0') << ltm->tm_mday;
	
	ss2 << setw(2) << setfill('0') << ltm->tm_hour;
	ss2 << setw(2) << setfill('0') << ltm->tm_min;
	ss2 << setw(2) << setfill('0') << ltm->tm_sec;


	// If no filename was supplied, use the date and time
	if (filename == NULL)
	{
		ss3 << ss1.str() << "_" << ss2.str() << ".dat";
		file.open(ss3.str(), ios::out);
	}
	else
	{
		//append timestamp to prevent overwrites
		ss3 << filename << "_" << ss1.str() << ss2.str() << ".dat";
		file.open(ss3.str(), ios::out);
	}

	// Write headers to file
	if (file.is_open())
	{

		//write current date
		file << "Date " << ss1.str() << endl;
		file << "Time " << ss2.str() << endl;
		//write trial parameters of interest
		file << "SubjectID " << SUBJECT_ID << endl;
		file << "Block_Type " << Target.TrType << endl;
		file << "Trial " << Target.trial << endl;
		file << "Redo " <<	Target.redo << endl;
		file << "VidA" << Target.vid1 << endl;
		file << "VidB" << Target.vid2 << endl;

		file << "--" << endl;
		
		file << "Device_Num "
			 << "Time "
			 << "Trial "
			 << "Redo "
			 << "TrialType "
			 << "Vid1 "
			 << "Vid2 "
			 << "VidType "
			 << "VidStatus "
			 << "Instruct "
			 << "CResp "
			 << "Resp "
			 << "Correct "
			 << "Score "
			 << "Practice "
			 << "Latency "
			 << endl;


		file << "-----" << endl;  //flag designator for finding start of the data stream.  everything above is header

		iswriting = 1;

	}
}

DataWriter::~DataWriter()
{
	iswriting = 0;
	file.close();
}
 
void DataWriter::Record(int deviceNo, TargetFrame Target)
{
	std::streamsize ss;
	ss = std::cout.precision();

	// Write data
	if (file.is_open())
	{
		file << deviceNo << " "
			<< std::fixed << showpoint << std::setprecision(5) 
			<< Target.time << " "
			//<< std::resetiosflags( std::ios::fixed | std::ios::showpoint )
			<< std::resetiosflags( std::ios::fixed | std::ios::showpoint ) << std::setprecision(ss)
			<< Target.trial << " "
			<< Target.redo << " "
			<< Target.vid1 << " "
			<< Target.vid2 << " "
			<< Target.vidtype << " "
			<< Target.vidstatus << " "
			<< Target.instruct << " "
			<< Target.cresp << " "
			<< Target.resp << " "
			<< Target.correct << " "
			<< Target.score << " "
			<< Target.practice << " "
			<< Target.lat
			<< endl;
	}
}

void DataWriter::Close()
{
	if (iswriting == 1)
		file.close();
	
	iswriting = 0;
}