#pragma once
#include <algorithm>
#include <cmath>
#include <utility>
#include <type_traits>

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
struct Rect;

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
struct Vec2
{
	T X, Y;

	constexpr Vec2(T x, T y):
		X(x),
		Y(y)
	{
	}

	template<typename S, typename = std::enable_if<std::is_constructible_v<T, S>, void>>
	constexpr explicit Vec2(Vec2<S> other):
		X(other.X),
		Y(other.Y)
	{
	}

	inline bool operator==(Vec2<T> other) const
	{
		return X == other.X && Y == other.Y;
	}

	inline bool operator!=(Vec2<T> other) const
	{
		return X != other.X || Y != other.Y;
	}

	template<typename S>
	constexpr inline Vec2<decltype(std::declval<T>() + std::declval<S>())> operator+(Vec2<S> other) const
	{
		return Vec2(X + other.X, Y + other.Y);
	}

	inline Vec2<T> operator-() const
	{
		return Vec2(-X, -Y);
	}

	template<typename S>
	inline Vec2<decltype(std::declval<T>() - std::declval<S>())> operator-(Vec2<S> other) const
	{
		return Vec2(X - other.X, Y - other.Y);
	}

	template<typename S, typename = std::enable_if<std::is_arithmetic_v<S>, void>>
	constexpr inline Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(S other) const
	{
		return Vec2(X * other, Y * other);
	}

	template<typename S, typename = std::enable_if<std::is_arithmetic_v<S>, void>>
	inline Vec2<decltype(std::declval<T>() / std::declval<S>())> operator/(S other) const
	{
		return Vec2(X / other, Y / other);
	}

	template<typename S>
	inline Vec2<T> &operator+=(Vec2<S> other)
	{
		return *this = *this + other;
	}

	template<typename S>
	inline Vec2<T> &operator-=(Vec2<S> other)
	{
		return *this = *this - other;
	}

	template<typename S>
	inline Vec2<T> &operator*=(Vec2<S> other)
	{
		return *this = *this * other;
	}

	template<typename S>
	inline Vec2<T> &operator/=(Vec2<S> other)
	{
		return *this = *this / other;
	}

	inline Vec2<T> Floor() const
	{
		return Vec2<T>(std::floor(X), std::floor(Y));
	}

	// Return a rectangle starting at origin, whose dimensions match this vector
	template<typename = std::enable_if<std::is_integral_v<T>, void>>
	inline Rect<T> OriginRect() const
	{
		return RectSized(Vec2<T>::Zero, *this);
	}

	constexpr static Vec2<T> Zero = Vec2<T>(0, 0);
};

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
struct Mat2
{
	// ⎛A B⎞
	// ⎝C D⎠, acting on column vectors
	T A, B, C, D;

	constexpr Mat2(T a, T b, T c, T d):
		A(a),
		B(b),
		C(c),
		D(d)
	{
	}

	inline bool operator==(Mat2<T> other) const
	{
		return A == other.A && B == other.B && C == other.C && D == other.D;
	}

	inline bool operator!=(Mat2<T> other) const
	{
		return A != other.A || B != other.B || C != other.C || D != other.D;
	}

	template<typename S>
	inline Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(Vec2<S> vec) const
	{
		return Vec2<decltype(std::declval<T>() * std::declval<S>())>(A * vec.X + B * vec.Y, C * vec.X + D * vec.Y);
	}

	constexpr static Mat2<T> Identity = Mat2(1, 0, 0, 1);
	constexpr static Mat2<T> MirrorX = Mat2(-1, 0, 0, 1);
	constexpr static Mat2<T> MirrorY = Mat2(1, 0, 0, -1);
	constexpr static Mat2<T> CCW = Mat2(0, 1, -1, 0); // reminder: the Y axis points down
};

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
static inline Rect<T> RectBetween(Vec2<T>, Vec2<T>);

template<typename T, typename>
struct Rect
{
	// Inclusive
	Vec2<T> TopLeft, BottomRight;

private:
	constexpr Rect(Vec2<T> topLeft, Vec2<T> bottomRight):
		TopLeft(topLeft),
		BottomRight(bottomRight)
	{
	}
	friend Rect<T> RectBetween<T>(Vec2<T>, Vec2<T>);

public:
	inline Rect<T> operator|(Rect<T> other) const
	{
		return Rect<T>(
			Vec2<T>(std::min(TopLeft.X, other.TopLeft.X), std::min(TopLeft.Y, other.TopLeft.Y)),
			Vec2<T>(std::max(BottomRight.X, other.BottomRight.X), std::max(BottomRight.Y, other.BottomRight.Y))
		);
	}

	inline Rect<T> &operator|=(Rect<T> other)
	{
		return *this = *this | other;
	}

	inline bool Contains(Vec2<T> point) const
	{
		return point.X >= TopLeft.X && point.X <= BottomRight.X && point.Y >= TopLeft.Y && point.Y <= BottomRight.Y;
	}

	inline Vec2<T> Clamp(Vec2<T> point) const
	{
		return Vec2<T>(std::clamp(point.X, TopLeft.X, BottomRight.X), std::clamp(point.Y, TopLeft.Y, BottomRight.Y));
	}

	inline Rect<T> Clamp(Rect<T> other) const
	{
		return Rect<T>(Clamp(other.TopLeft), Clamp(other.BottomRight));
	}

	template<typename = std::enable_if<std::is_integral_v<T>, void>>
	inline Vec2<T> Size() const
	{
		return BottomRight - TopLeft + Vec2<T>(1, 1);
	}
};

template<typename T, typename>
static inline Rect<T> RectBetween(Vec2<T> topLeft, Vec2<T> bottomRight)
{
	return Rect<T>(topLeft, bottomRight);
}

template<typename T, typename = std::enable_if<std::is_arithmetic_v<T>, void>>
static inline Rect<T> RectAt(Vec2<T> pos)
{
	return RectBetween<T>(pos, pos);
}

template<typename T, typename = std::enable_if<std::is_integral_v<T>, void>>
static inline Rect<T> RectSized(Vec2<T> topLeft, Vec2<T> dimen)
{
	return RectBetween<T>(topLeft, topLeft + dimen - Vec2<T>(1, 1));
}
