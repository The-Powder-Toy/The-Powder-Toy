#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <type_traits>
#include <utility>

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Rect;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vec2
{
	T X, Y;

	constexpr Vec2(T x, T y):
		X(x),
		Y(y)
	{
	}

	template<typename S, typename = std::enable_if_t<std::is_constructible_v<T, S>>>
	constexpr explicit Vec2(Vec2<S> other):
		X(other.X),
		Y(other.Y)
	{
	}

	constexpr bool operator==(Vec2<T> other) const
	{
		return X == other.X && Y == other.Y;
	}

	constexpr bool operator!=(Vec2<T> other) const
	{
		return X != other.X || Y != other.Y;
	}

	template<typename S>
	constexpr Vec2<decltype(std::declval<T>() + std::declval<S>())> operator+(Vec2<S> other) const
	{
		return Vec2<decltype(std::declval<T>() + std::declval<S>())>(X + other.X, Y + other.Y);
	}

	constexpr Vec2<T> operator-() const
	{
		return Vec2<T>(-X, -Y);
	}

	template<typename S>
	constexpr Vec2<decltype(std::declval<T>() - std::declval<S>())> operator-(Vec2<S> other) const
	{
		return Vec2<decltype(std::declval<T>() - std::declval<S>())>(X - other.X, Y - other.Y);
	}

	template<typename S, typename = std::enable_if_t<std::is_arithmetic_v<S>>>
	constexpr Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(S other) const
	{
		return Vec2<decltype(std::declval<T>() * std::declval<S>())>(X * other, Y * other);
	}

	template<typename S, typename = std::enable_if_t<std::is_arithmetic_v<S>>>
	constexpr Vec2<decltype(std::declval<T>() / std::declval<S>())> operator/(S other) const
	{
		return Vec2<decltype(std::declval<T>() / std::declval<S>())>(X / other, Y / other);
	}

	template<typename S>
	constexpr Vec2<T> &operator+=(Vec2<S> other)
	{
		return *this = *this + other;
	}

	template<typename S>
	constexpr Vec2<T> &operator-=(Vec2<S> other)
	{
		return *this = *this - other;
	}

	template<typename S>
	constexpr Vec2<T> &operator*=(Vec2<S> other)
	{
		return *this = *this * other;
	}

	template<typename S>
	constexpr Vec2<T> &operator/=(Vec2<S> other)
	{
		return *this = *this / other;
	}

	// Round towards -infinity
	template<typename S = T, typename = std::enable_if_t<std::is_floating_point_v<S>>>
	Vec2<T> Floor() const
	{
		return Vec2<T>(std::floor(X), std::floor(Y));
	}

	// Round towards nearest integer, halfpoints towards -infinity
	template<typename S = T, typename = std::enable_if_t<std::is_floating_point_v<S>>>
	Vec2<T> Round() const
	{
		return (*this + Vec2<T>(0.5, 0.5)).Floor();
	}

	Vec2<T> Clamp(Rect<T> rect) const
	{
		return Vec2<T>(
			std::clamp(X, rect.TopLeft.X, rect.BottomRight.X),
			std::clamp(Y, rect.TopLeft.Y, rect.BottomRight.Y)
		);
	}

	// Return a rectangle starting at origin, whose dimensions match this vector
	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr inline Rect<T> OriginRect() const
	{
		return RectSized(Vec2<T>(0, 0), *this);
	}

	static Vec2<T> const Zero;
};

template<typename T, typename V>
Vec2<T> const Vec2<T, V>::Zero = Vec2<T>(0, 0);

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
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

	constexpr bool operator==(Mat2<T> other) const
	{
		return A == other.A && B == other.B && C == other.C && D == other.D;
	}

	constexpr bool operator!=(Mat2<T> other) const
	{
		return A != other.A || B != other.B || C != other.C || D != other.D;
	}

	template<typename S>
	constexpr Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(Vec2<S> vec) const
	{
		return Vec2<decltype(std::declval<T>() * std::declval<S>())>(A * vec.X + B * vec.Y, C * vec.X + D * vec.Y);
	}

	template<typename S>
	constexpr Mat2<decltype(std::declval<T>() * std::declval<S>())> operator*(Mat2<S> mat) const
	{
		return Mat2<decltype(std::declval<T>() * std::declval<S>())>(
			A * mat.A + B * mat.C, A * mat.B + B * mat.D,
			C * mat.A + D * mat.C, C * mat.B + D * mat.D
		);
	}

	static Mat2<T> const Identity, MirrorX, MirrorY, CCW;
};

