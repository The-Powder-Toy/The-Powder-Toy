/******************************************************************************

Copyright (c) 2009-2010, Terry Caton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright 
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the projecct nor the names of its contributors 
      may be used to endorse or promote products derived from this software 
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include "visitor.h"
#include "reader.h"
#include <cassert>
#include <algorithm>
#include <map>

/*  

TODO:
* better documentation

*/

namespace json
{


inline Exception::Exception(const std::string& sMessage) :
   std::runtime_error(sMessage) {}


/////////////////////////
// UnknownElement members

class UnknownElement::Imp
{
public:
   virtual ~Imp() {}
   virtual Imp* Clone() const = 0;

   virtual bool Compare(const Imp& imp) const = 0;

   virtual void Accept(ConstVisitor& visitor) const = 0;
   virtual void Accept(Visitor& visitor) = 0;
};


template <typename ElementTypeT>
class UnknownElement::Imp_T : public UnknownElement::Imp
{
public:
   Imp_T(const ElementTypeT& element) : m_Element(element) {}
   virtual Imp* Clone() const { return new Imp_T<ElementTypeT>(*this); }

   virtual void Accept(ConstVisitor& visitor) const { visitor.Visit(m_Element); }
   virtual void Accept(Visitor& visitor) { visitor.Visit(m_Element); }

