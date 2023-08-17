<img src="https://img.shields.io/badge/C-A8B9CC?style=flat&logo=C&logoColor=white"/> <img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=C++&logoColor=white"/> <img src="https://img.shields.io/badge/OpenGL-5586A4?style=flat&logo=OpenGL&logoColor=white"/>
# moving_circles
This repository is about the first assignment of SKKU's 2021 Introduction to Computer Graphics
![cg1](https://github.com/dipreez/moving_circles/assets/50349104/70a12924-b41a-4ecd-a388-ff640bf5dbf6)

# Algorithms and data structures
## Circles generation
When the program initiates 20 circles are created with random position,
radius, color, velocity. Also, whenever “+/-“ keys are pressed the number
of circles is increased/decreased.
## Circle movement
Every time render() function is called, it calls glfwGetTIme() function and
saves its value. render() function compares current glfwGetTime() and
previously saved glfwGetTime() value to calculate the time interval
between the current and previous frame. According to this time interval,
the circle is moved.
## Collision detection and reaction
For collision detection, real-number measurement approach is used.
Especially For collision between circles, 2-dimentional vector object is
declared to represent a graph data structure. This graph stores
information about how much circles overlap. In this graph, if the value
of the current frame is bigger than that of the previous frame, it means
that the two circles are colliding(value 0 means that the two circles are
not overlapped).

# Discussions
1. There is a problem that some circles disappear over time after the
program runs. It seems to be an error in handling the conflict between
circles, but couldn't find a clear solution.
2. There seems to be a problem with the formula for calculating elastic
collisions. I used a coordinate transformation based on a straight line
connecting the two conflicting circles. However, The overall speed of
the circles slows down and the formula seems too complicated. I think
there's a better way.
