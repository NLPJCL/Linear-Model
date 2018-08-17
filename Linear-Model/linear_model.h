#pragma once
#include<iostream>
#include<map>
#include<vector>
#include"dataset.h"
using namespace std;
class linear_model
{
public:
	map<string, int> model;//ÌØÕ÷¿Õ¼ä¡£
	map<string, int> tag;//´ÊĞÔ
	void create_feature_space();
	void online_training();
	linear_model();
	
	~linear_model();
private:
	dataset train;
	dataset dev;
	vector<string> create_feature(sentence sentence, int pos, string tag);
	string maxscore_tag(sentence sen, int pos);
	double evaluate(dataset);
};

