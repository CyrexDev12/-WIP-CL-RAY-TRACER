#include "math/Matrix.h"
#include "iostream"
#include <cmath>
#include "math/Operations.h"
using namespace std; 


// Constructors
// Takes input r = rows, and c = collumns, as well as vector<double> to fill the matrix 
Matrix::Matrix(int r, int c, const vector<double>& arr) {
    int n = r * c;

    if (static_cast<std::size_t>(n) != arr.size()) {
        throw invalid_argument("Array is incorrectly sized!");
    }

    rows = r; 
    cols = c; 

    matrix.resize(r, vector<double>(c));

    for (int row = 0; row < r; row++) {
        for (int col = 0; col < c; col++) {
            matrix[row][col] = arr[row * c + col];
        }
    }

}

Matrix::Matrix(int r, int c, vector<vector<double>>& m) {
    rows = r; 
    cols = c; 
    matrix = m; 
}

// Default constructor 
// Constructs 4 * 4 identify matrix 
Matrix::Matrix() {
    rows = 4;
    cols = 4;

    matrix = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };
}


Matrix Matrix::scalarMultiply(double& scalar) {

    vector<vector<double>> result(rows, vector<double>(cols));

    for (std::size_t i = 0; i < matrix.size(); i++) {

        for (std::size_t j = 0; j < matrix[i].size(); j++) {
            result[i][j] = matrix[i][j] * scalar;
        }

    }

    return Matrix(rows, cols, result);

}



Matrix Matrix::multiplyMatrix(Matrix& b) {

    int rowsA = rows; 
    int colsA = cols; 
    int rowsB = b.getRows(); 
    int colsB = b.getCols(); 

    // To multiply two matrix A's col must = matrix B's rows
    if (cols != rowsB) {
        throw invalid_argument("Matrix b does does not have the same number of cols of matrix A!");
    }


   vector<vector<double>> result(rows, vector<double>(b.getCols(), 0));

    for (int row = 0; row < rowsA; row++) {
        for (int col = 0; col < colsB; col++) {
            for (int k = 0; k < colsA; k++) {
                result[row][col] += matrix[row][k] * b.matrix[k][col];
            }
        }
    }

    
    return Matrix(rowsA, colsB, result);
    

}

    // Returns the transpose of the matrix 
   Matrix Matrix::transpose() {
        vector<vector<double>> result(cols, vector<double>(rows));

        for (std::size_t i = 0; i < matrix.size(); i++) {

            for (std::size_t j = 0; j < matrix[i].size(); j++) {
                result[j][i] = matrix[i][j];
            }
        }

        return Matrix(cols, rows, result);
    }

    // Computes the inverse of a matrix
    Matrix Matrix::inverse() {
        double deter = determinant();

        if (deter == 0) {
            // Matrix is singular and is therefore not invertible 
            throw invalid_argument("Matrix is not invertible! It is singular and the determinant is 0");
        }

        // 1. Get CoFactor Matrix 
        Matrix Cof = cofactorMatrix();
        // 2. Transpose CoFactor Matrix 
        Matrix CofT = Cof.transpose(); 
        // 3. Divide each of the resuluting elements by the determinant of the original matrix 
        double scalar = 1 / deter; 
        Matrix inv = CofT.scalarMultiply(scalar);


        return inv; 

    }


    double Matrix::determinant() {
    if (rows != cols) {
        throw invalid_argument("Determinant can only be computed for a square matrix!");
    }

    return determinantHelper(matrix);
}

double Matrix::minorDeterminant(int skipRow, int skipCol) {
    vector<vector<double>> minor;

    for (int row = 0; row < rows; row++) {
        if (row == skipRow) {
            continue;
        }

        vector<double> newRow;

        for (int col = 0; col < cols; col++) {
            if (col == skipCol) {
                continue;
            }

            newRow.push_back(matrix[row][col]);
        }

        minor.push_back(newRow);
    }

    return determinantHelper(minor);
}

Matrix Matrix::cofactorMatrix() {
    if (rows != cols) {
        throw invalid_argument("Cofactor matrix can only be computed for a square matrix!");
    }

    vector<vector<double>> cofactors(rows, vector<double>(cols));

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            double minorDet = minorDeterminant(row, col);

            double sign = ((row + col) % 2 == 0) ? 1 : -1;

            cofactors[row][col] = sign * minorDet;
        }
    }

    return Matrix(rows, cols, cofactors);
}

