/*
* Simulate.cpp
*
*  Created on: 26 Þub 2016
*      Author: Guner
*/

#include "Simulate.h"


Simulate::Simulate(string &filename, int md, bool hlt ) :vmemory(*this) {
	parseFile(filename);
	mode = md;
	halt = hlt;
	faultFrame = -1;
	vmSize = 0;
	pageFault = false;
	/*allocate disk, if we declare disk as a static member, program stack overflow,
	so, we need to allocate disk space*/
	/*2^20=1048576*/
	//disk = new int[1048576];
	numberOfPageFault = 0;
	int index = 0;
	unsigned int last, last2;;
	int ramPageFrameMax = RAMSIZE / PAGESIZE;/*max index*/
	vmSize = memory.size();
	if (memory.size() > RAMSIZE)
	{
		/*save ram */
		for (unsigned int i = 0; i < RAMSIZE;)
		{
			for (unsigned j = 0; j < PAGESIZE; ++j, ++i)
			{
				ram[i] = memory[i].value;
			}
			pageTable[index].modified = true;
			pageTable[index].referenced = true;
			pageTable[index].valid = true;
			pageTable[index].pageFrameNumber = (i - 1) / PAGESIZE;
			pageTable[index].index = index;
			fifo.push_back(index);
			index++;
			last = i;
		}

		/*end of ram */

		/*start of disk*/
		for (unsigned int i = last; i+PAGESIZE < memory.size();)
		{
			for (unsigned j = 0; j < PAGESIZE; ++j, ++i)
			{
				disk.push_back(memory[i].value);
			}

			pageTable[index].modified = true;
			pageTable[index].referenced = true;
			pageTable[index].valid = false;
			pageTable[index].pageFrameNumber = (disk.size() - 1) / PAGESIZE;
			pageTable[index].index = index;
			index++;
			last2 = i;
		}

		if (last2 < memory.size())
		{
			int remained = memory.size() % PAGESIZE;
			for (int j = 0; j < remained; ++j)
			{
				disk.push_back(memory[j + last2].value);
			}


			for (int i = remained; i < PAGESIZE; ++i)
			{
				disk.push_back(-111111);
			}

			
			pageTable[index].modified = true;
			pageTable[index].referenced = true;
			pageTable[index].valid = false;
			pageTable[index].pageFrameNumber = (disk.size() - 1) / PAGESIZE;
			pageTable[index].index = index;
			index++;
			
			

		}
		/*end of disk*/
	}
	else
	{
		for (unsigned int i = 0; i < memory.size();)
		{
			for (unsigned j = 0; j < PAGESIZE; ++j, ++i)
			{
				ram[i] = memory[i].value;
			}
			pageTable[index].modified = true;
			pageTable[index].referenced = true;
			pageTable[index].valid = true;
			pageTable[index].pageFrameNumber = (i - 1) / PAGESIZE;
			pageTable[index].index = index;
			fifo.push_back(index);
			index++;
		}
	}

	it = fifo.begin();
	string s = "PC++";
	incrementer = instruction(-1, s);/*special instruction*/
}


Simulate::~Simulate(){
	//delete[] disk;/*delete allocated array*/
}

int Simulate::getVM(int index){
	int pageIndex = index / PAGESIZE;/*calculating page frame*/
	int offset = index % PAGESIZE;
	
	/*calculating vm index*/
	int memoryAddress = (pageTable[pageIndex].pageFrameNumber*PAGESIZE) + offset;
	if (pageTable[pageIndex].valid == true)
	{
		return ram[memoryAddress];
	}
	else{
		return disk[memoryAddress];
	}
	
}

void  Simulate::printAll() {
	for (int i = 0; i < vmSize; ++i) {
		cout << i << "  " << getVM(i) << " , ";
		
		if (i % 10 == 0) {
			cout << "\n";
		}
	}
	cout << endl;
}

void Simulate::printRam(){
	cout << "----------- RAM --------------" << endl;
	for (unsigned int i = 0; i < RAMSIZE; ++i)
	{
		cout << i << "  " << ram[i] << " , ";
		if (i % 10 == 0) {
			cout << "\n";
		}
	}
	cout << endl;
}

void Simulate::printDisk(){
	cout << "----------- DISK --------------" << endl;
	for (unsigned int i = 0; i < disk.size(); ++i)
	{
		cout << i << "  " << disk[i] << " , ";
		if (i % 10 == 0) {
			cout << "\n";
		}
	}
	cout << endl;
}


