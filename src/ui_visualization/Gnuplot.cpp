#include "../include/ui_visualization/Gnuplot.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <limits>

using namespace std;

Gnuplot::Gnuplot(const string& title_,
                 const string& xlabel_,
                 const string& ylabel_,
                 const vector<double>& xData_,
                 const vector<vector<double>>& yData_)
    : title(title_), xlabel(xlabel_), ylabel(ylabel_), xData(xData_), yData(yData_), gnuplotPipe(nullptr) {
    gnuplotPipe = popen("gnuplot -persist", "w");
    if (!gnuplotPipe) {
        cout << "gnuplot not found..." << '\n';
    }
}

Gnuplot::~Gnuplot() {
    if (gnuplotPipe) {
        fprintf(gnuplotPipe, "exit\n");
        pclose(gnuplotPipe);
        gnuplotPipe = nullptr;
    }
}

bool Gnuplot::plot() {
    if (xData.empty() || yData.empty() || yData.size() > 3) {
        cout << "invalid input data" << '\n';
        return false;
    }

    for (const vector<double>& series : yData) {
        if (series.size() != xData.size()) {
            cout << "invalid input data" << '\n';
            return false;
        }
    }

    if (!gnuplotPipe) {
        cout << "gnuplot pipe not initialized" << '\n';
        return false;
    }

    vector<string> tempFileNames;
    tempFileNames.reserve(yData.size());
    for (size_t i = 0; i < yData.size(); ++i) {
        tempFileNames.push_back("tempData" + to_string(i + 1) + ".dat");
        remove(tempFileNames.back().c_str());
    }

    for (size_t seriesIndex = 0; seriesIndex < yData.size(); ++seriesIndex) {
        FILE* tempDataFile = fopen(tempFileNames[seriesIndex].c_str(), "w");
        if (!tempDataFile) {
            for (const string& fileName : tempFileNames) {
                remove(fileName.c_str());
            }
            cout << "failed to create " << tempFileNames[seriesIndex] << '\n';
            return false;
        }

        for (size_t i = 0; i < xData.size(); ++i) {
            fprintf(tempDataFile, "%lf %lf\n", xData[i], yData[seriesIndex][i]);
        }
        fclose(tempDataFile);
    }

    fprintf(gnuplotPipe, "set grid\n");
    fprintf(gnuplotPipe, "set title '%s'\n", title.c_str());
    fprintf(gnuplotPipe, "set arrow from 0,graph(0,0) to 0,graph(1,1) nohead lc rgb 'red'\n");
    fprintf(gnuplotPipe, "set xlabel '%s'\nset ylabel '%s'\n", xlabel.c_str(), ylabel.c_str());
    fprintf(gnuplotPipe, "set lmargin 12\n");
    fprintf(gnuplotPipe, "set rmargin 6\n");
    fprintf(gnuplotPipe, "set tmargin 4\n");
    fprintf(gnuplotPipe, "set bmargin 5\n");

    ostringstream plotCommand;

    vector<string> labels = {"Beat", "Meet", "Miss"};
    vector<string> colors = {"green", "blue", "red"};

    plotCommand << "plot ";
    for (size_t i = 0; i < tempFileNames.size(); ++i) {
        if (i > 0) {
            plotCommand << ", ";
        }
        plotCommand << "\"" << tempFileNames[i]
            << "\" with lines lc rgb '" << colors[i]
            << "' title '" << labels[i] << "'";
    }
    plotCommand << "\n";

    fprintf(gnuplotPipe, "%s", plotCommand.str().c_str());
    fflush(gnuplotPipe);

    cout << "press enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();

    for (const string& fileName : tempFileNames) {
        remove(fileName.c_str());
    }
    return true;
}