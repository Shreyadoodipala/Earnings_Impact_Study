#include "../include/ui_visualization/Gnuplot.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <limits>

Gnuplot::Gnuplot(const std::string& title_, const std::string& xlabel_, const std::string& ylabel_,
                 const std::vector<double>& xData_, const std::vector<std::vector<double>>& yData_)
    : title(title_), xlabel(xlabel_), ylabel(ylabel_), xData(xData_), yData(yData_), gnuplotPipe(nullptr) {
    gnuplotPipe = popen("gnuplot -persist", "w");
    if (!gnuplotPipe) {
        std::cout << "gnuplot not found..." << std::endl;
    }
}

Gnuplot::~Gnuplot() {
    if (gnuplotPipe) {
        fprintf(gnuplotPipe, "exit\n");
        pclose(gnuplotPipe);
        gnuplotPipe = nullptr;
    }
}

bool Gnuplot::plot(bool saveToFile) {
    if (xData.empty() || yData.empty() || yData.size() > 3) {
        std::cout << "invalid input data" << std::endl;
        return false;
    }

    for (const std::vector<double>& series : yData) {
        if (series.size() != xData.size()) {
            std::cout << "invalid input data" << std::endl;
            return false;
        }
    }

    if (!gnuplotPipe) {
        std::cout << "gnuplot pipe not initialized" << std::endl;
        return false;
    }

    std::vector<std::string> tempFileNames;
    tempFileNames.reserve(yData.size());
    for (size_t i = 0; i < yData.size(); ++i) {
        tempFileNames.push_back("tempData" + std::to_string(i + 1) + ".dat");
        std::remove(tempFileNames.back().c_str());
    }

    for (size_t seriesIndex = 0; seriesIndex < yData.size(); ++seriesIndex) {
        FILE* tempDataFile = std::fopen(tempFileNames[seriesIndex].c_str(), "w");
        if (!tempDataFile) {
            for (const std::string& fileName : tempFileNames) {
                std::remove(fileName.c_str());
            }
            std::cout << "failed to create " << tempFileNames[seriesIndex] << std::endl;
            return false;
        }

        for (size_t i = 0; i < xData.size(); ++i) {
            fprintf(tempDataFile, "%lf %lf\n", xData[i], yData[seriesIndex][i]);
        }
        std::fclose(tempDataFile);
    }

    if (saveToFile) {
        fprintf(gnuplotPipe, "set terminal pngcairo enhanced font 'Arial,12' fontscale 1.0\n");
        int N = xData.size() / 2;
        std::string file_name = "results/CAAR_Plot_N" + std::to_string(N) + ".png";
        fprintf(gnuplotPipe, "set output '%s'\n", file_name.c_str());
    }

    fprintf(gnuplotPipe, "set grid\n");
    fprintf(gnuplotPipe, "set title '%s'\n", title.c_str());
    fprintf(gnuplotPipe, "set arrow from 0,graph(0,0) to 0,graph(1,1) nohead lc rgb 'red'\n");
    fprintf(gnuplotPipe, "set xlabel '%s'\nset ylabel '%s'\n", xlabel.c_str(), ylabel.c_str());
    fprintf(gnuplotPipe, "set lmargin 12\n");
    fprintf(gnuplotPipe, "set rmargin 6\n");
    fprintf(gnuplotPipe, "set tmargin 4\n");
    fprintf(gnuplotPipe, "set bmargin 5\n");

    std::ostringstream plotCommand;

    std::vector<std::string> labels = {"Beat", "Meet", "Miss"};
    std::vector<std::string> colors = {"green", "blue", "red"};

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

    std::cout << "press enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    for (const std::string& fileName : tempFileNames) {
        std::remove(fileName.c_str());
    }
    return true;
}