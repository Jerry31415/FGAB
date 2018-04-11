#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <functional>


class TestGeneratorI{
	std::vector<std::pair<std::vector<int>, int>> samples;
	std::function<int(std::vector<int> in)> generator;
	size_t _size, _size_in;
public:
	TestGeneratorI(std::function<int(std::vector<int> in)> gen_func, size_t size, size_t size_in) : _size(size), _size_in(size_in){
		generator = gen_func;
	}

	void generate(size_t in_max_value, bool useSign = false){
		std::vector<int> in;
		std::pair<std::vector<int>, int> t;
		for (int i = 0; i < _size; ++i){
			in.clear();
			if (!useSign){
				for (int k = 0; k < _size_in; ++k) in.push_back(rand() % in_max_value);
			}
			else {
				for (int k = 0; k < _size_in; ++k) in.push_back(((rand() % 2) ? 1 : -1)*(rand() % in_max_value));
			}
			try{
				t.first.clear();
				std::swap(in, t.first);
				t.second = generator(t.first);
				samples.push_back(t);
			}
			catch (...){
				std::cerr << "Error: TestGenerator->generate error\n";
				return;
			}
		}
	}

	void write(std::ostream& os){
		for (auto& e : samples){
			os << "<in>";
			for (int i = 0; i < _size_in;++i){
				os << e.first[i];
				if(i!=_size_in-1) os << " ";
			}
			os << "</in><out>" << e.second << "</out>\n";
		}
	}

	void save(const std::string& filename){
		std::ofstream tf(filename.c_str());
		write(tf);
		tf.close();
	}

};

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
	int a, b; //, c;
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
