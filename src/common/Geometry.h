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

	constexpr inline bool operator==(Vec2<T> other) const
	{
		return X == other.X && Y == other.Y;
	}

	constexpr inline bool operator!=(Vec2<T> other) const
	{
		return X != other.X || Y != other.Y;
	}

	template<typename S>
	constexpr inline Vec2<decltype(std::declval<T>() + std::declval<S>())> operator+(Vec2<S> other) const
	{
		return Vec2<decltype(std::declval<T>() + std::declval<S>())>(X + other.X, Y + other.Y);
	}

	constexpr inline Vec2<T> operator-() const
	{
		return Vec2<T>(-X, -Y);
	}

	template<typename S>
	constexpr inline Vec2<decltype(std::declval<T>() - std::declval<S>())> operator-(Vec2<S> other) const
	{
		return Vec2<decltype(std::declval<T>() - std::declval<S>())>(X - other.X, Y - other.Y);
	}

	template<typename S, typename = std::enable_if_t<std::is_arithmetic_v<S>>>
	constexpr inline Vec2<decltype(std::declval<T>() * std::declval<S>())> operator*(S other) const
	{
		return Vec2<decltype(std::declval<T>() * std::declval<S>())>(X * other, Y * other);
	}

	template<typename S, typename = std::enable_if_t<std::is_arithmetic_v<S>>>
	constexpr inline Vec2<decltype(std::declval<T>() / std::declval<S>())> operator/(S other) const
	{
		return Vec2<decltype(std::declval<T>() / std::declval<S>())>(X / other, Y / other);
	}

	template<typename S>
	constexpr inline Vec2<T> &operator+=(Vec2<S> other)
	{
		return *this = *this + other;
	}

	template<typename S>
	constexpr inline Vec2<T> &operator-=(Vec2<S> other)
	{
		return *this = *this - other;
	}

	template<typename S>
	constexpr inline Vec2<T> &operator*=(Vec2<S> other)
	{
		return *this = *this * other;
	}

	template<typename S>
	constexpr inline Vec2<T> &operator/=(Vec2<S> other)
	{
		return *this = *this / other;
	}

	constexpr inline Vec2<T> Floor() const
	{
		return Vec2<T>(std::floor(X), std::floor(Y));
	}

	// Return a rectangle starting at origin, whose dimensions match this vector
	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	constexpr inline Rect<T> OriginRect() const
	{
		return RectSized(Vec2<T>(0, 0), *this);
	}

	static Vec2<T> Zero;
};

template<typename T, typename V>
Vec2<T> Vec2<T, V>::Zero = Vec2<T>(0, 0);

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

	static Mat2<T> Identity, MirrorX, MirrorY, CCW;
};

template<typename T, typename V>
Mat2<T> Mat2<T, V>::Identity = Mat2<T>(1, 0, 0, 1);
template<typename T, typename V>
Mat2<T> Mat2<T, V>::MirrorX = Mat2<T>(-1, 0, 0, 1);
template<typename T, typename V>
Mat2<T> Mat2<T, V>::MirrorY = Mat2<T>(1, 0, 0, -1);
template<typename T, typename V>
Mat2<T> Mat2<T, V>::CCW = Mat2<T>(0, 1, -1, 0); // reminder: the Y axis points down

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr static inline Rect<T> RectBetween(Vec2<T>, Vec2<T>);

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

	struct iterator
	{
		int x, y, minX, maxX;

		iterator &operator++()
		{
			if (x == maxX)
			{
				x = minX;
				y++;
			}
			else
			{
				x++;
			}
			return *this;
		}

		Vec2<T> operator*() const
		{
			return Vec2<T>(x, y);
		}

		bool operator!=(iterator other) const
		{
			return x != other.x || y != other.y || minX != other.minX || maxX != other.maxX;
		}

		using difference_type = void;
		using value_type = Vec2<T>;
		using pointer = void;
		using reference = void;
		using iterator_category = std::forward_iterator_tag;
	};

