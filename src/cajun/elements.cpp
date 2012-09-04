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

#include <cassert>
#include <algorithm>
#include <map>
#include "visitor.h"
#include "reader.h"

/*  

TODO:
* better documentation

*/

namespace json
{


TPT_NO_INLINE Exception::Exception(const std::string& sMessage) :
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




TPT_NO_INLINE UnknownElement::UnknownElement() :                               m_pImp( new Imp_T<Null>( Null() ) ) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const UnknownElement& unknown) :  m_pImp( unknown.m_pImp->Clone()) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const Object& object) :           m_pImp( new Imp_T<Object>(object) ) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const Array& array) :             m_pImp( new Imp_T<Array>(array) ) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const Number& number) :           m_pImp( new Imp_T<Number>(number) ) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const Boolean& boolean) :         m_pImp( new Imp_T<Boolean>(boolean) ) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const String& string) :           m_pImp( new Imp_T<String>(string) ) {}
TPT_NO_INLINE UnknownElement::UnknownElement(const Null& null) :               m_pImp( new Imp_T<Null>(null) ) {}

TPT_NO_INLINE UnknownElement::~UnknownElement()   { delete m_pImp; }

TPT_NO_INLINE UnknownElement::operator const Object& () const    { return CastTo<Object>(); }
TPT_NO_INLINE UnknownElement::operator const Array& () const     { return CastTo<Array>(); }
TPT_NO_INLINE UnknownElement::operator const Number& () const    { return CastTo<Number>(); }
TPT_NO_INLINE UnknownElement::operator const Boolean& () const   { return CastTo<Boolean>(); }
TPT_NO_INLINE UnknownElement::operator const String& () const    { return CastTo<String>(); }
TPT_NO_INLINE UnknownElement::operator const Null& () const      { return CastTo<Null>(); }

TPT_NO_INLINE UnknownElement::operator Object& ()    { return ConvertTo<Object>(); }
TPT_NO_INLINE UnknownElement::operator Array& ()     { return ConvertTo<Array>(); }
TPT_NO_INLINE UnknownElement::operator Number& ()    { return ConvertTo<Number>(); }
TPT_NO_INLINE UnknownElement::operator Boolean& ()   { return ConvertTo<Boolean>(); }
TPT_NO_INLINE UnknownElement::operator String& ()    { return ConvertTo<String>(); }
TPT_NO_INLINE UnknownElement::operator Null& ()      { return ConvertTo<Null>(); }

TPT_NO_INLINE UnknownElement& UnknownElement::operator = (const UnknownElement& unknown) 
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

TPT_NO_INLINE UnknownElement& UnknownElement::operator[] (const std::string& key)
{
   // the people want an object. make us one if we aren't already
   Object& object = ConvertTo<Object>();
   return object[key];
}

TPT_NO_INLINE const UnknownElement& UnknownElement::operator[] (const std::string& key) const
{
   // throws if we aren't an object
   const Object& object = CastTo<Object>();
   return object[key];
}

TPT_NO_INLINE UnknownElement& UnknownElement::operator[] (size_t index)
{
   // the people want an array. make us one if we aren't already
   Array& array = ConvertTo<Array>();
   return array[index];
}

TPT_NO_INLINE const UnknownElement& UnknownElement::operator[] (size_t index) const
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


TPT_NO_INLINE void UnknownElement::Accept(ConstVisitor& visitor) const { m_pImp->Accept(visitor); }
TPT_NO_INLINE void UnknownElement::Accept(Visitor& visitor)            { m_pImp->Accept(visitor); }


TPT_NO_INLINE bool UnknownElement::operator == (const UnknownElement& element) const
{
   return m_pImp->Compare(*element.m_pImp);
}



//////////////////
// Object members


TPT_NO_INLINE Object::Member::Member(const std::string& nameIn, const UnknownElement& elementIn) :
   name(nameIn), element(elementIn) {}

TPT_NO_INLINE bool Object::Member::operator == (const Member& member) const 
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



