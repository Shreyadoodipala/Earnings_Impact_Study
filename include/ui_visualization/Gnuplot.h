#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <string>
#include <vector>

class Gnuplot {
private:
    std::string title;
    std::string xlabel;
    std::string ylabel;
    std::vector<double> xData;
    std::vector<std::vector<double>> yData;
    FILE* gnuplotPipe;

public:
    Gnuplot(const std::string& title_,
            const std::string& xlabel_,
            const std::string& ylabel_,
            const std::vector<double>& xData_,
            const std::vector<std::vector<double>>& yData_);

    ~Gnuplot();

    bool plot(bool saveToFile = false);
};

#endif