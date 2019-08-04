
#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <llvm/Support/LLVMBuilder.h>
#include <vector>

class Float
{
protected:
	llvm::Value *_value;
	bool _isPointer;
	mutable bool _accessed;

public:
	Float() : _value(llvm::ConstantFP::get(llvm::Type::FloatTy, APFloat(0.0f))), _isPointer(false), _accessed(false) { }
	Float(float v) : _value(llvm::ConstantFP::get(llvm::Type::FloatTy, APFloat(v))), _isPointer(false), _accessed(false) { }
	Float(Value *value, bool isPointer) : _value(value), _isPointer(isPointer), _accessed(false) { }
	Float(const Float &other) : _value(other._value), _isPointer(other._isPointer), _accessed(false) { }
	Float &operator=(const Float &other) { if(this != &other) { _value = other._value; _isPointer = other._isPointer; _accessed = false; } return *this; }
	virtual ~Float() { }
	
	inline bool accessed() const { return _accessed; }
	inline Value *value() const { _accessed = true; return _isPointer ? Builder()->CreateLoad(_value) : _value; }

	virtual inline Value *address(bool keep = true)
	{
		if(!_isPointer)
		{
			Value *ptr = Builder()->CreateAlloca(llvm::Type::FloatTy);
			if(keep) Builder()->CreateStore(_value, ptr);
			_value = ptr; _isPointer = true;
		}
		return _value;
	}

	Float operator +() const { return *this; }
	Float operator -() const { return Float(Builder()->CreateNeg(value()), false); }
	Float &operator++() { Builder()->CreateStore(Builder()->CreateAdd(value(), llvm::ConstantFP::get(llvm::Type::FloatTy, APFloat(1.0f))), address()); return *this; }
	Float &operator--() { Builder()->CreateStore(Builder()->CreateSub(value(), llvm::ConstantFP::get(llvm::Type::FloatTy, APFloat(1.0f))), address()); return *this; }
	Float operator++(int) { Float v(*this); Builder()->CreateStore(Builder()->CreateAdd(value(), llvm::ConstantFP::get(llvm::Type::FloatTy, APFloat(1.0f))), address()); return v; }
	Float operator--(int) { Float v(*this); Builder()->CreateStore(Builder()->CreateSub(value(), llvm::ConstantFP::get(llvm::Type::FloatTy, APFloat(1.0f))), address()); return v; }
	Float operator +(const Float &other) const { return Float(Builder()->CreateAdd(value(), other.value()), false); }
	Float operator -(const Float &other) const { return Float(Builder()->CreateSub(value(), other.value()), false); }
	Float operator *(const Float &other) const { return Float(Builder()->CreateMul(value(), other.value()), false); }
	Float operator /(const Float &other) const { return Float(Builder()->CreateFDiv(value(), other.value()), false); }
	Float operator %(const Float &other) const { return Float(Builder()->CreateFRem(value(), other.value()), false); }
};

inline Float min(const Float &f1, const Float &f2) { return Float(Builder()->CreateSelect(Builder()->CreateFCmpOLT(f1.value(), f2.value()), f1.value(), f2.value()), false); }
inline Float max(const Float &f1, const Float &f2) { return Float(Builder()->CreateSelect(Builder()->CreateFCmpOGT(f1.value(), f2.value()), f1.value(), f2.value()), false); }
inline Float clamp(const Float &f, const Float &lower, const Float &upper) { return min(upper, max(lower, f)); }
inline Float saturate(const Float &f) { return clamp(f, Float(0.0f), Float(1.0f)); }
inline Float lerp(const Float &f1, const Float &f2, const Float &f) { return f1 + (f2 - f1) * f; }

/*inline float sign(const Float &f) { return (f == 0.0f) ? 0.0f : ((f > 0.0f) ? 1.0f : -1.0f); }*/

// Vectors --------------------------------------------------------------------

// 2-dimensional vector -------------------------------------------------------

class Float2
{
	class Field : public Float
	{
		Float2 *_vector;
		int _idx;