template<typename T, typename V>
Mat2<T> const Mat2<T, V>::Identity = Mat2<T>(1, 0, 0, 1);
template<typename T, typename V>
Mat2<T> const Mat2<T, V>::MirrorX = Mat2<T>(-1, 0, 0, 1);
template<typename T, typename V>
Mat2<T> const Mat2<T, V>::MirrorY = Mat2<T>(1, 0, 0, -1);
template<typename T, typename V>
Mat2<T> const Mat2<T, V>::CCW = Mat2<T>(0, 1, -1, 0); // reminder: the Y axis points down

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr static inline Rect<T> RectBetween(Vec2<T>, Vec2<T>);

enum IterationDirection
{
	TOP_TO_BOTTOM,
	BOTTOM_TO_TOP,
	LEFT_TO_RIGHT,
	RIGHT_TO_LEFT,
};

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
	friend constexpr Rect<T> RectBetween<T>(Vec2<T>, Vec2<T>);

	struct end_sentinel
	{};

	template<IterationDirection D1, IterationDirection D2>
	struct range_row_major
	{
		static_assert(D1 == TOP_TO_BOTTOM || D1 == BOTTOM_TO_TOP);
		static_assert(D2 == LEFT_TO_RIGHT || D2 == RIGHT_TO_LEFT);
		T left, top, right, bottom;

		struct iterator
		{
			T x, y;
			T const first_x, last_x, end_y;

			iterator &operator++()
			{
				if (x == last_x)
				{
					x = first_x;
					if constexpr (D1 == TOP_TO_BOTTOM)
						y++;
					else
						y--;
				}
				else
				{
					if constexpr (D2 == LEFT_TO_RIGHT)
						x++;
					else
						x--;
				}
				return *this;
			}

			Vec2<T> operator*() const
			{
				return Vec2<T>(x, y);
			}

			bool operator!=(end_sentinel) const
			{
				if constexpr (D1 == TOP_TO_BOTTOM)
					return y < end_y;
				else
					return y > end_y;
			}

			using difference_type = void;
			using value_type = Vec2<T>;
			using pointer = void;
			using reference = void;
			using iterator_category = std::forward_iterator_tag;
		};

		iterator begin() const
		{
			T first_x = D2 == LEFT_TO_RIGHT ? left : right;
			T last_x = D2 == LEFT_TO_RIGHT ? right : left;
			T first_y = D1 == TOP_TO_BOTTOM ? top : bottom;
			T end_y = D1 == TOP_TO_BOTTOM ? bottom + 1 : top - 1;
			return iterator{first_x, right >= left ? first_y : end_y, first_x, last_x, end_y};
		}

		end_sentinel end() const
		{
			return end_sentinel();
		}
	};

	template<IterationDirection D1, IterationDirection D2>
	struct range_column_major
	{
		static_assert(D1 == LEFT_TO_RIGHT || D1 == RIGHT_TO_LEFT);
		static_assert(D2 == TOP_TO_BOTTOM || D2 == BOTTOM_TO_TOP);
		T left, top, right, bottom;

		struct iterator
		{
			T x, y;
			T const first_y, last_y, end_x;

			iterator &operator++()
			{
				if (y == last_y)
				{
					y = first_y;
					if constexpr (D1 == LEFT_TO_RIGHT)
						x++;
					else
						x--;
				}
				else
				{
					if constexpr (D2 == TOP_TO_BOTTOM)
						y++;
					else
						y--;
				}
				return *this;
			}

			Vec2<T> operator*() const
			{
				return Vec2<T>(x, y);
			}

			bool operator!=(end_sentinel) const
			{
				if constexpr (D1 == LEFT_TO_RIGHT)
					return x < end_x;
				else
					return x > end_x;
			}

			using difference_type = void;
			using value_type = Vec2<T>;
			using pointer = void;
			using reference = void;
			using iterator_category = std::forward_iterator_tag;
		};

		iterator begin() const
		{
			T first_y = D2 == TOP_TO_BOTTOM ? top : bottom;
			T last_y = D2 == TOP_TO_BOTTOM ? bottom : top;
			T first_x = D1 == LEFT_TO_RIGHT ? left : right;
			T end_x = D1 == LEFT_TO_RIGHT ? right + 1 : left - 1;
			return iterator{bottom >= top ? first_x : end_x, first_y, first_y, last_y, end_x};
		}

		end_sentinel end() const
		{
			return end_sentinel();
		}
	};

