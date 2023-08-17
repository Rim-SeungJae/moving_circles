#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

struct circle_t
{
	vec2	center=vec2(0);		// 2D position for translation
	float	radius=1.0f;		// radius
	float	theta=0.0f;			// rotation angle
	vec4	color;				// RGBA color in [0,1]
	vec2	velocity;			// velocity
	mat4	model_matrix;		// modeling transformation
	float previous_up = 0.0f;
	float previous_down = 0.0f;
	float previous_left = 0.0f;
	float previous_right = 0.0f;

	// public functions
	void	update(float dt);
};

inline bool is_collide(circle_t c1, circle_t c2)
{
	float dx = c1.center.x - c2.center.x;
	float dy = c1.center.y - c2.center.y;
	float distance = (float)sqrt(dx * dx + dy * dy);
	if (distance > c1.radius + c2.radius) return false;
	else return true;
}

inline float overlap_dist(circle_t c1, circle_t c2)
{
	float dx = c1.center.x - c2.center.x;
	float dy = c1.center.y - c2.center.y;
	float distance = (float)sqrt(dx * dx + dy * dy);
	return c1.radius + c2.radius - distance;
}

inline bool collide_up(circle_t c)
{
	float dx = 1.0f - c.center.x;
	if (dx > c.radius) return false;
	else return true;
}

inline float up_dist(circle_t c)
{
	return 1.0f - c.center.x;
}

inline bool collide_down(circle_t c)
{
	float dx = c.center.x + 1.0f;
	if (dx > c.radius) return false;
	else return true;
}

inline float down_dist(circle_t c)
{
	return 1.0f + c.center.x;
}

inline bool collide_right(circle_t c)
{
	float dy = 1.0f - c.center.y;
	if (dy > c.radius) return false;
	else return true;
}

inline float right_dist(circle_t c)
{
	return 1.0f - c.center.y;
}

inline bool collide_left(circle_t c)
{
	float dy = c.center.y + 1.0f;
	if (dy > c.radius) return false;
	else return true;
}

inline float left_dist(circle_t c)
{
	return 1.0f + c.center.y;
}

inline std::vector<circle_t> create_circles(int n)
{
	std::vector<circle_t> circles;
	circle_t c;
	
	srand((unsigned int)time(NULL));

	for (int i = 0;i < n;i++)
	{
		c = { vec2((float)rand() / (float)(RAND_MAX / 2) - 1.0f,(float)rand() / (float)(RAND_MAX / 2) - 1.0f),((float)rand() / (float)(RAND_MAX * 2)+0.2f)/(float)sqrt(n),0.0f,vec4((float)rand() / (float)(RAND_MAX),(float)rand() / (float)(RAND_MAX),(float)rand() / (float)(RAND_MAX),1.0f),vec2((float)rand() / (float)(RAND_MAX/2) - 1.0f, (float)rand() / (float)(RAND_MAX/2) - 1.0f) };
		std::vector<circle_t>::iterator it;
		for (it = circles.begin();it != circles.end();it++)
		{
			if (is_collide(*it, c)||collide_up(c)||collide_down(c)||collide_right(c)||collide_left(c))
			{
				break;
			}
		}
		if (it != circles.end())
		{
			i--;
			continue;
		}
		circles.emplace_back(c);
	}
	return circles;
}

inline void circle_t::update(float dt)
{
	//radius	= 0.35f;		// simple animation
	theta	= 0;
	float c	= cos(theta), s=sin(theta);
	center.x += velocity.x * dt;
	center.y += velocity.y * dt;

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

#endif