	public:
		Field(Float2 *vector, int idx) : _vector(vector), _idx(idx) { }
		Field(Float2 *vector, int idx, const Float &f) : Float(f.value(), false), _vector(vector), _idx(idx) { }
		Field(Float2 *vector, int idx, Value *ptr) : Float(ptr, true), _vector(vector), _idx(idx) { }

		Float &operator=(const Field &other)
		{
			if(this != &other)
			{
				if(_isPointer) Builder()->CreateStore(other.value(), _value);
				else _value = other.value(); 
			}
			return *this;
		}

		Float &operator=(const Float &other)
		{
			if(this != &other)
			{
				if(_isPointer) Builder()->CreateStore(other.value(), _value);
				else _value = other.value(); 
			}
			return *this;
		}

		inline Value *address()
		{
			if(!_isPointer)
			{
				_value = Builder()->CreateStructGEP(_vector->address(), _idx, (_idx == 0) ? "x" : "y");
				_isPointer = true;
			}
			return _value;
		}
	};

	Value *_ptr;

public:
	Field x, y;

	inline Float2() : _ptr(0), x(this, 0), y(this, 1) {}
	inline Float2(const Float2 &v) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y) {}
	inline Float2(const Float &f) : _ptr(0), x(this, 0, f), y(this, 1, f) {}
	inline Float2(const Float &x, const Float &y) : _ptr(0), x(this, 0, x), y(this, 1, y) {}
	Float2(const class Float3 &v);
	Float2(const class Float4 &v);

	inline Float2(Value *ptr) : _ptr(ptr),
		x(this, 0, Builder()->CreateStructGEP(_ptr, 0, "x")), y(this, 1, Builder()->CreateStructGEP(_ptr, 1, "y"))
	{ }

	inline bool accessed() const { return x.accessed() | y.accessed(); }

	inline Value *address(bool keep = true)
	{
		if(_ptr == 0)
		{
			_ptr = Builder()->CreateAlloca(StructType::get(std::vector<const Type *>(2, llvm::Type::FloatTy), true));
			Value *xvalue = x.value(), *yvalue = y.value();
			if(keep)
			{
				Builder()->CreateStore(xvalue, x.address());
				Builder()->CreateStore(yvalue, y.address());
			}
			else
				{ x.address(); y.address(); }
		}
		return _ptr;
	}

	inline Float2 operator +() const { return *this; }
	inline Float2 operator -() const { return Float2(-x, -y); }
};

inline Float2 operator +(const Float2 &v1, const Float2 &v2) { return Float2(v1.x + v2.x, v1.y + v2.y); }
inline Float2 operator -(const Float2 &v1, const Float2 &v2) { return Float2(v1.x - v2.x, v1.y - v2.y); }
inline Float2 operator *(const Float2 &v1, const Float2 &v2) { return Float2(v1.x * v2.x, v1.y * v2.y); }
inline Float2 operator /(const Float2 &v1, const Float2 &v2) { return Float2(v1.x / v2.x, v1.y / v2.y); }

inline Float2 operator +(const Float2 &v, const Float &f) { return Float2(v.x + f, v.y + f); }
inline Float2 operator +(const Float &f, const Float2 &v) { return Float2(v.x + f, v.y + f); }
inline Float2 operator -(const Float2 &v, const Float &f) { return Float2(v.x - f, v.y - f); }
inline Float2 operator -(const Float &f, const Float2 &v) { return Float2(f - v.x, f - v.y); }
inline Float2 operator *(const Float2 &v, const Float &f) { return Float2(v.x * f, v.y * f); }
inline Float2 operator *(const Float &f, const Float2 &v) { return Float2(v.x * f, v.y * f); }
inline Float2 operator /(const Float2 &v, const Float &f) { Float inv(Float(1.0f) / f); return Float2(v.x * inv, v.y * inv); }
inline Float2 operator /(const Float &f, const Float2 &v) { return Float2(f / v.x, f / v.y); }

inline Float dot(const Float2 &v1, const Float2 &v2) { return v1.x * v2.x + v1.y * v2.y; }
inline Float2 saturate(const Float2 &v) { return Float2(saturate(v.x), saturate(v.y)); }

