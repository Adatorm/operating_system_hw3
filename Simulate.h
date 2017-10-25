/*
* Simulate.h
*
*  Created on: 26 Þub 2016
*      Author: Guner
*/

#ifndef SIMULATE_H_
#define SIMULATE_H_

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <list>
#include <cstring>
#include <iterator>

#define PAGESIZE 64 /* 2^6 */ /*page table * page size must equal to 2^9, */
#define RAMSIZE 512 /* 2^9 */
#define DISKSIZE 1048576 /*2^20*/
#define PAGETABLESIZE 512 /*2^9*/
#define DISKPAGES 508 /*because,ram only have 8 page, number of remained pages are 508*/
#define FIFOSECOND 1 /*if this value changed to zero, the clock replacement algorithm run instead of fifo second change*/
//you may need to add more functions

using namespace std;

class mmry {
public:
	mmry(int i, int v) {
		index = i;
		value = v;
	}
	~mmry(){}
	int index;
	int value;

};


class instruction {
public:
	instruction(int i, string &s, int a = 0, int b = 0, int numberOfOp = 0) {
		index = i;
		command = s;
		A = a;
		B = b;
		numberOfOperand = numberOfOp;
	}
	instruction(){ index = -1, command = "GTU", A = -1, B = -1; }
	~instruction(){}
	int index;
	string command;                  //like "ADD" ,"SET" ...
	int A;
	int B;
	int numberOfOperand;
	friend ostream& operator <<(ostream &outputStream, const instruction &inst){
		outputStream << "address: " << inst.index;
		outputStream << " command: " << inst.command;
		if (inst.numberOfOperand >= 1){ outputStream << " operand1: " << inst.A; }
		if (inst.numberOfOperand >= 2){ outputStream << " operand2: " << inst.B; }
		return outputStream;
	}
};


class PageFrame{
public:
	PageFrame(bool mod, bool ref, bool val, int pFN){
		modified = mod;
		referenced = ref;
		valid = val;
		pageFrameNumber = pFN;
		index = -1;
	}
	PageFrame()
	{
		modified = false;
		referenced = false;
		valid = false;
		pageFrameNumber = -1;
		index = -1;
	}
	bool modified;
	bool referenced;
	bool valid;/*if true, page frame is in ram otherwise is in disk*/
	int pageFrameNumber;
	int index;

};

class Simulate {
private:
	class VirtualMemory;
public:
	Simulate() ;/*default constructor*/
	Simulate(string &filename, int md, bool hlt = false);/*file name constructor*/
	~Simulate();/*destructor*/
	vector<mmry> memory;/*memory, used as a main mem in hw1, in this(hw3) hw , hold all data before split data to disk between ram*/
	vector<instruction> instructions;

	void printAll();/*print all of memory with address and content on screen*/
	bool tick();/*execute the instruction which pointed by program counter*/

	void printInstruction();/*print instruction list,using for debugging*/
	void parseFile(string &fileName);/*open file with given file name,init memory and instructions*/
	void executeInstruction(instruction &i);/*execute given instruction*/
	void executeInstruction2(instruction &i);/*execute given instruction*/
	bool isHalted();/*return value of halt variable*/
	void printMemory();/*print memory*/
	int getMode(){ return mode; }/*get mode*/
	bool handlePageFault();/*page fault handler with fifo then second chanceW*/
	bool handlePageFault2();/*page fault handler with clock replacement algorithm*/
	int numberOfPageFault;
	bool pageFault;
	int before ;
	list<int>::iterator it;/*clock replacement algorithm*/
	void printPageTable();/*print pageTable*/
	void printRam();/*print ram */
	void printDisk();/*print disk*/
	instruction incrementer;

	static unsigned int getPlaceValue(int number);/*calculate place value*/
private:
	bool halt;

	int mode;
	int ram[RAMSIZE];/*2^9*/
	vector<int> disk;/*2^20*/
	int faultFrame;/*if page fault occured, this variable will set with which page frame is not in ram*/
	PageFrame pageTable[PAGETABLESIZE];/*page table 2^9*2^6=2^15*/
	instruction saveIns;

	list<int> fifo;/*my fifo*/
	int vmSize;/*size of virtual memory*/
	int getVM(int index);/*return whatever, page fault cannot stop this function,only used printe vm content when the cpu is halted*/


	bool getData(int virtualAddress,int & output);/*read data from ram, if page fault occured,return false*/
	bool setData(int virtualAddress, int & input);/*write data to ram, if page fault occured,return false*/
	void writeDiskToRam(int ramAddress, int diskAddress);/*read data from disk, used in handlePageFault*/
	
	class VirtualMemory{
		public:

			VirtualMemory(Simulate & sim) :parent(sim){ cerr << "fun" << endl; }

			int & operator[](int index)
			{
				int pageIndex = index / PAGESIZE;/*calculating page frame*/
				int offset = index % PAGESIZE;

				/*calculating vm index*/
				int memoryAddress = (parent.pageTable[pageIndex].pageFrameNumber*PAGESIZE) + offset;
				if (parent.pageTable[pageIndex].valid == true)
				{
					parent.pageTable[pageIndex].referenced = true;
					return parent.ram[memoryAddress];
				}
				else{
					parent.faultFrame = pageIndex;
					throw new exception();
				}
			}
		private:
			Simulate &parent;
	};
	/*end of inner class*/

	VirtualMemory vmemory;

	
};


#endif /* SIMULATE_H_ */
