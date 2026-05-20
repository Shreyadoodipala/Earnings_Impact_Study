#include "../include/common/Matrix.h"
#include <cmath>
#include <cassert>

Vector operator+(const Vector &v1, const Vector &v2) {
    assert(v1.size() == v2.size());
    int d = (int)v1.size();
    Vector U(d);
    for (int j = 0; j < d; j++)
        U[j] = v1[j] + v2[j];
    return U;
}

Vector operator-(const Vector &v1, const Vector &v2) {
    assert(v1.size() == v2.size());
    int d = (int)v1.size();
    Vector U(d);
    for (int j = 0; j < d; j++)
        U[j] = v1[j] - v2[j];
    return U;
}

Vector operator/(const Vector &v1, const Vector &v2) {
    assert(v1.size() == v2.size());
    int d = (int)v1.size();
    Vector U(d);
    for (int j = 0; j < d; j++)
        U[j] = v1[j] / v2[j];
    return U;
}

Vector operator*(const double &a, const Vector &v) {
    int d = (int)v.size();
    Vector U(d);
    for (int j = 0; j < d; j++)
        U[j] = a * v[j];
    return U;
}

Matrix operator-(const Matrix &m, const Vector &v) {
    int rows = (int)m.size();
    int cols = (int)m[0].size();
    assert((int)v.size() == cols);

    Matrix result(rows, Vector(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            result[i][j] = m[i][j] - v[j];
    }
    return result;
}

std::ostream &operator<<(std::ostream &os, const Vector &v) {
    for (int i = 0; i< (int)v.size(); i++)
        os << std::setw(12) << std::fixed << std::setprecision(3) << v[i] << " ";
    os << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    for (int i = 0; i < (int)m.size(); i++)
        os << m[i];
    return os;
}

Vector colMean(const Matrix &m) {
    int rows = (int)m.size();
    int cols = (int)m[0].size();
    Vector result(cols, 0.0);
    for (int j = 0; j < cols; j++)
    {
        for (int i = 0; i < rows; i++)
            result[j] += m[i][j];
        result[j] /= rows;
    }
    return result;
}

Vector calcCAAR(const Vector &AAR) {
    int d = (int)AAR.size();
    Vector CAAR(d);
    CAAR[0] = AAR[0];
    for (int i = 1; i < d; i++)
        CAAR[i] = CAAR[i - 1] + AAR[i];

    return CAAR;
}

Vector colStd(const Matrix &m) {
    int rows = (int)m.size();
    int cols = (int)m[0].size();

    Vector mean = colMean(m);
    Vector result(cols, 0.0);
    for (int j = 0; j < cols; j++)
    {
        double sum = 0.0;
        for (int i = 0; i < rows; i++)
        {
            double diff = m[i][j] - mean[j];
            sum += diff * diff;
        }
        result[j] = sqrt(sum / (rows - 1));
    }
    return result;
}

Vector dailyReturns(const Vector& prices) {
    Vector returns(prices.size() - 1);
    for (size_t i = 0; i < prices.size() - 1; i++) {
        returns[i] = log(prices[i + 1] / prices[i]);
    }
    return returns;
}

void printDates(const std::vector<std::string>& dates) {
    for (int i = 0; i < (int)dates.size(); i++)
    {
        std::cout << std::setw(14) << dates[i];
        if ((i + 1) % 10 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}

void printData(const Vector& prices) {
    for (int i = 0; i < (int)prices.size(); i++)
    {
        std::cout << std::fixed << std::setprecision(3) 
                  << std::setw(10) << prices[i];
        if ((i + 1) % 10 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}