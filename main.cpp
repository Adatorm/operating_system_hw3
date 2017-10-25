/*
* main.cpp
*
*  Created on: 26 Þub 2016
*      Author: Guner Acet 121044049
*/

#include "Simulate.h"


int main(int argc, char *argv[]) {

	// here you should get the command line arguments and pass to the sim(string filename, int mode)
	string fileName;
	int mode = -1;
	char key;
	if (argc != 4)
	{
		cerr << "please enter 3 argument\nUsage exec <filename> -D <integer>";
		return 1;
	}
	else
	{
		fileName = argv[1];
		mode = atoi(argv[3]);
		cerr << "opened file is: " << fileName << endl;
		if (mode < 0 || mode > 3)
		{
			cerr << "debug mode has to be 0,1 or 2. Program aborted" << endl;
			return 1;
		}
		cerr << "debug mode: " << mode << endl;
	}
	cerr << "i am here" << endl;
	Simulate sim(fileName, mode);
	//sim.printInstruction();/*was used for debugging*/
	//sim.cpuRun();
	int i = 0;
	while (!sim.isHalted()){
		if (sim.tick() == false)
		{
			if (sim.getMode()==3)
				cout << "PAGE FAULT OCCURED" << endl;
			sim.handlePageFault();
		}
		if (sim.getMode() == 1)
		{
			sim.printAll();
		}
		else if (sim.getMode() == 2)
		{
			sim.printAll();
			cout << "enter a key" << endl;
			cin >> key;
		}
	}

	cout << "Program Halted" << endl;
	sim.printAll();
	cout << sim.numberOfPageFault << " page fault and page replacement occured" << endl;
	system("pause");

	return 0;
}