/*inline float lengthsq(const Float2 &v) { return dot(v, v); }
inline float length(const Float2 &v) { return sqrtf(lengthsq(v)); }
inline Float2 lerp(const Float2 &v1, const Float2 &v2, const Float &f) { return v1 + (v2 - v1) * f; }
inline Float2 normalize(const Float2 &v) { return v * (1.0f / length(v)); }
inline Float2 min(const Float2 &v1, const Float2 &v2) { return Float2(min(v1.x, v2.x), min(v1.y, v2.y)); }
inline Float2 max(const Float2 &v1, const Float2 &v2) { return Float2(max(v1.x, v2.x), max(v1.y, v2.y)); }
inline Float2 abs(const Float2 &v) { return Float2(fabs(v.x), fabs(v.y)); }*/

// 3-dimensional vector -------------------------------------------------------

class Float3
{
	class Field : public Float
	{
		Float3 *_vector;
		int _idx;

	public:
		Field(Float3 *vector, int idx) : _vector(vector), _idx(idx) { }
		Field(Float3 *vector, int idx, const Float &f) : Float(f.value(), false), _vector(vector), _idx(idx) { }
		Field(Float3 *vector, int idx, Value *ptr) : Float(ptr, true), _vector(vector), _idx(idx) { }

		Float &operator=(const Field &other)
		{
			if(this != &other)
			{
				if(_isPointer) Builder()->CreateStore(other.value(), _value);
				else _value = other.value(); 
			}
			return *this;
		}

		Float &operator=(const Float &other)
		{
			if(this != &other)
			{
				if(_isPointer) Builder()->CreateStore(other.value(), _value);
				else _value = other.value(); 
			}
			return *this;
		}

		inline Value *address()
		{
			if(!_isPointer)
			{
				_value = Builder()->CreateStructGEP(_vector->address(), _idx, (_idx == 0) ? "x" : ((_idx == 1) ? "y" : "z"));
				_isPointer = true;
			}
			return _value;
		}
	};

	Value *_ptr;

public:
	Field x, y, z;

	inline Float3() : _ptr(0), x(this, 0), y(this, 1), z(this, 2) {}
	inline Float3(const Float3 &v) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y), z(this, 2, v.z) {}
	inline Float3(const Float &f) : _ptr(0), x(this, 0, f), y(this, 1, f), z(this, 2, f) {}
	inline Float3(const Float &x, const Float &y, const Float &z) : _ptr(0), x(this, 0, x), y(this, 1, y), z(this, 2, z) {}
	inline Float3(const Float2 &v, const Float &z) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y), z(this, 2, z) {}
	Float3(const class Float4 &v);

	inline Float3(Value *ptr) : _ptr(ptr),
		x(this, 0, Builder()->CreateStructGEP(_ptr, 0, "x")), y(this, 1, Builder()->CreateStructGEP(_ptr, 1, "y")),
		z(this, 2, Builder()->CreateStructGEP(_ptr, 2, "z"))
	{ }

	inline bool accessed() const { return x.accessed() | y.accessed() | z.accessed(); }

	inline Value *address(bool keep = true)
	{
		if(_ptr == 0)
		{
			_ptr = Builder()->CreateAlloca(StructType::get(std::vector<const Type *>(3, llvm::Type::FloatTy), true));
			Value *xvalue = x.value(), *yvalue = y.value(), *zvalue = z.value();
			if(keep)
			{
				Builder()->CreateStore(xvalue, x.address());
				Builder()->CreateStore(yvalue, y.address());
				Builder()->CreateStore(zvalue, z.address());
			}
			else
				{ x.address(); y.address(); z.address(); }
		}
		return _ptr;
	}

	inline Float3 operator +() const { return *this; }
	inline Float3 operator -() const { return Float3(-x, -y, -z); }
};

inline Float3 operator +(const Float3 &v1, const Float3 &v2) { return Float3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
inline Float3 operator -(const Float3 &v1, const Float3 &v2) { return Float3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
inline Float3 operator *(const Float3 &v1, const Float3 &v2) { return Float3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z); }
inline Float3 operator /(const Float3 &v1, const Float3 &v2) { return Float3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z); }