TPT_NO_INLINE Object::iterator Object::Begin() { return m_Members.begin(); }
TPT_NO_INLINE Object::iterator Object::End() { return m_Members.end(); }
TPT_NO_INLINE Object::const_iterator Object::Begin() const { return m_Members.begin(); }
TPT_NO_INLINE Object::const_iterator Object::End() const { return m_Members.end(); }

TPT_NO_INLINE size_t Object::Size() const { return m_Members.size(); }
TPT_NO_INLINE bool Object::Empty() const { return m_Members.empty(); }

TPT_NO_INLINE Object::iterator Object::Find(const std::string& name) 
{
   return std::find_if(m_Members.begin(), m_Members.end(), Finder(name));
}

TPT_NO_INLINE Object::const_iterator Object::Find(const std::string& name) const 
{
   return std::find_if(m_Members.begin(), m_Members.end(), Finder(name));
}

TPT_NO_INLINE Object::iterator Object::Insert(const Member& member)
{
   return Insert(member, End());
}

TPT_NO_INLINE Object::iterator Object::Insert(const Member& member, iterator itWhere)
{
   iterator it = Find(member.name);
   if (it != m_Members.end())
      throw Exception(std::string("Object member already exists: ") + member.name);

   it = m_Members.insert(itWhere, member);
   return it;
}

TPT_NO_INLINE Object::iterator Object::Erase(iterator itWhere) 
{
   return m_Members.erase(itWhere);
}

TPT_NO_INLINE UnknownElement& Object::operator [](const std::string& name)
{

   iterator it = Find(name);
   if (it == m_Members.end())
   {
      Member member(name);
      it = Insert(member, End());
   }
   return it->element;      
}

TPT_NO_INLINE const UnknownElement& Object::operator [](const std::string& name) const 
{
   const_iterator it = Find(name);
   if (it == End())
      throw Exception(std::string("Object member not found: ") + name);
   return it->element;
}

TPT_NO_INLINE void Object::Clear() 
{
   m_Members.clear(); 
}

TPT_NO_INLINE bool Object::operator == (const Object& object) const 
{
   return m_Members == object.m_Members;
}


/////////////////
// Array members

TPT_NO_INLINE Array::iterator Array::Begin()  { return m_Elements.begin(); }
TPT_NO_INLINE Array::iterator Array::End()    { return m_Elements.end(); }
TPT_NO_INLINE Array::const_iterator Array::Begin() const  { return m_Elements.begin(); }
TPT_NO_INLINE Array::const_iterator Array::End() const    { return m_Elements.end(); }

TPT_NO_INLINE Array::iterator Array::Insert(const UnknownElement& element, iterator itWhere)
{ 
   return m_Elements.insert(itWhere, element);
}

TPT_NO_INLINE Array::iterator Array::Insert(const UnknownElement& element)
{
   return Insert(element, End());
}

TPT_NO_INLINE Array::iterator Array::Erase(iterator itWhere)
{ 
   return m_Elements.erase(itWhere);
}

TPT_NO_INLINE void Array::Resize(size_t newSize)
{
   m_Elements.resize(newSize);
}

TPT_NO_INLINE size_t Array::Size() const  { return m_Elements.size(); }
TPT_NO_INLINE bool Array::Empty() const   { return m_Elements.empty(); }

TPT_NO_INLINE UnknownElement& Array::operator[] (size_t index)
{
   size_t nMinSize = index + 1; // zero indexed
   if (m_Elements.size() < nMinSize)
      m_Elements.resize(nMinSize);
   return m_Elements[index]; 
}

TPT_NO_INLINE const UnknownElement& Array::operator[] (size_t index) const 
{
   if (index >= m_Elements.size())
      throw Exception("Array out of bounds");
   return m_Elements[index]; 
}

TPT_NO_INLINE void Array::Clear() {
   m_Elements.clear();
}

TPT_NO_INLINE bool Array::operator == (const Array& array) const
{
   return m_Elements == array.m_Elements;
}


//////////////////
// Null members

TPT_NO_INLINE bool Null::operator == (const Null& trivial) const
{
   return true;
}



} // End namespace