   virtual bool Compare(const Imp& imp) const
   {
      ConstCastVisitor_T<ElementTypeT> castVisitor;
      imp.Accept(castVisitor);
      return castVisitor.m_pElement &&
             m_Element == *castVisitor.m_pElement;
   }

private:
   ElementTypeT m_Element;
};


class UnknownElement::ConstCastVisitor : public ConstVisitor
{
   virtual void Visit(const Array& array) {}
   virtual void Visit(const Object& object) {}
   virtual void Visit(const Number& number) {}
   virtual void Visit(const String& string) {}
   virtual void Visit(const Boolean& boolean) {}
   virtual void Visit(const Null& null) {}
};

template <typename ElementTypeT>
class UnknownElement::ConstCastVisitor_T : public ConstCastVisitor
{
public:
   ConstCastVisitor_T() : m_pElement(0) {}
   virtual void Visit(const ElementTypeT& element) { m_pElement = &element; } // we don't know what this is, but it overrides one of the base's no-op functions
   const ElementTypeT* m_pElement;
};


class UnknownElement::CastVisitor : public Visitor
{
   virtual void Visit(Array& array) {}
   virtual void Visit(Object& object) {}
   virtual void Visit(Number& number) {}
   virtual void Visit(String& string) {}
   virtual void Visit(Boolean& boolean) {}
   virtual void Visit(Null& null) {}
};

template <typename ElementTypeT>
class UnknownElement::CastVisitor_T : public CastVisitor
{
public:
   CastVisitor_T() : m_pElement(0) {}
   virtual void Visit(ElementTypeT& element) { m_pElement = &element; } // we don't know what this is, but it overrides one of the base's no-op functions
   ElementTypeT* m_pElement;
};




inline UnknownElement::UnknownElement() :                               m_pImp( new Imp_T<Null>( Null() ) ) {}
inline UnknownElement::UnknownElement(const UnknownElement& unknown) :  m_pImp( unknown.m_pImp->Clone()) {}
inline UnknownElement::UnknownElement(const Object& object) :           m_pImp( new Imp_T<Object>(object) ) {}
inline UnknownElement::UnknownElement(const Array& array) :             m_pImp( new Imp_T<Array>(array) ) {}
inline UnknownElement::UnknownElement(const Number& number) :           m_pImp( new Imp_T<Number>(number) ) {}
inline UnknownElement::UnknownElement(const Boolean& boolean) :         m_pImp( new Imp_T<Boolean>(boolean) ) {}
inline UnknownElement::UnknownElement(const String& string) :           m_pImp( new Imp_T<String>(string) ) {}
inline UnknownElement::UnknownElement(const Null& null) :               m_pImp( new Imp_T<Null>(null) ) {}

inline UnknownElement::~UnknownElement()   { delete m_pImp; }

inline UnknownElement::operator const Object& () const    { return CastTo<Object>(); }
inline UnknownElement::operator const Array& () const     { return CastTo<Array>(); }
inline UnknownElement::operator const Number& () const    { return CastTo<Number>(); }
inline UnknownElement::operator const Boolean& () const   { return CastTo<Boolean>(); }
inline UnknownElement::operator const String& () const    { return CastTo<String>(); }
inline UnknownElement::operator const Null& () const      { return CastTo<Null>(); }

inline UnknownElement::operator Object& ()    { return ConvertTo<Object>(); }
inline UnknownElement::operator Array& ()     { return ConvertTo<Array>(); }
inline UnknownElement::operator Number& ()    { return ConvertTo<Number>(); }
inline UnknownElement::operator Boolean& ()   { return ConvertTo<Boolean>(); }
inline UnknownElement::operator String& ()    { return ConvertTo<String>(); }
inline UnknownElement::operator Null& ()      { return ConvertTo<Null>(); }

inline UnknownElement& UnknownElement::operator = (const UnknownElement& unknown) 
{
   // always check for this
   if (&unknown != this)
   {
      // we might be copying from a subtree of ourselves. delete the old imp
      //  only after the clone operation is complete. yes, this could be made 
      //  more efficient, but isn't worth the complexity
      Imp* pOldImp = m_pImp;
      m_pImp = unknown.m_pImp->Clone();
      delete pOldImp;
   }

   return *this;
}

inline UnknownElement& UnknownElement::operator[] (const std::string& key)
{
   // the people want an object. make us one if we aren't already
   Object& object = ConvertTo<Object>();
   return object[key];
}

inline const UnknownElement& UnknownElement::operator[] (const std::string& key) const
{
   // throws if we aren't an object
   const Object& object = CastTo<Object>();
   return object[key];
}

inline UnknownElement& UnknownElement::operator[] (size_t index)
{
   // the people want an array. make us one if we aren't already
   Array& array = ConvertTo<Array>();
   return array[index];
}

inline const UnknownElement& UnknownElement::operator[] (size_t index) const
{
   // throws if we aren't an array
   const Array& array = CastTo<Array>();
   return array[index];
}


template <typename ElementTypeT>
const ElementTypeT& UnknownElement::CastTo() const
{
   ConstCastVisitor_T<ElementTypeT> castVisitor;
   m_pImp->Accept(castVisitor);
   if (castVisitor.m_pElement == 0)
      throw Exception("Bad cast");
   return *castVisitor.m_pElement;
}



template <typename ElementTypeT>
ElementTypeT& UnknownElement::ConvertTo() 
{
   CastVisitor_T<ElementTypeT> castVisitor;
   m_pImp->Accept(castVisitor);
   if (castVisitor.m_pElement == 0)
   {
      // we're not the right type. fix it & try again
      *this = ElementTypeT();
      m_pImp->Accept(castVisitor);
   }

   return *castVisitor.m_pElement;
}


inline void UnknownElement::Accept(ConstVisitor& visitor) const { m_pImp->Accept(visitor); }
inline void UnknownElement::Accept(Visitor& visitor)            { m_pImp->Accept(visitor); }


inline bool UnknownElement::operator == (const UnknownElement& element) const
{
   return m_pImp->Compare(*element.m_pImp);
}



//////////////////
// Object members


inline Object::Member::Member(const std::string& nameIn, const UnknownElement& elementIn) :
   name(nameIn), element(elementIn) {}

inline bool Object::Member::operator == (const Member& member) const 
{
   return name == member.name &&
          element == member.element;
}

class Object::Finder : public std::unary_function<Object::Member, bool>
{
public:
   Finder(const std::string& name) : m_name(name) {}
   bool operator () (const Object::Member& member) {
      return member.name == m_name;
   }

private:
   std::string m_name;
};



inline Object::iterator Object::Begin() { return m_Members.begin(); }
inline Object::iterator Object::End() { return m_Members.end(); }
inline Object::const_iterator Object::Begin() const { return m_Members.begin(); }
inline Object::const_iterator Object::End() const { return m_Members.end(); }

inline size_t Object::Size() const { return m_Members.size(); }
inline bool Object::Empty() const { return m_Members.empty(); }

inline Object::iterator Object::Find(const std::string& name) 
{
   return std::find_if(m_Members.begin(), m_Members.end(), Finder(name));
}

inline Object::const_iterator Object::Find(const std::string& name) const 
{
   return std::find_if(m_Members.begin(), m_Members.end(), Finder(name));
}

inline Object::iterator Object::Insert(const Member& member)
{
   return Insert(member, End());
}

inline Object::iterator Object::Insert(const Member& member, iterator itWhere)
{
   iterator it = Find(member.name);
   if (it != m_Members.end())
      throw Exception(std::string("Object member already exists: ") + member.name);

   it = m_Members.insert(itWhere, member);
   return it;
}

inline Object::iterator Object::Erase(iterator itWhere) 
{
   return m_Members.erase(itWhere);
}

inline UnknownElement& Object::operator [](const std::string& name)
{

   iterator it = Find(name);
   if (it == m_Members.end())
   {
      Member member(name);
      it = Insert(member, End());
   }
   return it->element;      
}

inline const UnknownElement& Object::operator [](const std::string& name) const 
{
   const_iterator it = Find(name);
   if (it == End())
      throw Exception(std::string("Object member not found: ") + name);
   return it->element;
}

inline void Object::Clear() 
{
   m_Members.clear(); 
}

inline bool Object::operator == (const Object& object) const 
{
   return m_Members == object.m_Members;
}


/////////////////
// Array members

inline Array::iterator Array::Begin()  { return m_Elements.begin(); }
inline Array::iterator Array::End()    { return m_Elements.end(); }
inline Array::const_iterator Array::Begin() const  { return m_Elements.begin(); }
inline Array::const_iterator Array::End() const    { return m_Elements.end(); }

inline Array::iterator Array::Insert(const UnknownElement& element, iterator itWhere)
{ 
   return m_Elements.insert(itWhere, element);
}

inline Array::iterator Array::Insert(const UnknownElement& element)
{
   return Insert(element, End());
}

inline Array::iterator Array::Erase(iterator itWhere)
{ 
   return m_Elements.erase(itWhere);
}

inline void Array::Resize(size_t newSize)
{
   m_Elements.resize(newSize);
}

inline size_t Array::Size() const  { return m_Elements.size(); }
inline bool Array::Empty() const   { return m_Elements.empty(); }

inline UnknownElement& Array::operator[] (size_t index)
{
   size_t nMinSize = index + 1; // zero indexed
   if (m_Elements.size() < nMinSize)
      m_Elements.resize(nMinSize);
   return m_Elements[index]; 
}

inline const UnknownElement& Array::operator[] (size_t index) const 
{
   if (index >= m_Elements.size())
      throw Exception("Array out of bounds");
   return m_Elements[index]; 
}

inline void Array::Clear() {
   m_Elements.clear();
}

inline bool Array::operator == (const Array& array) const
{
   return m_Elements == array.m_Elements;
}


////////////////////////
// TrivialType_T members

template <typename DataTypeT>
TrivialType_T<DataTypeT>::TrivialType_T(const DataTypeT& t) :
   m_tValue(t) {}

template <typename DataTypeT>
TrivialType_T<DataTypeT>::operator DataTypeT&()
{
   return Value(); 
}

template <typename DataTypeT>
TrivialType_T<DataTypeT>::operator const DataTypeT&() const
{
   return Value(); 
}

template <typename DataTypeT>
DataTypeT& TrivialType_T<DataTypeT>::Value()
{
   return m_tValue; 
}

template <typename DataTypeT>
const DataTypeT& TrivialType_T<DataTypeT>::Value() const
{
   return m_tValue; 
}

template <typename DataTypeT>
bool TrivialType_T<DataTypeT>::operator == (const TrivialType_T<DataTypeT>& trivial) const
{
   return m_tValue == trivial.m_tValue;
}



//////////////////
// Null members

inline bool Null::operator == (const Null& trivial) const
{
   return true;
}



} // End namespace
