#pragma once
#include "Vector4.h"
#include "Vector3.h"

class Light {
public:
	Light() = default;
	Light(const Vector3& position, const Vector4& colour, float radius) :colour(colour), position(position), radius(radius) {}
	~Light() = default;

	Vector3		GetPosition() const { return position; }
	void		SetPosition(const Vector3& val) { position = val; }

	float		GetRadius() const { return radius; }
	void		SetRadius(const float val) { radius = val; }

	Vector4		GetColour() const { return colour; }
	void		SetColour(const Vector4& val) { colour = val; }

protected:
	Vector3		position;
	Vector4		colour;
	float		radius;
};