void Simulate::parseFile(string &fileName){
	//initialize memory and instructions after parsing file
	ifstream input;
	input.open(fileName.c_str(), ios::in);
	if (!input.is_open())
	{
		cerr << "error occured,given file cannot opened!!\nprogram aborted\n";
		exit(-2);
	}
	string line;/*input file line*/
	string instructionName;/*instruction name*/
	stringstream ss;/*stringstream*/
	int inputType = -1;/*0 for memory,1 for instruction,2 for end of instruction*/
	int data_address, memory_content;
	int operand1, operand2;/*instruction operands*/
	int numberOfOperand;
	while (!input.eof() && inputType != 2){
		getline(input, line);
		ss << line;
		getline(ss, line, '#');/*remove commands*/
		ss.str(""); /*clear sstream*/
		ss.clear(); /*clear error flags*/

		if (line.size() == 0)
		{
			/*no input*/
		}
		else if (strcmp(line.c_str(), "Begin Data Section") == 0)/*start of program*/
		{
			inputType = 0;/*set 0,start of memory block*/
		}
		else if (strcmp(line.c_str(), "End Data Section") == 0)/*end of memory*/
		{
			inputType = -1;
		}
		else if (strcmp(line.c_str(), "Begin Instruction Section") == 0)
		{
			inputType = 1;/*start of instruction*/
		}
		else if (strcmp(line.c_str(), "End Instruction Section") == 0)/*end of program*/
		{
			inputType = 2;/*end of program*/
		}
		else
		{
			if (inputType == 0)/**memory*/
			{
				ss << line;/* take line to sstream*/
				ss >> data_address; /* read memory address*/
				ss >> memory_content;/*read memory content*/
				memory.push_back(mmry(data_address, memory_content));
				ss.str(""); /*clear sstream*/
				ss.clear(); /*clear error flags*/
			}
			else if (inputType == 1)/*input type ==1,instruction*/
			{
				ss << line;/* take line to sstream*/
				ss >> data_address; /* read instruction address*/
				ss >> instructionName;/* read instruction name*/
				operand1 = 0;
				operand2 = 0;
				numberOfOperand = 0;
				if (((ss >> std::ws).peek() > 47 && (ss >> std::ws).peek() < 58) || (char)(ss >> std::ws).peek() == '-')/*instruction has operand*/
				{
					ss >> operand1;
					numberOfOperand++;
					if (((ss >> std::ws).peek() > 47 && (ss >> std::ws).peek() < 58) || (char)(ss >> std::ws).peek() == '-')/*instruction has operand*/
					{
						ss >> operand2;
						numberOfOperand++;
					}
				}
				instructions.push_back(instruction(data_address, instructionName, operand1, operand2, numberOfOperand));
				ss.str("");
				ss.clear();
			}
		}

	}/*end of while loop*/
	input.close();/*close opened file*/
}


void Simulate::printInstruction(){
	for (unsigned int i = 0; i < instructions.size(); ++i)
	{
		cout << instructions[i] << endl;
	}
}
/*command */

void Simulate::executeInstruction(instruction &i){
	/*set instruction,put A to location B*/
	if (i.command.compare("SET") == 0){
		memory[i.B].value = i.A;
	}
	/*copy direct instruction*/
	else if (i.command.compare("CPY") == 0){
		memory[i.B].value = memory[i.A].value;
	}
	/*copy indirect instruction*/
	else if (i.command.compare("CPYI") == 0){
		memory[i.B].value = memory[memory[i.A].value].value;
	}
	/*copy indirect instruction version 2*/
	else if (i.command.compare("CPYI2") == 0){
		memory[memory[i.B].value].value = memory[i.A].value;
	}
	/*add instruction,add A to location B*/
	else if (i.command.compare("ADD") == 0){
		memory[i.B].value += i.A;
	}
	/*add indirect,add contents of A to contents of B*/
	else if (i.command.compare("ADDI") == 0){
		memory[i.B].value += memory[i.A].value;
	}
	/*Indirect Subtraction: Subtract the contents of memory address A2 from
	address A1, put the result in A2*/
	else if (i.command.compare("SUBI") == 0){
		memory[i.B].value = memory[i.B].value - memory[i.A].value;
	}
	/*Set the CPU program counter with B if memory location A content is less
	than or equal to 0*/
	else if (i.command.compare("JIF") == 0){
		if (memory[i.A].value <= 0)/*jump*/
		{
			memory[0].value = i.B;
		}
	}
	/*shuts down the cpu*/
	else if (i.command.compare("HLT") == 0){
		cerr << "omg halet " << endl;
		halt = true;
	}
	/*Calls the operting system service with PARAMS*/
	else if (i.command.compare("SYS") == 0){
		cerr << "i: " << " j: " << endl;
	}
	else
	{
		cerr << "syntax error-" << "not found \"" << i.command << "\" instructions\nprogram aborted";
		exit(-3);
	}
}


