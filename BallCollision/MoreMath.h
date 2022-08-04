#pragma once
#include "SFML/Graphics.hpp"

float dot(sf::Vector2f v1, sf::Vector2f v2)
{
	return (v1.x * v2.x + v1.y * v2.y);
}

float sqr_magnitude(sf::Vector2f v)
{
	return dot(v, v);
}

float magnitude(sf::Vector2f v)
{
	return std::sqrtf(sqr_magnitude(v));
}

sf::Vector2f normalized(sf::Vector2f v)
{
	float mag = magnitude(v);
	return sf::Vector2f(v.x / mag, v.y / mag);
}

sf::Vector2f rotated(sf::Vector2f v, float angle)
{
	auto cos = std::cosf(angle);
	auto sin = std::sinf(angle);

	return sf::Vector2f(cos * v.x - sin * v.y,
						cos * v.y + sin * v.x);
}

sf::Vector2f reflected(sf::Vector2f v, sf::Vector2f normal)
{
	return v - normal * 2.f * dot(v, normal);
}