inline Float3 operator +(const Float3 &v, const Float &f) { return Float3(v.x + f, v.y + f, v.z + f); }
inline Float3 operator +(const Float &f, const Float3 &v) { return Float3(v.x + f, v.y + f, v.z + f); }
inline Float3 operator -(const Float3 &v, const Float &f) { return Float3(v.x - f, v.y - f, v.z - f); }
inline Float3 operator -(const Float &f, const Float3 &v) { return Float3(f - v.x, f - v.y, f - v.z); }
inline Float3 operator *(const Float3 &v, const Float &f) { return Float3(v.x * f, v.y * f, v.z * f); }
inline Float3 operator *(const Float &f, const Float3 &v) { return Float3(v.x * f, v.y * f, v.z * f); }
inline Float3 operator /(const Float3 &v, const Float &f) { Float inv(Float(1.0f) / f); return Float3(v.x * inv, v.y * inv, v.z * inv); }
inline Float3 operator /(const Float &f, const Float3 &v) { return Float3(f / v.x, f / v.y, f / v.z); }

inline Float dot(const Float3 &v1, const Float3 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

/*inline Float3 cross(const Float3 &v1, const Float3 &v2) { return Float3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x); }
inline float lengthsq(const Float3 &v) { return dot(v, v); }
inline float length(const Float3 &v) { return sqrtf(lengthsq(v)); }
inline Float3 lerp(const Float3 &v1, const Float3 &v2, const Float &f) { return v1 + (v2 - v1) * f; }
inline Float3 normalize(const Float3 &v) { return v * (1.0f / length(v)); }
inline Float3 min(const Float3 &v1, const Float3 &v2) { return Float3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z)); }
inline Float3 max(const Float3 &v1, const Float3 &v2) { return Float3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z)); }
inline Float3 abs(const Float3 &v) { return Float3(fabs(v.x), fabs(v.y), fabs(v.z)); }*/

// 4-dimensional vector -------------------------------------------------------

class Float4
{
	class Field : public Float
	{
		Float4 *_vector;
		int _idx;

	public:
		Field(Float4 *vector, int idx) : _vector(vector), _idx(idx) { }
		Field(Float4 *vector, int idx, const Float &f) : Float(f.value(), false), _vector(vector), _idx(idx) { }
		Field(Float4 *vector, int idx, Value *ptr) : Float(ptr, true), _vector(vector), _idx(idx) { }

		Float &operator=(const Field &other)
		{
			if(this != &other)
			{
				if(_isPointer) Builder()->CreateStore(other.value(), _value);
				else _value = other.value(); 
			}
			return *this;
		}

		Float &operator=(const Float &other)
		{
			if(this != &other)
			{
				if(_isPointer) Builder()->CreateStore(other.value(), _value);
				else _value = other.value(); 
			}
			return *this;
		}

		inline Value *address()
		{
			if(!_isPointer)
			{
				_value = Builder()->CreateStructGEP(_vector->address(), _idx, (_idx == 0) ? "x" : ((_idx == 1) ? "y" : ((_idx == 2) ? "z" : "w")));
				_isPointer = true;
			}
			return _value;
		}
	};

	Value *_ptr;

public:
	Field x, y, z, w;

	inline Float4() : _ptr(0), x(this, 0), y(this, 1), z(this, 2), w(this, 3) {}
	inline Float4(const Float4 &v) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y), z(this, 2, v.z), w(this, 3, v.w) {}
	inline Float4(const Float &f) : _ptr(0), x(this, 0, f), y(this, 1, f), z(this, 2, f), w(this, 3, f) {}
	inline Float4(const Float &x, const Float &y, const Float &z, const Float &w) : _ptr(0), x(this, 0, x), y(this, 1, y), z(this, 2, z), w(this, 3, w) {}
	inline Float4(const Float2 &v, const Float &z, const Float &w) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y), z(this, 2, z), w(this, 3, w) {}
	inline Float4(const Float3 &v, const Float &w) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y), z(this, 2, v.z), w(this, 3, w) {}

	inline Float4(Value *ptr) : _ptr(ptr),
		x(this, 0, Builder()->CreateStructGEP(_ptr, 0, "x")), y(this, 1, Builder()->CreateStructGEP(_ptr, 1, "y")),
		z(this, 2, Builder()->CreateStructGEP(_ptr, 2, "z")), w(this, 3, Builder()->CreateStructGEP(_ptr, 3, "w"))
	{ }

	inline bool accessed() const { return x.accessed() | y.accessed() | z.accessed() | w.accessed(); }

	inline Value *address(bool keep = true)
	{
		if(_ptr == 0)
		{
			_ptr = Builder()->CreateAlloca(StructType::get(std::vector<const Type *>(4, llvm::Type::FloatTy), true));
			Value *xvalue = x.value(), *yvalue = y.value(), *zvalue = z.value(), *wvalue = w.value();
			if(keep)
			{
				Builder()->CreateStore(xvalue, x.address());
				Builder()->CreateStore(yvalue, y.address());
				Builder()->CreateStore(zvalue, z.address());
				Builder()->CreateStore(wvalue, w.address());
			}
			else
				{ x.address(); y.address(); z.address(); w.address(); }
		}
		return _ptr;
	}

	inline Float4 operator +() const { return *this; }
	inline Float4 operator -() const { return Float4(-x, -y, -z, -w); }
};

