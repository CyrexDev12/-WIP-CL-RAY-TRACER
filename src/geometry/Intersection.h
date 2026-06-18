// Intersection.h
#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include <iostream>

// Forward declaration of Shape so compiler knows it exists
class Shape; 

class Intersection {
private: 
    double t;  
    const Shape* object; 

public:
    Intersection(double t, const Shape* object) : t(t), object(object) {}

    double getT() const { return t; }
    void setT(double tVal) {t = tVal; }
    // Pass pointer variable holding the memory address for shape 
    const Shape* getObject() const { return object; }
    void setObj(Shape* obj) {object = obj; }

    // Operator overloading to easily sort intersections by distance
    bool operator<(const Intersection& other) const {
        return this->t < other.t;
    }


    // Overload << operator to easily print a single intersection
    friend std::ostream& operator<<(std::ostream& os, const Intersection& intersection) {
        os << "Intersection(t: " << intersection.t << ", object address: " << intersection.object << ")";
        return os;
    }


};


//TODO: Imeplement a sort algorithm to sort the intersections in ascending order. 
class Intersections {
private:
    std::vector<Intersection> intersectionList;

public:
    // Add an intersection to our collection
    void addIntersection(const Intersection& intersection) {
        intersectionList.push_back(intersection);
    }

    // Retrieve all intersections found
    const std::vector<Intersection>& getIntersections() const {
        return intersectionList;
    }

    // Clear list between rays
    void clear() {
        intersectionList.clear();
    }

    bool empty() {
        if (intersectionList.empty()) {
            return true;
        }

        return false; 
    }
    // Returns the closest valid intersection point (t > 0)
    // Returns -1.0 if there is no valid hit in front of the ray
    // Update: Returns an intersection obj instead of a double val
    const Intersection* hit() const;

    // Helper to get the actual object pointer of the closest hit
    const Shape* hitObject() const;

    // Sort function to sort the intersection list into ascending order
    void Sort(); 

     // Added: Member function to print the entire list
    void print() const {
        std::cout << "--- Intersections List (Count: " << intersectionList.size() << ") ---\n";
        for (size_t i = 0; i < intersectionList.size(); ++i) {
            std::cout << "[" << i << "] " << intersectionList[i] << "\n";
        }
    }
};

#endif