double Matrix::determinantHelper(vector<vector<double>> mat) {
    int n = mat.size();

    // Base case: 1x1
    if (n == 1) {
        return mat[0][0];
    }

    // Base case: 2x2
    if (n == 2) {
        return (mat[0][0] * mat[1][1]) -
               (mat[0][1] * mat[1][0]);
    }

    double det = 0;

    // Expand along first row
    for (int col = 0; col < n; col++) {
        vector<vector<double>> minor;

        for (int row = 1; row < n; row++) {
            vector<double> newRow;

            for (int currCol = 0; currCol < n; currCol++) {
                if (currCol == col) {
                    continue;
                }

                newRow.push_back(mat[row][currCol]);
            }

            minor.push_back(newRow);
        }

        double sign = (col % 2 == 0) ? 1 : -1;

        det += sign * mat[0][col] * determinantHelper(minor);
    }

    return det;
}

    // Multiplies matrix by tuple, input args tuple should be (x, y, z, w) 4 elements
    // W = 1 -> Point
    // W = 0 -> Vector 
   vector<double> Matrix::multiplyTuple(const vector<double>& tuple) {
    if (tuple.size() != 4) {
        throw invalid_argument("Tuple size must be 4!");
    }

    // Convert tuple to 4x1 matrix
    Matrix m(4, 1, tuple);

    Matrix p = multiplyMatrix(m);

    // Extract x, y, z, w from 4x1 result
    double x = p.matrix[0][0];
    double y = p.matrix[1][0];
    double z = p.matrix[2][0];
    double w = p.matrix[3][0];

    vector<double> result = {x, y, z, w};

    return result;
}

    // Matrix factory function 
    // Creates 4 x 4 matrix to tell our system to move it by points x, y, z
    Matrix Matrix::translation(double x, double y, double z) {
    vector<double> arr = {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    };

    return Matrix(4, 4, arr);
}


    // Scales vectors and points 
    // Returns a 4 * 4 matrix 
    // Matrix factory function 
    // Sclales along the diagonal
    Matrix Matrix::scale(double x, double y, double z) {
        vector<double> arr = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };

    return Matrix(4, 4, arr);
    }


    // Shearing transformation
    // xy: x changes in proportion to y
    // xz: x changes in proportion to z
    // yx: y changes in proportion to x
    // yz: y changes in proportion to z
    // zx: z changes in proportion to x
    // zy: z changes in proportion to y
    Matrix Matrix::shearing(double xy, double xz, double yx, double yz, double zx, double zy) {
        vector<double> arr = {
            1,  xy, xz, 0,
            yx, 1,  yz, 0,
            zx, zy, 1,  0,
            0,  0,  0,  1
        };

        return Matrix(4, 4, arr);
    }



    // For rotation around the x-axis, x stays the same, while y and z change:
    Matrix Matrix::rotateX(double r) {

        vector<double> arr = {
        1, 0, 0, 0,
        0, cos(r), -sin(r), 0,
        0, sin(r), cos(r), 0,
        0, 0, 0, 1
    };

    return Matrix(4, 4, arr);
    }

    // For rotation around the y-axis, y stays the same, while x and z change:
    Matrix Matrix::rotateY(double r) {
        vector<double> arr = {
        cos(r), 0, sin(r), 0,
         0, 1, 0, 0,
        -sin(r), 0, cos(r), 0,
        0, 0, 0, 1
        };

        return Matrix(4, 4, arr);

    }

     // For rotation around the z-axis, z stays the same, while x and y change:
    Matrix Matrix::rotateZ(double r) {
        vector<double> arr = {
    cos(r), -sin(r), 0, 0,
    sin(r), cos(r), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
    };


    return Matrix(4, 4, arr);

    }

 // Returns a view orientation matrix 
Matrix Matrix::ViewOrientation(vector<double>& left, vector<double>& trueUp, vector<double>& forward) {
    // Corrected to map x->0, y->1, z->2 properly for all three axes
    vector<double> arr = {
         left[0],     left[1],     left[2],    0.0,
       trueUp[0],   trueUp[1],   trueUp[2],    0.0,
     -forward[0], -forward[1], -forward[2],    0.0,
             0.0,         0.0,         0.0,    1.0
    };

    return Matrix(4, 4, arr); 
}

// Translates the eye vector 
// pretends the eye moves, not the world itself
// Params; from -> the point at which you are moving from, to -> The point in the scene in which you want to look, up -> a vector indicating which direction is up
// MUST BE TUPLES 
Matrix Matrix::viewTransformation(vector<double>& from, vector<double>& to, vector<double>& up) {
    // 1. Calculate and normalize forward vector (ensure w=0 if your SubtractTuples doesn't)
    vector<double> forward = NormalizeTuple(SubtractTuples(to, from)); 
    
    // 2. Normalize up helper vector
    vector<double> upN = NormalizeTuple(up); 

    // 3. Compute left and trueUp (cross product logic should handle 4D or drop w)
    vector<double> left = CrossProduct(forward, upN); 
    vector<double> trueUp = CrossProduct(left, forward); 

    Matrix m; // Identity Matrix
    Matrix orMatrix = m.ViewOrientation(left, trueUp, forward); 
    Matrix trans = m.translation(-from[0], -from[1], -from[2]); 

    return orMatrix.multiplyMatrix(trans); 
}
  

    // Debug functions 

    void Matrix::printMatrix() {
    for (std::size_t row = 0; row < matrix.size(); row++) {
        for (std::size_t col = 0; col < matrix[row].size(); col++) {
            cout << matrix[row][col] << " ";
        }
        cout << endl;
    }
}


