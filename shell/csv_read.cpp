#include<iostream> //入出力ライブラリ
#include<fstream> //iostreamのファイル入出力をサポート
#include<string>
#include<sstream>
#include<vector>

using namespace std;

enum {
	

	RFJ_col = 0, // Column 1 in .CSV File
	AFJ_col = 1, // Column 2 in .CSV File
	TR_col  = 2, // Column 3 in .CSV File
	AR_col  = 3, // Column 4 in .CSV File
	SD_col  = 4, // Column 5 in .CSV File
	ENERGY  = 5, // Column 6 in .CSV File
	MDLINE  = 6  // Column 7 in .CSV File
};

#define FileFirstIndex 40
#define FileIndexDuration 5

int main(int argc, char **argv){
	string str;
	vector<float> rfj_acc, afj_acc, TargetResponse_acc, AverageResponse_acc, StandardDeviation_acc, energy_acc;
	vector<int> miss_dline_acc;
	float rfj = (float) 0.0, afj = (float) 0.0, TargetResponse  = (float) 0.0, AverageResponse = (float) 0.0, StandardDeviation = (float) 0.0, energy = (float) 0.0;
	int miss_dline = (int) 0;
	short state, file_index;
	file_index = (short) FileFirstIndex;
	char filename[100], output_FileName[100];

	if(argc != 4) {
		cout << "only " << argc << " arguements are given" << endl;
		cout << "argv[1]: 1) NonDVFS, 2) DVFS.NonOverhead, 3) DVFS.Overhead" << endl;
		cout << "Task#" << endl;
		cout << "Experiment#" << endl;
		exit(1);
	}

	sprintf(output_FileName,"Total_%s.%s.csv", argv[1], argv[2]);
	ofstream ofs(output_FileName); //ファイル出力ストリーム

while(file_index < 100 + (short) FileIndexDuration) {
	rfj = (float) 0.0; afj = (float) 0.0; TargetResponse  = (float) 0.0; AverageResponse = (float) 0.0;
	StandardDeviation = (float) 0.0; energy = (float) 0.0;
	rfj_acc.clear(); afj_acc.clear();
	TargetResponse_acc.clear(); AverageResponse_acc.clear();
	StandardDeviation_acc.clear(); miss_dline_acc.clear(); energy_acc.clear(); 
	
	sprintf(filename,"../result/csv/Experiment_%s/Alpha_%d/%s.%s.csv", argv[3], (int) file_index, argv[1], argv[2]);
	cout << "Reading " << filename << endl;
	//ファイルの読み込み
	ifstream ifs(filename);
 	if(!ifs){
		cout<<"入力エラー";
		exit(1);
	}

    //csvファイルを1行ずつ読み込む
    while(getline(ifs,str)) {
        string token;
        istringstream stream(str);
	state = (short) RFJ_col;

        //1行のうち、文字列とコンマを分割する
        while(getline(stream,token,',')) {
            //すべて文字列として読み込まれるため
            //数値は変換が必要
            if(state == (short) RFJ_col) {
		rfj_acc.push_back(stof(token));
		cout << stof(token) << ",";
	    }
            else if(state == (short) AFJ_col) {
		afj_acc.push_back(stof(token));
		cout << stof(token) << ",";
	    }
            else if(state == (short) TR_col) {
		TargetResponse_acc.push_back(stof(token));
		cout << stof(token) << ",";
	    }
            else if(state == (short) AR_col) {
		AverageResponse_acc.push_back(stof(token));
		cout << stof(token) << ",";
	    }
            else if(state == (short) SD_col) {
		StandardDeviation_acc.push_back(stof(token));
		cout << stof(token) << ",";
	    }
            else if(state == (short) ENERGY) {
		energy_acc.push_back(stof(token));
		cout << stof(token);
	    }
            else  { // MDLINE
		miss_dline_acc.push_back(stof(token));
		cout << stof(token);
	    }
		state += 1;
        }
        cout << endl;
    }
	if(rfj_acc.size() != afj_acc.size() || rfj_acc.size() != energy_acc.size() || afj_acc.size() != energy_acc.size()) {
		cout << "The number of contents among all column is not consistent" << endl;
		exit(1);
	}

	for(unsigned int i = 0; i < rfj_acc.size(); i++) {
		rfj += rfj_acc[i];
		afj += afj_acc[i];
		TargetResponse += TargetResponse_acc[i];
		AverageResponse += AverageResponse_acc[i];
		StandardDeviation += StandardDeviation_acc[i];
		energy += energy_acc[i];
		miss_dline += miss_dline_acc[i];
	}
	//for(unsigned int i = 0; i < energy_acc.size(); i++) 
	//	energy += energy_acc[i];
	rfj = rfj / rfj_acc.size(); //cout << "£"<< rfj << ",";
	afj = afj / afj_acc.size(); //cout << afj << ",";
	TargetResponse = TargetResponse / TargetResponse_acc.size(); //cout << TargetResponse << ",";
	AverageResponse = AverageResponse / AverageResponse_acc.size(); //cout << AverageResponse << ",";
	StandardDeviation = StandardDeviation / StandardDeviation_acc.size(); //cout << StandardDeviation << ",";
	energy = energy / energy_acc.size(); //cout << energy << ",";
	miss_dline = miss_dline / miss_dline_acc.size(); //cout << miss_dline << endl;

	ofs << file_index << ","  
	    << rfj << ","
	    << afj << ","
	    << TargetResponse << ","
	    << AverageResponse << ","
	    << StandardDeviation << ","
	    << energy << ","
	    << miss_dline << endl;

	file_index += (short) FileIndexDuration;
	ifs.close();
}
    return 0;
}