inline Float4 operator +(const Float4 &v1, const Float4 &v2) { return Float4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w); }
inline Float4 operator -(const Float4 &v1, const Float4 &v2) { return Float4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w); }
inline Float4 operator *(const Float4 &v1, const Float4 &v2) { return Float4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w); }
inline Float4 operator /(const Float4 &v1, const Float4 &v2) { return Float4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w); }

inline Float4 operator +(const Float4 &v, const Float &f) { return Float4(v.x + f, v.y + f, v.z + f, v.w + f); }
inline Float4 operator +(const Float &f, const Float4 &v) { return Float4(v.x + f, v.y + f, v.z + f, v.w + f); }
inline Float4 operator -(const Float4 &v, const Float &f) { return Float4(v.x - f, v.y - f, v.z - f, v.w - f); }
inline Float4 operator -(const Float &f, const Float4 &v) { return Float4(f - v.x, f - v.y, f - v.z, f - v.w); }
inline Float4 operator *(const Float4 &v, const Float &f) { return Float4(v.x * f, v.y * f, v.z * f, v.w * f); }
inline Float4 operator *(const Float &f, const Float4 &v) { return Float4(v.x * f, v.y * f, v.z * f, v.w * f); }
inline Float4 operator /(const Float4 &v, const Float &f) { Float inv(Float(1.0f) / f); return Float4(v.x * inv, v.y * inv, v.z * inv, v.w * inv); }
inline Float4 operator /(const Float &f, const Float4 &v) { return Float4(f / v.x, f / v.y, f / v.z, f / v.w); }

inline Float4 lerp(const Float4 &v1, const Float4 &v2, const Float &f) { return v1 + (v2 - v1) * f; }

/*inline float dot(const Float4 &v1, const Float4 &v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w; }
inline float lengthsq(const Float4 &v) { return dot(v, v); }
inline float length(const Float4 &v) { return sqrtf(lengthsq(v)); }
inline Float4 lerp(const Float4 &v1, const Float4 &v2, const Float &f) { return v1 + (v2 - v1) * f; }
inline Float4 normalize(const Float4 &v) { return v * (1.0f / length(v)); }
inline Float4 min(const Float4 &v1, const Float4 &v2) { return Float4(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z), min(v1.w, v2.w)); }
inline Float4 max(const Float4 &v1, const Float4 &v2) { return Float4(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z), max(v1.w, v2.w)); }
inline Float4 abs(const Float4 &v) { return Float4(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w)); }*/

// ----------------------------------------------------------------------------

inline Float2::Float2(const class Float3 &v) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y) { }
inline Float2::Float2(const class Float4 &v) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y) { }

inline Float3::Float3(const class Float4 &v) : _ptr(0), x(this, 0, v.x), y(this, 1, v.y), z(this, 2, v.z) { }

#endif // PRIMITIVES_H

