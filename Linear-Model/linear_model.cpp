#include "linear_model.h"
//实例化特征
vector<string> linear_model::create_feature(sentence sentence, int pos, string tag)
{
	string word = sentence.word[pos];//当前词。
	string word_char_first = sentence.word_char[pos][0];//当前词的第一个元素。
	string word_char_last = sentence.word_char[pos][sentence.word_char[pos].size() - 1];//当前词的最后一个元素。
	string word_m1;
	string word_char_m1m1;
	string word_p1;
	string word_char_p1_first;
	int word_count = sentence.word.size();//当前句的总词数。
	if (pos == 0)
	{
		word_m1 = "$$";
		word_char_m1m1 = "$";
	}
	else
	{
		word_m1 = sentence.word[pos - 1];
		word_char_m1m1 = sentence.word_char[pos - 1][(sentence.word_char[pos - 1].size() - 1)];
	}
	if (pos == word_count - 1)
	{
		word_p1 = "##";
		word_char_p1_first = "#";
	}
	else
	{
		word_p1 = sentence.word[pos + 1];
		word_char_p1_first = sentence.word_char[pos + 1][0];
	}
	vector<string> f;
	f.push_back("02:" + tag + "*" + word);
	f.push_back("03:" + tag + "*" + word_m1);
	f.push_back("04:" + tag + "*" + word_p1);
	f.push_back("05:" + tag + "*" + word + "*" + word_char_m1m1);
	f.push_back("06:" + tag + "*" + word + "*" + word_char_p1_first);
	f.push_back("07:" + tag + "*" + word_char_first);
	f.push_back("08:" + tag + "*" + word_char_last);
	int pos_word_len = sentence.word_char[pos].size();
	for (int k = 0; k < pos_word_len - 1; k++)
	{
		string cik = sentence.word_char[pos][k];
		f.push_back("09:" + tag + "*" + cik);
		f.push_back("10:" + tag + "*" + word_char_first + "*" + cik);
		f.push_back("11:" + tag + "*" + word_char_last + "*" + cik);
		string cikp1 = sentence.word_char[pos][k + 1];
		if (cik == cikp1)
		{
			f.push_back("13:" + tag + "*" + cik + "*" + "consecutive");
		}
	}
	if (pos_word_len == 1)
	{
		f.push_back("12:" + tag + "*" + word + "*" + word_char_m1m1 + "*" + word_char_p1_first);
	}
	for (int k = 0; k <pos_word_len; k++)
	{
		if (k >= 4)break;
		string prefix, suffix;
		//获取前缀
		for (int n = 0; n <= k; n++)
		{
			prefix = prefix + sentence.word_char[pos][n];
		}
		//获取后缀。
		for (int n = pos_word_len - k - 1; n <= pos_word_len - 1; n++)
		{
			suffix = suffix + sentence.word_char[pos][n];
		}
		f.push_back("14:" + tag + "*" + prefix);
		f.push_back("15:" + tag + "*" + suffix);
	}
	return f;
}
//创建特征空间
void linear_model::create_feature_space()
{
	for (auto z = train.sentences.begin(); z != train.sentences.end(); z++)
	{
		for (int i = 0; i < z->word.size(); i++)
		{
			vector <string> f;
			f = create_feature(*z, i, z->tag[i]);
			for (auto q = f.begin(); q != f.end(); q++)
			{
				if (model.find(*q) == model.end())//如果不在词性里面。
				{
					model[*q] = 0;
				}
				if (tag.find(z->tag[i]) == tag.end())
				{
					tag[z->tag[i]] = 0;
				}
				else
				{
					tag[z->tag[i]] = tag[z->tag[i]] + 1;

				}
			}
		}
	}
	cout << "the total number of features is " << model.size() << endl;
	cout << "the total number of tags is " << tag.size() << endl;
}
string linear_model::maxscore_tag(sentence  sen, int pos)
{
	double max_num = -1e10,score;
	string max_tag;
	for (auto z = tag.begin(); z != tag.end(); z++)//遍历词性
	{
		score = 0;
		vector<string> f = create_feature(sen, pos,z->first);
		//计算权值。
		for (auto z0 = f.begin(); z0 != f.end(); z0++)
		{
			auto z1 = model.find(*z0);
			if (z1 != model.end())
			{
				score = score + model[z1->first];
			}
		}
		if (score > max_num+ 1e-10)
		{
			max_num = score;
			max_tag = z->first;
		}
	}
	return max_tag;
}
double linear_model::evaluate(dataset data)
{
	int c = 0, total = 0;
	for (auto z = data.sentences.begin(); z != data.sentences.end(); z++)
	{
		for (int z0 = 0; z0 < z->word.size(); z0++)
		{
			total++;
			string max_tag = maxscore_tag(*z, z0);
			string correct_tag = z->tag[z0];
			if (max_tag == correct_tag)
			{
				c++;
			}
		}
	}
	return (c / double(total));
	
}
void linear_model::online_training()
{
	double max_train_precision = 0;
	double max_dev_precision = 0;
	for (int i = 0; i < 20; i++)
	{
		for (auto sen = train.sentences.begin();sen!=train.sentences.end();sen++)
		{
			for (int pos = 0; pos < sen->word.size(); pos++)
			{
				string max_tag = maxscore_tag(*sen, pos);
				string correct_tag = sen->tag[pos];
				if (max_tag != correct_tag)
				{
					vector<string> fmax_tag = create_feature(*sen, pos, max_tag);
					vector<string> fcorrect_tag = create_feature(*sen, pos, correct_tag);
					for (auto z = fmax_tag.begin(); z != fmax_tag.end(); z++)
					{
						if (model.find(*z)!= model.end())
						{
							model[*z]--;
						}
					}
					for (auto z = fcorrect_tag.begin(); z != fcorrect_tag.end(); z++)
					{
						if (model.find(*z) != model.end())
						{
							model[*z]++;
						}
					}
				}
			}
		}
		double train_precision = evaluate(train);
		double dev_precision = evaluate(dev);
		cout << train.name << "=" << train_precision << endl;
		cout << dev.name << "=" << dev_precision << endl;
		if (train_precision > max_train_precision)
		{
			max_train_precision = train_precision;
		}
		if (dev_precision > max_train_precision)
		{
			max_dev_precision = dev_precision;
		}
	}
	cout << train.name << "=" << max_train_precision << endl;
	cout << dev.name << "=" << max_dev_precision << endl;
}

linear_model::linear_model()
{
	train.read_data("train");
	dev.read_data("dev");
}


linear_model::~linear_model()
{
}
