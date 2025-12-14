#include "DroneSimulatorCore/Public/Controller/Throttle.h"


FPropellerSetThrottle::FPropellerSetThrottle(double in_front_left, double in_front_right, double in_rear_left, double in_rear_right)
	: front_left(in_front_left), front_right(in_front_right), rear_left(in_rear_left), rear_right(in_rear_right)
{}

FPropellerSetThrottle FPropellerSetThrottle::from_real(double real)
{
	return FPropellerSetThrottle(real, real, real, real);
}

FString FPropellerSetThrottle::to_string() const
{
	return FString::Printf(
		TEXT("{ front L=%.2f R=%.2f back L=%.2f R=%.2f }"),
		this->front_left,
		this->front_right,
		this->rear_left,
		this->rear_right);
}

FPropellerSetThrottle FPropellerSetThrottle::operator+(const FPropellerSetThrottle& other) const
{
	return FPropellerSetThrottle(
		this->front_left + other.front_left,
		this->front_right + other.front_right,
		this->rear_left + other.rear_left,
		this->rear_right + other.rear_right
	);
}

FPropellerSetThrottle FPropellerSetThrottle::operator-(const FPropellerSetThrottle& other) const
{
	return FPropellerSetThrottle(
		this->front_left - other.front_left,
		this->front_right - other.front_right,
		this->rear_left - other.rear_left,
		this->rear_right - other.rear_right
	);
}

FPropellerSetThrottle FPropellerSetThrottle::operator-() const
{
	return FPropellerSetThrottle(
		-this->front_left,
		-this->front_right,
		-this->rear_left,
		-this->rear_right
	);
}

FPropellerSetThrottle FPropellerSetThrottle::operator*(const FPropellerSetThrottle& other) const
{
	return FPropellerSetThrottle(
		this->front_left * other.front_left,
		this->front_right * other.front_right,
		this->rear_left * other.rear_left,
		this->rear_right * other.rear_right
	);
}

FPropellerSetThrottle FPropellerSetThrottle::operator*(double other) const
{
	return FPropellerSetThrottle(
		this->front_left * other,
		this->front_right * other,
		this->rear_left * other,
		this->rear_right * other
	);
}

FPropellerSetThrottle FPropellerSetThrottle::operator/(const FPropellerSetThrottle& other) const
{
	return FPropellerSetThrottle(
		this->front_left / other.front_left,
		this->front_right / other.front_right,
		this->rear_left / other.rear_left,
		this->rear_right / other.rear_right
	);
}

FPropellerSetThrottle FPropellerSetThrottle::operator/(double other) const
{
	return FPropellerSetThrottle(
		this->front_left / other,
		this->front_right / other,
		this->rear_left / other,
		this->rear_right / other
	);
}