public:
	inline operator bool() const
	{
		return BottomRight.X >= TopLeft.X || BottomRight.Y >= TopLeft.Y;
	}

	// Return the smallest rectangle that contains both input rectangles,
	// **assuming neither are empty**
	inline Rect<T> operator|(Rect<T> other) const
	{
		return Rect<T>(
			Vec2<T>(std::min(TopLeft.X, other.TopLeft.X), std::min(TopLeft.Y, other.TopLeft.Y)),
			Vec2<T>(std::max(BottomRight.X, other.BottomRight.X), std::max(BottomRight.Y, other.BottomRight.Y))
		);
	}

	// Return the intersection of two rectangles (possibly empty)
	inline Rect<T> operator&(Rect<T> other) const
	{
		return Rect<T>(
			Vec2<T>(std::max(TopLeft.X, other.TopLeft.X), std::max(TopLeft.Y, other.TopLeft.Y)),
			Vec2<T>(std::min(BottomRight.X, other.BottomRight.X), std::min(BottomRight.Y, other.BottomRight.Y))
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

	template<typename S = T, typename = std::enable_if_t<std::is_integral_v<S>>>
	inline Vec2<T> Size() const
	{
		return BottomRight - TopLeft + Vec2<T>(1, 1);
	}

	iterator begin() const
	{
		return iterator{TopLeft.X, TopLeft.Y, TopLeft.X, BottomRight.X};
	}

	iterator end() const
	{
		return iterator{TopLeft.X, BottomRight.Y + 1, TopLeft.X, BottomRight.X};
	}
};

template<typename T, typename>
constexpr static inline Rect<T> RectBetween(Vec2<T> topLeft, Vec2<T> bottomRight)
{
	return Rect<T>(topLeft, bottomRight);
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
static inline Rect<T> RectAt(Vec2<T> pos)
{
	return RectBetween<T>(pos, pos);
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr static inline Rect<T> RectSized(Vec2<T> topLeft, Vec2<T> dimen)
{
	return RectBetween<T>(topLeft, topLeft + dimen - Vec2<T>(1, 1));
}

template<typename T>
class PlaneAdapter
{
	int width;
public:
	T Base;

	// ideally, value_type = std::indirectly_readable_traits<T>::value_type
	using value_type = std::remove_reference_t<decltype(std::declval<T>()[0])>;
	using iterator = decltype(std::begin(std::declval<T &>()));
	using const_iterator = decltype(std::begin(std::declval<T const &>()));

	PlaneAdapter() = default;

	PlaneAdapter(int width, T &&Base):
		width(width),
		Base(std::move(Base))
	{
	}

	PlaneAdapter(Vec2<int> size, value_type defaultVal):
		width(size.X),
		Base(size.X * size.Y, defaultVal)
	{
	}

	iterator begin()
	{
		return std::begin(Base);
	}

	const_iterator begin() const
	{
		return std::begin(Base);
	}

	iterator end()
	{
		return std::end(Base);
	}

	const_iterator end() const
	{
		return std::end(Base);
	}

	iterator At(Vec2<int> p)
	{
		return std::begin(Base) + (p.X + p.Y * width);
	}

	const_iterator At(Vec2<int> p) const
	{
		return std::begin(Base) + (p.X + p.Y * width);
	}

	value_type *data()
	{
		return std::data(Base);
	}

	value_type const *data() const
	{
		return std::data(Base);
	}

	value_type &operator[](Vec2<int> p)
	{
		return Base[p.X + p.Y * width];
	}

	value_type const &operator[](Vec2<int> p) const
	{
		return Base[p.X + p.Y * width];
	}

	void SetWidth(int width)
	{
		this->width = width;
	}
};

template<int width, typename T>
class StaticPlaneAdapter
{
public:
	T Base;

	using value_type = std::remove_reference_t<decltype(std::declval<T>()[0])>;
	using iterator = decltype(std::begin(std::declval<T &>()));
	using const_iterator = decltype(std::begin(std::declval<T const &>()));

	StaticPlaneAdapter() = default;

	StaticPlaneAdapter(T &&Base):
		Base(std::move(Base))
	{
	}

	StaticPlaneAdapter(int height, value_type defaultVal):
		Base(width * height, defaultVal)
	{
	}

	iterator begin()
	{
		return std::begin(Base);
	}

	const_iterator begin() const
	{
		return std::begin(Base);
	}

	iterator end()
	{
		return std::end(Base);
	}

	const_iterator end() const
	{
		return std::end(Base);
	}

	iterator At(Vec2<int> p)
	{
		return std::begin(Base) + (p.X + p.Y * width);
	}

	const_iterator At(Vec2<int> p) const
	{
		return std::begin(Base) + (p.X + p.Y * width);
	}

	value_type *data()
	{
		return std::data(Base);
	}

	value_type const *data() const
	{
		return std::data(Base);
	}

	value_type &operator[](Vec2<int> p)
	{
		return Base[p.X + p.Y * width];
	};

	value_type const &operator[](Vec2<int> p) const
	{
		return Base[p.X + p.Y * width];
	};
};
