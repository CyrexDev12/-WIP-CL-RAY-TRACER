Notes: 

Compile Command 
mingw32-make

Clean Command: 
mingw32-make clean 


-------------------------------------------------------------------------------------------------------------

Project TODOS: 
Current Status: 

implementing Hexagon, can properly render, however we need to add it so you can add materials and patterns with no issues. 


This project is currently all entirely based off on the CPU. We plan to use CUDA using NVIDA technology. 


1. Finish CPU ray tracer.
Triangle, CSG


2. Add CPU multithreading.
    - Compare Times for more complex renders (with multi threading vs without)
3. Build CUDA mini-renderer with spheres and planes.
4. Compare render times.
5. Add reflection/refraction/Schlick to the GPU version.

-------------------------------------------------------------------------------------------------------------

- Tuple: A point like (-4, 4, 3)
(x,y,z,w)
For a point in 3D space, you typically use w = 1 → (x, y, z, 1)
For a direction/vector, you use w = 0 → (x, y, z, 0)

W = 1 -> Point 
W = 0 -> Vector 



Matrix Implementation 
Using 2D vector dynamic arrays

Matrix Class includes:
vector<vector<double>>
rows
cols

and all of the needed operations 

Matrix Transformations 

Translation, Scaling, Rotation, Shearing can all be represented as matricies 


Identifying Hits 
- When rendering a scene we will need to identify which one of the intersections is actually visible from the ray's origin. 
(Some can be behind the ray, and others may be hidden or behind by other objects)
- We call the visible intersection the 'hit' 
(The hit will never be behind the ray's origin) (Since it is effectivley behind the camera), so we can ignore all intersections with negative t values. The hit will always be the intersection with the lowest nonnegative t value. 


Moving the sphere 
When we increase the distance between the sphere and the ray, we can translate the ray away from the sphere, and it is indifferent from
translating the sphere away from the ray. 

Scaling 
- If we want to make our sphere bigger it is just the same as shrinking the distance between the ray and the sphere. 
- It is an inverse relationship. 
- We scale the ray by the inverse of how you were wanting to scale the sphere. 

Rotation 
- If we want to rotate the sphere, you rotate the sphere by the inverse of the rotation you wanted to apply to the sphere. 

To sum up transformations, whatever transformation we want to apply to the sphere, we apply the inverse to the ray. 


Lighting and Shading 

- We implement a model to simulate the reflection of light from a surface, which allows us to draw the sphere and make it look '3D' 

HOW TO: 
- Implement a source of light 
- Implement a shading algorithm to approximate how birhgtly that light illuminates the surfaces it shines on 

(Most ray tracers favor approximations over physically accurate simulations)

We define 4 different vectors 
If p is where your ray intersects an object, these four vectors are defined as: 
- E is the eye vector, pointing from p to the origin of the ray.
- L is the light vector, poiting from p to the position of the light source.
- N is the surface normal, a vector that is perpendicular to the surface at P. 
- R is the reflection vector, poiting in the direction that incoming light would bounce, or reflect. 

Phong Reflection Model 
Simulates interaction between three different types of lighting. 

- Ambient Reflection: Background lighting. Or light reflected from other objects in its environment. The Phong model treats this as a constant coloring all points on the surface equally. 
- Diffuse Reflection: Light reflected from a matte surface. It depends only on the angle between the light source and the surface normal. 
- Specular Reflection: Reflection of the light source itself and results in what is called a specular highlight. The bright spot on a curved surface. It depends only on the angle between the reflection vector and the eye vector and is controlled by a parameter that we cann shininess. The higher the shininess the smaller and tighter the specular highlight. 


The lighting Function (Located within the object class, e.g. sphere)
Expects 5 arguments material, point being illuminated, the light source, eye and normal vectors from the phong reflection model. 


Camera 

Map a 3-Dimensional scene onto a two dimensional canvas. 
Cameras Canvas will always be one unit away from the camera 



To Do: 

- Switch from using vector<double> to using a tuple class, so it is dealt with on the stack istead of heap to make the program faster. 



Shadows

Ray tracer computes shadows by casting a ray, called a shadow ray. From each point of intersection toward the light source. If something intersects that shadow
ray between the point and the light source, then the point is considered to be a shadow. 


Implementing New Objects 

Base class Shape* is an abstract class that handles generic operations like transformations and material assignment. 

Plane
- Normal is always straight up at (0, 1, 0)
- Local Intersect: If the ray is parallel to the plane (ray.direction.y is close to 0), it misses completeley. Otherwise, t = -rayorigin.y  ray.direction.y 

Patterns 
A pattern is a function that accepts a point in space and returns a color. 


To create better Render
- Update resolution usually (800, 400) is high resolution, (200, 100) is low resolution for the camera 


Pattern Design choice using Shared_Ptr vs Raw Pointers
- using std::shared_ptr instead of raw pointers for the pattern system fundementally shifts responsibility of memory management 
from the developers to the compiler. 

Patterns are unique because they are structural data, they don't live in one single palce, and their layouts are often shared or deeply nested. 

A ray tracer shoots millions of ray, and calls LocalPatternAt() on every hit. If a ray hits an object with a deleted pattern, it will access garbage memory, resulting in a immediate seg fault. 

Shared_ptr uses reference counting; 

- When we create a pattern the ref count is 1, 
- When we pass it to a shape the count becomes 2
- When the scene function eneds the local variable dies, dropping the count back to 1. 
- Because the count is not zero, the pattern stays alive in memory for the shape to safely use during rendering. 


relection 

We add a reflection attribute to material. 

When the reflective is 0, the surface is completely NONREFLECTIVE, whereas setting it to 1 produces a perfect mirror. Numbers in between will represent partial reflections. 

The prepare_computations function will pre-compute the reflectV vector. 

To do this we will create a plane and position a ray above it, slanting downward at a 45 degree angle. Position the intersection on the plane, and have prepare_computations() compute the reflection vector. 

ReflectedColor()
Create a new ray originating at the hits location and pointing in the diretion of reflectv. Find the color of the new ray via color_at(). 
Then multiply the result by the reflective value. If reflective is set to something between 0-1, it will give you partial reflection. 

Implement int remaining to color_at() reflectedColor() and shadeHit() to limit recursion calls 

Groups are abstract shapes with no surface of their own. Taking their form instead from the shapes they contain. 