bool Simulate::isHalted(){
	return halt;
}

bool Simulate::tick(){

	int current;
	int e = 4;
	try{
		current = vmemory[0];
		vmemory[0]++;/*increase program counter*/
	}
	catch (...){
		saveIns = incrementer;/*pc incrementer instruction*/
		return false;
	}

	

	try{
		executeInstruction2(instructions[current]);/*exec instruction*/
	}
	catch(...){
		saveIns = instructions[current];/*save not executed instruction*/
		return false;
	}
	before = current;
	return true;
}

void Simulate::printMemory(){
	for (unsigned int i = 0; i < memory.size(); ++i)
	{
		cout << i << " " << memory[i].value << " ,";
		if (i % 10 == 9)
		{
			cout << endl;
		}
	}
}


bool Simulate::handlePageFault(){
	int findex;
	int indexTemp;
	numberOfPageFault++;
	while (true)
	{
		findex = fifo.front();
		if (pageTable[findex].referenced == true)
		{
			pageTable[findex].referenced = false;
			fifo.pop_front();
			fifo.push_back(findex);
		}
		else
		{
			/*change ram and disk data*/
			writeDiskToRam(pageTable[findex].pageFrameNumber*PAGESIZE, pageTable[faultFrame].pageFrameNumber*PAGESIZE);

			/*change valid bits*/
			pageTable[findex].valid = false;
			pageTable[faultFrame].valid = true;
			pageTable[faultFrame].referenced = true;

			indexTemp = pageTable[faultFrame].pageFrameNumber;
			pageTable[faultFrame].pageFrameNumber = pageTable[findex].pageFrameNumber;
			pageTable[findex].pageFrameNumber = indexTemp;


			fifo.pop_front();
			fifo.push_back(faultFrame);


			if (mode == 3)
			{
				cout << "PAGE TABLE CONTENTS" << endl;
				cout << " index    modified  referenced  valid  PFN " << endl;
				for (unsigned int i = 0; i <= memory.size() / PAGESIZE; ++i)
				{
					cout << i << "  " << pageTable[i].modified << "  " << pageTable[i].referenced << "  " << pageTable[i].valid << "  " << pageTable[i].pageFrameNumber << endl;
				}

				cout << "index " << findex << " replaced with " << faultFrame << " in disk" << endl;
				cout << "pfn " << pageTable[findex].pageFrameNumber << " and " << pageTable[faultFrame].pageFrameNumber << endl;

				cout << " fifo " << endl;
				for (std::list<int>::iterator it = fifo.begin(); it != fifo.end(); ++it)
					std::cout << ' ' << *it;
			}
			try
			{
				executeInstruction2(saveIns);/*rerun instruction*/
			}
			catch(...){
				if (mode==3)
					cout << "PAGE FAULT OCCURED" << endl;
				handlePageFault();
				
			}
			return true;
		}
	}
	return false;

}


void Simulate::writeDiskToRam(int ramAddress, int diskAddress)
{
	int temp[PAGESIZE];
	/*save the data which will writed to disk in the ram*/
	for (unsigned int i = 0; i < PAGESIZE; ++i)
	{
		temp[i] = ram[ramAddress + i];
	}

	/*copy disk data(page) to ram location*/
	for (unsigned int i = 0; i < PAGESIZE; ++i)
	{
		ram[ramAddress + i] = disk[diskAddress + i];
	}

	/*restore the page which is  saved in temp array to disk */
	for (unsigned int i = 0; i < PAGESIZE; ++i)
	{
		disk[diskAddress + i] = temp[i];
	}
}


