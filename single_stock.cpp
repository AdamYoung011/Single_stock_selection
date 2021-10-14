#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <math.h>
#include "stock_cal.h"
using namespace std;
namespace fs = std::filesystem;

vector <string> name;
vector <double> d;
vector <int> index;
vector <double> Y;
vector <string> sliding_name = { "H2Q" };
//vector <string> sliding_name = { "Y2Y","Y2H","Y2Q","Y2M","H#","H2H","H2Q","H2M","Q#","Q2Q","Q2M","M#","M2M" };
vector <string> stock = { "AAPL","AXP","BA","CAT","CSCO","CVX","DIS","DD","GS","HD","IBM","INTC","JNJ","JPM","KO","MCD","MMM","MRK","MSFT","NKE","PFE","PG","TRV","UNH","RTX","V","VZ","WBA","WMT","XOM" };
//vector <string> stock = { "DJIA" };

double long_return_sum = 0, long_risk_sum = 0, long_trend_sum = 0, both_return_sum = 0, both_trend_sum = 0, both_risk_sum = 0;
double short_return_sum = 0, short_risk_sum = 0, short_trend_sum = 0;

//double expected_return = 0, risk = 0;
//美股

//計算趨勢值
double trend_ratio_cal(double expected_return, double risk)
{
	double trend_tmp = 0;

	if (expected_return > 0)
	{
		trend_tmp = expected_return / risk;
	}
	else
	{
		trend_tmp = expected_return * risk;
	}

	return trend_tmp;
}

//計算回歸趨勢線之值
void Y_cal(double expected_return, int times, double initial_money)
{
	for (int h = 1; h < times; h++)
	{
		Y.push_back(expected_return * h + initial_money);
	}
}

//每日風險
double risk_cal(double expected_return, int times, double initial_money, vector <double> money)
{
	double tmp = 0, risk_tmp = 0;

	Y_cal(expected_return, times, initial_money);
	for (int h = 0; h < times - 1; h++)
	{
		tmp = tmp + (money[h] - Y[h]) * (money[h] - Y[h]);
	}
	risk_tmp = sqrt(tmp / (times - 1));
	Y.clear();
	return risk_tmp;

	/*double tmp = 0, risk_tmp = 0;

	Y_cal(expected_return, times, initial_money);
	for (int h = 0; h < times - 1; h++)
	{
		tmp = tmp + abs(money[h] - Y[h]);
	}
	risk_tmp = tmp / double(times - 1);
	Y.clear();
	return risk_tmp;*/
}



//每日預期報酬
double expected_return_cal(vector <double> money, int times, double initial_money)
{
	double numerator = 0, denominator = 0, expected_return_tmp = 0;
	for (int h = 0, k = 1; h < times - 1; h++, k++)
	{
		numerator = numerator + (k * money[h] - k * initial_money);
		denominator = denominator + double(k * k);
	}
	expected_return_tmp = numerator / denominator;
	return expected_return_tmp;
}

//尋找指定股票的檔案位置
void find_stock_index(int stock_choose_number, string stock_name[], vector <int>& index)
{
	for (int h, p = 0; p < stock_choose_number; p++)
	{
		for (h = 0; h < name.size(); h++)
		{
			if (name[h] == stock_name[p])
			{
				index.push_back(h);
				break;
			}
		}
	}
}

//對讀檔進來的資料進行處理(處理逗號)
void data_processing(vector <string>& name, vector <double>& d, string str, int times)
{
	int start = 0;
	stringstream ss;
	string tmp;
	double number;

	for (int i = 0; i < str.size(); i++)
	{
		if (str[str.size() - 1] != ',')
		{
			str = str + ',';
		}

		if (str[i] == ',')
		{
			for (int j = start; j < i; j++)
			{
				tmp = tmp + str[j];
			}
			if (times == 0)
			{
				name.push_back(tmp);
			}
			else
			{
				ss << tmp;
				ss >> number;
				d.push_back(number);
				ss.str("");
				ss.clear();
			}
			tmp.clear();
			start = i + 1;
		}
	}
	start = 0;
}