public:
	constexpr bool operator==(Rect other) const
	{
		return TopLeft == other.TopLeft && BottomRight == other.BottomRight;
	}

	constexpr bool operator!=(Rect other) const
	{
		return TopLeft != other.TopLeft || BottomRight != other.BottomRight;
	}

	constexpr explicit operator bool() const
	{
		return BottomRight.X >= TopLeft.X && BottomRight.Y >= TopLeft.Y;
	}

	// Return the smallest rectangle that contains both input rectangles,
	// **assuming neither are empty**
	Rect<T> operator|(Rect<T> other) const
	{
		return Rect<T>(
			Vec2<T>(std::min(TopLeft.X, other.TopLeft.X), std::min(TopLeft.Y, other.TopLeft.Y)),
			Vec2<T>(std::max(BottomRight.X, other.BottomRight.X), std::max(BottomRight.Y, other.BottomRight.Y))
		);
	}

	// Return the intersection of two rectangles (possibly empty)
	Rect<T> operator&(Rect<T> other) const
	{
		auto rect = Rect<T>(
			Vec2<T>(std::max(TopLeft.X, other.TopLeft.X), std::max(TopLeft.Y, other.TopLeft.Y)),
			Vec2<T>(std::min(BottomRight.X, other.BottomRight.X), std::min(BottomRight.Y, other.BottomRight.Y))
		);
		return Rect<T>(
			rect.TopLeft,
			Vec2<T>(
				std::max(rect.TopLeft.X - 1, rect.BottomRight.X),
				std::max(rect.TopLeft.Y - 1, rect.BottomRight.Y)
			)
		);
	}

	inline Rect<T> &operator|=(Rect<T> other)
	{
		return *this = *this | other;
	}

	inline Rect<T> &operator&=(Rect<T> other)
	{
		return *this = *this & other;
	}

	constexpr bool Contains(Vec2<T> point) const
	{
		return point.X >= TopLeft.X && point.X <= BottomRight.X && point.Y >= TopLeft.Y && point.Y <= BottomRight.Y;
	}

	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	inline Vec2<T> Size() const
	{
		return BottomRight - TopLeft + Vec2<T>(1, 1);
	}

	template<typename S>
	Rect<decltype(std::declval<T>() + std::declval<S>())> Inset(S delta) const
	{
		return Rect<decltype(std::declval<T>() + std::declval<S>())>(TopLeft + Vec2(delta, delta), BottomRight - Vec2(delta, delta));
	}

	template<IterationDirection D1, IterationDirection D2, typename S = T, typename = std::enable_if_t<std::is_integral_v<T>>>
	constexpr auto Range() const
	{
		static_assert(
			((D1 == TOP_TO_BOTTOM || D1 == BOTTOM_TO_TOP) && (D2 == LEFT_TO_RIGHT || D2 == RIGHT_TO_LEFT)) ||
			((D1 == LEFT_TO_RIGHT || D1 == RIGHT_TO_LEFT) && (D2 == TOP_TO_BOTTOM || D2 == BOTTOM_TO_TOP)),
			"Must include exactly 1 of TOP_TO_BOTTOM/BOTTOM_TO_TOP and exactly 1 of LEFT_TO_RIGHT/RIGHT_TO_LEFT"
		);
		if constexpr (D1 == TOP_TO_BOTTOM || D1 == BOTTOM_TO_TOP)
		{
			return range_row_major<D1, D2>{TopLeft.X, TopLeft.Y, BottomRight.X, BottomRight.Y};
		}
		else
			return range_column_major<D1, D2>{TopLeft.X, TopLeft.Y, BottomRight.X, BottomRight.Y};
	}

	// Use when the order isn't important
	constexpr typename range_row_major<TOP_TO_BOTTOM, LEFT_TO_RIGHT>::iterator begin() const
	{
		return Range<TOP_TO_BOTTOM, LEFT_TO_RIGHT>().begin();
	}

	constexpr end_sentinel end() const
	{
		return end_sentinel();
	}
};

template<typename T, typename>
constexpr inline Rect<T> RectBetween(Vec2<T> topLeft, Vec2<T> bottomRight)
{
	return Rect<T>(topLeft, bottomRight);
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr inline Rect<T> RectAt(Vec2<T> pos)
{
	return RectBetween<T>(pos, pos);
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr inline Rect<T> RectSized(Vec2<T> topLeft, Vec2<T> dimen)
{
	return RectBetween<T>(topLeft, topLeft + dimen - Vec2<T>(1, 1));
}
