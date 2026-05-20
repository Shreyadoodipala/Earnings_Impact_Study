#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

typedef std::vector<double> Vector;
typedef std::vector<Vector> Matrix;

Vector operator+(const Vector& v1, const Vector& v2); //calculate CI Upper
Vector operator-(const Vector& v1, const Vector& v2); //calculate AR / CI Lower
Vector operator/(const Vector& v1, const Vector& v2); //calculate t-statistic
Vector operator*(const double& a, const Vector& v); //calculate CI
Matrix operator-(const Matrix& m, const Vector& v); //calculate AR

std::ostream& operator<<(std::ostream& os, const Vector& v); //print Vector
std::ostream& operator<<(std::ostream& os, const Matrix& m); //print Matrix

Vector colMean(const Matrix& m); //calculate AAR, EAAR, ECAAR
Vector calcCAAR(const Vector& AAR); //calculate CAAR
Vector colStd(const Matrix& m); //calculate ARR_STD, CAAR_STD
Vector dailyReturns(const Vector& prices); //calculate daily returns

void printDates(const std::vector<std::string>& dates);   // print Dates
void printData(const Vector& prices);  // print Prices