#pragma once
#include <vector>
#include <cmath>
#include "cinder/gl/gl.h"
using namespace std;

class Particle {
public:
	float x, y;
	float xv, yv;
	float xf, yf;
	Particle(float _x = 0, float _y = 0,
		float _xv = 0, float _yv = 0) :
		x(_x), y(_y),
		xv(_xv), yv(_yv) {
	}
	void updatePosition(float timeStep) {
		// f = ma, m = 1, f = a, v = int(a)
		xv += xf;
		yv += yf;
		x += xv * timeStep;
		y += yv * timeStep;
	}
	void resetForce() {
		xf = 0;
		yf = 0;
	}
	void bounceOffWalls(float left, float top, float right, float bottom, float damping = .3) {
		bool collision = false;

		if (x > right){
			x = right;
			xv *= -1;
			collision = true;
		} else if (x < left){
			x = left;
			xv *= -1;
			collision = true;
		}

		if (y > bottom){
			y = bottom;
			yv *= -1;
			collision = true;
		} else if (y < top){
			y = top;
			yv *= -1;
			collision = true;
		}

		if (collision == true){
			xv *= damping;
			yv *= damping;
		}
	}
	void addDampingForce(float damping = .01) {
		xf = xf - xv * damping;
    yf = yf - yv * damping;
	}
	void draw() {
		glVertex2f(x, y);
	}
};
