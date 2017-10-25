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
	indexOfRamLast = -1;
	indexOfDiskLast = -1;
	faultFrame = -1;
	head = NULL;
	tail = NULL;
	vmSize = 0;
	/*allocate disk, if we declare disk as a static member, program stack overflow,
	so, we need to allocate disk space*/
	/*2^20=1048576*/
	disk = new int[1048576];

	int index = 0;
	Node * n;
	int ramPageFrameMax = RAMSIZE / PAGESIZE;/*max index*/
	if (memory.size() > RAMSIZE)
	{
		for (unsigned int i = 0; i < RAMSIZE; ++i)
		{
			ram[i] = memory[i].value;
		}
		vmSize += RAMSIZE;
		indexOfRamLast = RAMSIZE - 1;
		while (index < ramPageFrameMax)
		{
			if (head == NULL)
			{
				n = new Node();
				n->data = index;
				n->next = NULL;
				tail = n;
				tail->next = NULL;
				head = n;
				pageTable[index].modified = true;
				pageTable[index].referenced = true;
				pageTable[index].valid = true;
				pageTable[index].pageFrameNumber = index;

				index++;
			}
			else
			{
				n = new Node();
				n->next = head;
				head = n;
				head->data = index;

				pageTable[index].modified = true;
				pageTable[index].referenced = true;
				pageTable[index].valid = true;
				pageTable[index].pageFrameNumber = index;
				index++;

			}

		}/*end of fifo while loop*/

		/*remained data will be writed to disk*/
		for (unsigned int i = RAMSIZE; i < memory.size(); ++i)
		{
			disk[i - RAMSIZE] = memory[i].value;
		}
		
		int diskPageFrameMax = memory.size() - RAMSIZE / PAGESIZE;
		if (diskPageFrameMax > PAGETABLESIZE - ramPageFrameMax)
		{
			diskPageFrameMax = PAGETABLESIZE - ramPageFrameMax;
			cerr << "mayushii desuu" << diskPageFrameMax << endl;
		}
		vmSize += diskPageFrameMax;
		indexOfDiskLast = memory.size() - RAMSIZE - 1;

		/*with this initial values, page table can address 512 page, and ram has 8 page,
		so , the disk can addressed max 504 pages*/

		while (index < diskPageFrameMax)
		{
			pageTable[index].modified = true;
			pageTable[index].referenced = true;
			pageTable[index].valid = false;
			pageTable[index].pageFrameNumber = index;

			index++;

		}/*end of pageTable updating while loop*/


	}
	else
	{
		for (unsigned int i = 0; i < memory.size(); ++i)
		{
			ram[i] = memory[i].value;
		}
		indexOfRamLast = memory.size() - 1;
		vmSize += memory.size();
		ramPageFrameMax = indexOfRamLast / PAGESIZE;/*max index*/
		while (index < ramPageFrameMax)
		{
			if (head == NULL)
			{
				n = new Node();
				n->data = index;
				n->next = NULL;
				tail = n;
				head = n;
				pageTable[index].modified = true;
				pageTable[index].referenced = true;
				pageTable[index].valid = true;
				pageTable[index].pageFrameNumber = index;

				index++;
			}
			else
			{
				n = new Node();
				n->next = head;
				head = n;
				head->data = index;
				pageTable[index].modified = true;
				pageTable[index].referenced = true;
				pageTable[index].valid = true;
				pageTable[index].pageFrameNumber = index - ramPageFrameMax;
				index++;

			}

		}/*end of fifo while loop*/
	}
	
	Node*tn;
	tn = head;
	while (tn != NULL)
	{
		cerr <<" debug  "  <<  tn->data << endl;
		tn = tn->next;
	}
	
}

/*delete given list*/
void deleteTree(Node * head)
{
	Node* temp;
	while (head != NULL)
	{
		temp = head->next;
		delete head;
		head = temp;
	}
}


Simulate::~Simulate(){
	delete[] disk;/*delete allocated array*/
	deleteTree(head);/*delete fifo*/
}

void  Simulate::printAll() {
	for (unsigned int i = 0; i < vmSize; ++i) {
		cout << i << "  " << vmemory[i] << " , ";
		if (i % 10 == 0) {
			cout << "\n";
		}
	}

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
	int current = vmemory[0];
	vmemory[0]++;/*increase program counter*/
	try{
		executeInstruction2(instructions[current]);/*exec instruction*/
	}
	catch(int e){
		saveIns = instructions[current];/*save not executed instruction*/
		return false;
	}
	return true;
}

void Simulate::printMemory(){
	for (unsigned int i = 0; i < memory.size(); ++i)
	{
		cout << i << " " << vmemory[i] << " ,";
		if (i % 10 == 9)
		{
			cout << endl;
		}
	}
}


bool Simulate::getData(int virtualAddress, int & output)/*read data from ram, if page fault occured,return false*/
{
	int pageFrame = virtualAddress / PAGESIZE;/*calculating page frame*/
	int offset = virtualAddress%PAGESIZE;
	if (pageTable[pageFrame].valid == false)
	{
		return false;
	}
	else
	{
		/*calculating ram index*/
		int memoryAddress = (pageTable[pageFrame].pageFrameNumber*PAGESIZE) + offset;
		output = ram[memoryAddress];
		pageTable[pageFrame].referenced = true;
		pageTable[pageFrame].modified = false;
		return true;
	}
}

bool Simulate::setData(int virtualAddress, int & input)/*write data to ram, if page fault occured,return false*/
{
	int pageFrame = virtualAddress / PAGESIZE;/*calculating page frame*/
	int offset = virtualAddress%PAGESIZE;
	if (pageTable[pageFrame].valid == false)
	{
		faultFrame = pageFrame;
		return false;
	}
	else
	{
		/*calculating ram index*/
		int memoryAddress = (pageTable[pageFrame].pageFrameNumber*PAGESIZE) + offset;
		ram[memoryAddress]=input;
		pageTable[pageFrame].referenced = true;
		pageTable[pageFrame].modified = true;
		return true;
	}
}

unsigned int Simulate::getPlaceValue(int number)
{
	int count = 0;
	if (number == 0)
	{
		return 1;
	}
	while (number != 0)
	{
		number = number / 10;
		count++;
	}
	return count;
}


bool Simulate::handlePageFault(){
	Node * temp=NULL;
	PageFrame f;
	int indexTemp;
	while (true)
	{
		
		if (pageTable[(*head).data].referenced == true)
		{
			pageTable[(*head).data].referenced = false;
			temp = head->next;
			tail->next = head;
			tail = head;
			tail->next = NULL;
			head = temp;
		}
		else
		{
			temp = head->next;
			tail->next = head;
			tail = head;
			tail->next = NULL;
			head = temp;
			
			/*change ram and disk data*/
			writeDiskToRam(pageTable[(*tail).data].pageFrameNumber*PAGESIZE, pageTable[faultFrame].pageFrameNumber*PAGESIZE);

			/*change valid bits*/
			pageTable[(*tail).data].valid = false;
			pageTable[faultFrame].valid = true;

			indexTemp = pageTable[faultFrame].pageFrameNumber;
			pageTable[faultFrame].pageFrameNumber = pageTable[(*tail).data].pageFrameNumber;
			pageTable[(*tail).data].pageFrameNumber = indexTemp;
			executeInstruction2(saveIns);/*rerun instruction*/
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
	else
	{
		cerr << "syntax error-" << "not found \"" << i.command << "\" instructions\nprogram aborted";
		exit(-3);
	}
}