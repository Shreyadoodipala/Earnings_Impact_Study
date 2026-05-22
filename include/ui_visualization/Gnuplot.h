#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <string>
#include <vector>

using namespace std;

class Gnuplot {
private:
    string title;
    string xlabel;
    string ylabel;
    vector<double> xData;
    vector<vector<double>> yData;
    FILE* gnuplotPipe;

public:
    Gnuplot(const string& title_,
            const string& xlabel_,
            const string& ylabel_,
            const vector<double>& xData_,
            const vector<vector<double>>& yData_);

    ~Gnuplot();

    bool plot();
};

#endif