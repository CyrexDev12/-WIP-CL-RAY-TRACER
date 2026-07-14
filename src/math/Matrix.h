#ifndef MATRIX_H
#define MATRIX_H
#include <vector>
using namespace std; 

// Using 2D vectors to represent matricies 
// This class includes all of the operations required for matricies in a ray tracing engine 
// Author: Luke Neumann 

class Matrix {
    private:

    int rows; 
    int cols; 
    vector<vector<double>> matrix; 

    public:

    // Constructors
    Matrix(int r, int c, const vector<double>& arr);  
    Matrix(int r, int c, vector<vector<double>>& m);
    Matrix(); 

    // Getters 

    int getRows() const {
        return rows; 
    }

    int getCols() const {
        return cols; 
    }

    double at(int row, int col) const {
        return matrix.at(row).at(col);
    }


    // Standard Operations 
    
    Matrix multiplyMatrix(Matrix& b);
    Matrix transpose();
    Matrix inverse();
    Matrix cofactorMatrix();
    Matrix scalarMultiply(double& scalar);
    Matrix translation(double x, double y, double z);
    Matrix scale(double x, double y, double z);
    Matrix shearing(double xy, double xz, double yx, double yz, double zx, double zy);
    Matrix ViewOrientation(vector<double>& left, vector<double>& trueUp, vector<double>& forward); 
    Matrix viewTransformation(vector<double>& from, vector<double>& to, vector<double>& up);
    vector<double> multiplyTuple(const vector<double>& tuple);
    // Rotations 
    Matrix rotateX(double radians);
    Matrix rotateY(double radians);
    Matrix rotateZ(double radians);
    // Calculations returned as double 
    double determinant(); 
    double determinantHelper(vector<vector<double>> mat);
    double minorDeterminant(int skipRow, int skipCol);
    
    
    // Debug Functions 

    void printMatrix();

};











#endif
