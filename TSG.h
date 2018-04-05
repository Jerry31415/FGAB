#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

void GenMaxTrainFile(int size, std::string filename = "train_max.txt"){
	std::ofstream f(filename.c_str());
	int a, b;
	for (int i = 0; i < size; ++i){
		a = rand() % 1024;
		b = rand() % 1024;
		f << "<in>" << a << " " << b << "</in><out>" << std::min(a, b) << "</out>\n";
	}
	f.close();
}


void GenDetermTrainFile(int size, std::string filename = "train_determ.txt"){
	std::ofstream f(filename.c_str());
	int a, b, c;
	for (int i = 0; i < size; ++i){
		a = rand() % 128;
		b = rand() % 128;
		c = rand() % 128;
		f << "<in>" << a << " " << b << " " << c << "</in><out>" << b*b - 4 * a*c << "</out>\n";
	}
	f.close();
}


void GenSqrTrainFile(int size, std::string filename = "train_sqr.txt"){
	std::ofstream f(filename.c_str());
	int a, b, c;
	for (int i = 0; i < size; ++i){
		a = rand() % 128;
		b = rand() % 128;
		f << "<in>" << a << " " << b << "</in><out>" << b*b + a*a << "</out>\n";
	}
	f.close();
}

void GenMax3TrainFile(int size, std::string filename = "train_sqr.txt"){
	std::ofstream f(filename.c_str());
	int a, b, c;
	for (int i = 0; i < size; ++i){
		a = rand() % 1024;
		b = rand() % 1024;
		c = rand() % 1024;
		f << "<in>" << a << " " << b << " " << c << "</in><out>" << std::max(a,std::max(b,c)) << "</out>\n";
	}
	f.close();
}

void GenMin3TrainFile(int size, std::string filename = "train_sqr.txt"){
	std::ofstream f(filename.c_str());
	int a, b, c;
	for (int i = 0; i < size; ++i){
		a = rand() % 1024;
		b = rand() % 1024;
		c = rand() % 1024;
		f << "<in>" << a << " " << b << " " << c << "</in><out>" << std::max(a, std::min(b, c)) << "</out>\n";
	}
	f.close();
}
