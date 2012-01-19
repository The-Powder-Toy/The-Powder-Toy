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

#pragma once

#include "elements.h"
#include "visitor.h"

namespace json
{

class Writer : private ConstVisitor
{
public:
   static void Write(const Object& object, std::ostream& ostr);
   static void Write(const Array& array, std::ostream& ostr);
   static void Write(const String& string, std::ostream& ostr);
   static void Write(const Number& number, std::ostream& ostr);
   static void Write(const Boolean& boolean, std::ostream& ostr);
   static void Write(const Null& null, std::ostream& ostr);
   static void Write(const UnknownElement& elementRoot, std::ostream& ostr);

private:
   Writer(std::ostream& ostr);

   template <typename ElementTypeT>
   static void Write_i(const ElementTypeT& element, std::ostream& ostr);

   void Write_i(const Object& object);
   void Write_i(const Array& array);
   void Write_i(const String& string);
   void Write_i(const Number& number);
   void Write_i(const Boolean& boolean);
   void Write_i(const Null& null);
   void Write_i(const UnknownElement& unknown);

   virtual void Visit(const Array& array);
   virtual void Visit(const Object& object);
   virtual void Visit(const Number& number);
   virtual void Visit(const String& string);
   virtual void Visit(const Boolean& boolean);
   virtual void Visit(const Null& null);

   std::ostream& m_ostr;
   int m_nTabDepth;
};


} // End namespace


#include "writer.inl"