void Simulate::executeInstruction2(instruction &i){
	/*set instruction,put A to location B*/
	if (i.command.compare("SET") == 0){
		vmemory[i.B] = i.A;
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*copy direct instruction*/
	else if (i.command.compare("CPY") == 0){
		vmemory[i.B] = vmemory[i.A];
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*copy indirect instruction*/
	else if (i.command.compare("CPYI") == 0){
		vmemory[i.B] = vmemory[vmemory[i.A]];
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*copy indirect instruction version 2*/
	else if (i.command.compare("CPYI2") == 0){
		vmemory[vmemory[i.B]] = vmemory[i.A];
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*add instruction,add A to location B*/
	else if (i.command.compare("ADD") == 0){
		vmemory[i.B] += i.A;
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*add indirect,add contents of A to contents of B*/
	else if (i.command.compare("ADDI") == 0){
		vmemory[i.B] += vmemory[i.A];
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*Indirect Subtraction: Subtract the contents of memory address A2 from
	address A1, put the result in A2*/
	else if (i.command.compare("SUBI") == 0){
		vmemory[i.B] = vmemory[i.B] - vmemory[i.A];
		pageTable[i.B / PAGESIZE].modified = true;
	}
	/*Set the CPU program counter with B if memory location A content is less
	than or equal to 0*/
	else if (i.command.compare("JIF") == 0){
		if (vmemory[i.A] <= 0)/*jump*/
		{
			vmemory[0] = i.B;
			pageTable[0].modified = true;
		}
	}
	/*shuts down the cpu*/
	else if (i.command.compare("HLT") == 0){
		halt = true;
	}
	/*Calls the operting system service with PARAMS*/
	else if (i.command.compare("SYS") == 0){
		cerr << "i: " << " j: " << endl;
	}
	/*increment program counter*/
	else if (i.command.compare("PC++") == 0){
		vmemory[0]++;
	}
	else
	{
		cerr << "syntax error-" << "not found \"" << i.command << "\" instructions\nprogram aborted";
		exit(-3);
	}
}

bool Simulate::handlePageFault2(){
	int findex;
	int indexTemp=-1;
	numberOfPageFault++;
	while (true)
	{
		findex = *it;
		if (pageTable[findex].referenced == true)
		{
			pageTable[findex].referenced = false;
			if (it == fifo.end())
			{
				it = fifo.begin();
			}
			else
			{
				++it;
			}
		}
		else
		{
			/*change ram and disk data*/
			writeDiskToRam(pageTable[findex].pageFrameNumber*PAGESIZE, pageTable[faultFrame].pageFrameNumber*PAGESIZE);

			/*change valid bits*/
			pageTable[findex].valid = false;
			pageTable[findex].modified = false;
			pageTable[faultFrame].valid = true;
			pageTable[faultFrame].referenced = true;

			indexTemp = pageTable[faultFrame].pageFrameNumber;
			pageTable[faultFrame].pageFrameNumber = pageTable[findex].pageFrameNumber;
			pageTable[findex].pageFrameNumber = indexTemp;


			if (it == fifo.end())
			{
				it = fifo.begin();
			}
			else
			{
				++it;
			}

			if (mode == 3)
			{
				cout << "PAGE TABLE CONTENTS" << endl;
				cout << " index    modified  referenced  valid  PFN " << endl;
				for (unsigned int i = 0;i <= memory.size() / PAGESIZE; ++i)
				{
					cout << i << "  " << pageTable[i].modified << "  " << pageTable[i].referenced << "  " << pageTable[i].valid << "  " << pageTable[i].pageFrameNumber << endl;
				}

				cout << "index " << findex << " replaced with " << faultFrame << " in disk" << endl;
				cout << "pfn " << pageTable[findex].pageFrameNumber << " and " << pageTable[faultFrame].pageFrameNumber << endl;

				cout << " clock " << endl;
				for (std::list<int>::iterator it = fifo.begin(); it != fifo.end(); ++it)
					std::cout << ' ' << *it;
			}
			try
			{
				executeInstruction2(saveIns);/*rerun instruction*/
			}
			catch (...){
				if (mode==3)
					cout << "PAGE FAULT OCCURED" << endl;
				handlePageFault2();
			}
			return true;
		}
	}
	return false;

}


void Simulate::printPageTable(){

	cout << " __________________ " << endl;
	cout << " PAGE TABLE " << endl;
	cout << " index    modified  referenced  valid  PFN " << endl;
	for (unsigned int i = 0; i <= memory.size() / PAGESIZE; ++i)
	{
		cout << i << "  " << pageTable[i].modified << "  " << pageTable[i].referenced << "  " << pageTable[i].valid << "  " << pageTable[i].pageFrameNumber << endl;
	}
}