void main_func(string slide)
{
	string stock_name[1], str_tmp;
	vector <string> str;
	int k = 0, times = 0;
	vector <double>  stock_price, remain_money, each_money, money;
	double divide_remain_money, initial_money = 10000000, expected_return = 0, risk = 0, short_trend = 0, long_trend = 0;
	vector <int> stock_buy_number;
	int stock_choose_number = 1, divide_money;
	vector <filesystem::path> file_name;
	double max_long_trend = 0, max_long_return = 0, max_long_risk = 0, max_short_trend = 0, max_short_return = 0, max_short_risk = 0;
	vector <double> long_money, short_money, both_fund;
	double both_expected_return = 0, both_risk = 0, both_trend = 0;
	vector <string> first_stock;

	//money >> 資金水位, remain_money >> 買後剩餘資金
	//stock_buy_number >> 可買張數, stock_price >> 每張買入價格
	//divide_remain_money >> 分配剩餘資金, stock_choose_number >> 選擇幾檔股票

	for (auto& p : fs::directory_iterator("./2010-2020_30_13種滑動視窗股價/" + slide + "/train"))
	{
		file_name.push_back(p.path());
	}

	for (int i = 0; i < file_name.size(); i++)
	{
		ifstream in(file_name[i]);

		for (int p = 0; p < stock.size(); p++)
		{
			stock_name[0] = stock[p];

			divide_money = initial_money / stock_choose_number;

			while (in >> str_tmp)
			{
				str.push_back(str_tmp);
			}

			for (int k = 0; k < str.size(); k++)
			{
				data_processing(name, d, str[k], times);
				times++;
			}

			for (int time = 0; time < times; time++)
			{
				//ofstream output("./single_stock/long/check.csv", ios_base::app);
				//if (p == 0)
				//{
					//data_processing(name, d, str[k], times);
				//}

				//如果讀進來是股票名，先搜尋指定股票的位置
				if (time == 0)
				{
					find_stock_index(stock_choose_number, stock_name, index);
				}
				else
				{
					if (time == 1)    //第一次買股票
					{

						for (int h = 0; h < stock_choose_number; h++)
						{
							stock_price.push_back(d[index[h]]);
							stock_buy_number.push_back(divide_money / stock_price[h]);
							remain_money.push_back(divide_money - stock_price[h] * stock_buy_number[h]);
							each_money.push_back(divide_money);
						}
						divide_remain_money = initial_money - (double)divide_money * stock_choose_number;

						money.push_back(initial_money);
					}
					else
					{
						double tmp = 0;
						for (int h = 0; h < stock_choose_number; h++)
						{
							each_money.push_back(d[index[h] + name.size() * (time - 1)] * stock_buy_number[h] + remain_money[h]);
							//each_money[time - 1] = d[index[h] + name.size() * (time - 1)] * stock_buy_number[h] + remain_money[h];
							tmp = tmp + each_money[time-1];
							if (h == stock_choose_number - 1)
							{
								tmp = tmp + divide_remain_money;
							}
						}
						money.push_back(tmp);
						tmp = 0;

						money[time - 1] = money[time - 1] + divide_remain_money;

						//output << "FS(" << times << "),";
						for (int h = 0; h < stock_choose_number; h++)
						{
							//output << fixed << setprecision(20) << each_money[h] << ",";
						}
						//output << fixed << setprecision(20) << money[times - 1] << endl;
					}
				}
			}
			

			expected_return = expected_return_cal(money, times, initial_money);
			risk = risk_cal(expected_return, times, initial_money, money);
			long_trend = trend_ratio_cal(expected_return, risk);
			short_trend= trend_ratio_cal(-expected_return, risk);

			if (i == 21 || i == 39)
			{
				ofstream output("./single_stock/short/" + slide + "/skip_first_rank.csv", ios_base::app);

				if (p == 0)
				{
					output << file_name[i] << endl;
					output << "Rank" << endl;
				}

				output << fixed << setprecision(20);
				output << "," << stock[p] << ",";
				output << "預期報酬," << expected_return << ",";
				output << "風險,"<< risk << ",";
				output << "趨勢值," << short_trend << endl;

				if (p == stock.size() - 1)
				{
					ofstream output("./single_stock/short/" + slide + "/skip_first_rank.csv", ios_base::app);
					output << endl << endl;
				}
			}

			if (long_trend > max_long_trend)
			{
				max_long_trend = long_trend;
				max_long_risk = risk;
				max_long_return = expected_return;
				long_money.clear();
				for (int g = 0; g < money.size(); g++)
				{
					long_money.push_back(money[g]);
				}

			}
			else if (max_long_trend == 0 && p == stock.size() - 1)
			{
				for (int g = 0; g < money.size(); g++)
				{
					long_money.push_back(initial_money);
				}
			}

			if (short_trend > max_short_trend)
			{
				max_short_trend = short_trend;
				max_short_risk = risk;
				max_short_return = expected_return;
				short_money.clear();
				for (int g = 0; g < money.size(); g++)
				{
					short_money.push_back(money[g]);
				}
				first_stock.clear();
				first_stock.push_back(stock[p]);
			}
			else if (max_short_trend == 0 && p == stock.size() - 1)
			{
				for (int g = 0; g < money.size(); g++)
				{
					short_money.push_back(initial_money);
				}
			}
			
			//output << "預期報酬," << fixed << setprecision(20) << expected_return << endl;
			//output << "風險," << fixed << setprecision(20) << risk << endl;
			//output << "趨勢值," << fixed << setprecision(20) << trend << endl;
			//output.close();
			expected_return = 0;
			risk = 0;
			long_trend = 0;
			short_trend = 0;
			money.clear();
			divide_money = 0;
			divide_remain_money = 0;
			stock_buy_number.clear();
			remain_money.clear();
			each_money.clear();
			stock_choose_number = 1;
			stock_name[0] = "";
			stock_price.clear();
			times = 0;
			index.clear();
			d.clear();
			name.clear();
		}
		if (first_stock.size() == 0)
		{
			cout << "0" << endl;
		}
		else
		{
			cout << first_stock[0] << endl;
		}
		first_stock.clear();

		long_return_sum += max_long_return;
		long_risk_sum += max_long_risk;
		long_trend_sum += max_long_trend;
		short_return_sum += max_short_return;
		short_risk_sum += max_short_risk;
		short_trend_sum += max_short_trend;

		for (int g = 0; g < long_money.size(); g++)
		{
			both_fund.push_back(long_money[g] + initial_money - short_money[g]);
		}
		both_expected_return = expected_return_cal(both_fund, both_fund.size(), 10000000);
		both_risk = risk_cal(both_expected_return, both_fund.size(), 10000000, both_fund);
		both_trend = (trend_ratio_cal(both_expected_return, both_risk));

		both_return_sum += both_expected_return;
		both_risk_sum += both_risk;
		both_trend_sum += both_trend;

		str.clear();
		max_long_return = 0;
		max_long_risk = 0;
		max_long_trend = 0;
		max_short_return = 0;
		max_short_risk = 0;
		max_short_trend = 0;
		both_expected_return = 0;
		both_fund.clear();
		both_risk = 0;
		both_trend = 0;
		short_money.clear();
		long_money.clear();
		//d.clear();
		//name.clear();
	}

	/*ofstream output1("./single_stock/DJI/long/long_average.csv", ios_base::app);
	output1 << fixed << setprecision(20);
	output1 << slide << "," << "預期報酬," << long_return_sum / file_name.size() << ",風險," << long_risk_sum / file_name.size() << ",趨勢值," << long_trend_sum / file_name.size() << endl;
	output1.close();

	ofstream output2("./single_stock/DJI/short/short_average.csv", ios_base::app);
	output2 << fixed << setprecision(20);
	output2 << slide << "," << "預期報酬," << short_return_sum / file_name.size() << ",風險," << short_risk_sum / file_name.size() << ",趨勢值," << short_trend_sum / file_name.size() << endl;
	output2.close();

	ofstream output3("./single_stock/DJI/both/both_average.csv", ios_base::app);
	output3 << fixed << setprecision(20);
	output3 << slide << "," << "預期報酬," << both_return_sum / file_name.size() << ",風險," << both_risk_sum / file_name.size() << ",趨勢值," << both_trend_sum / file_name.size() << endl;
	output3.close();*/

	name.clear();
	d.clear();
	long_return_sum = 0;
	long_risk_sum = 0;
	long_trend_sum = 0;
	short_trend_sum = 0;
	short_return_sum = 0;
	short_risk_sum = 0;
}

int main()
{
	for (int i = 0; i < sliding_name.size(); i++)
	{
		main_func(sliding_name[i]);
	}
}