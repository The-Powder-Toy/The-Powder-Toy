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

#include <iostream>
#include <iomanip>
#include "writer.h"

/*  

TODO:
* better documentation
* unicode character encoding

*/

namespace json
{


TPT_NO_INLINE void Writer::Write(const UnknownElement& elementRoot, std::ostream& ostr) { Write_i(elementRoot, ostr); }
TPT_NO_INLINE void Writer::Write(const Object& object, std::ostream& ostr)              { Write_i(object, ostr); }
TPT_NO_INLINE void Writer::Write(const Array& array, std::ostream& ostr)                { Write_i(array, ostr); }
TPT_NO_INLINE void Writer::Write(const Number& number, std::ostream& ostr)              { Write_i(number, ostr); }
TPT_NO_INLINE void Writer::Write(const String& string, std::ostream& ostr)              { Write_i(string, ostr); }
TPT_NO_INLINE void Writer::Write(const Boolean& boolean, std::ostream& ostr)            { Write_i(boolean, ostr); }
TPT_NO_INLINE void Writer::Write(const Null& null, std::ostream& ostr)                  { Write_i(null, ostr); }


TPT_NO_INLINE Writer::Writer(std::ostream& ostr) :
   m_ostr(ostr),
   m_nTabDepth(0)
{}

template <typename ElementTypeT>
void Writer::Write_i(const ElementTypeT& element, std::ostream& ostr)
{
   Writer writer(ostr);
   writer.Write_i(element);
   ostr.flush(); // all done
}

TPT_NO_INLINE void Writer::Write_i(const Array& array)
{
   if (array.Empty())
      m_ostr << "[]";
   else
   {
      m_ostr << '[' << std::endl;
      ++m_nTabDepth;

      Array::const_iterator it(array.Begin()),
                            itEnd(array.End());
      while (it != itEnd) {
         m_ostr << std::string(m_nTabDepth, '\t');
         
         Write_i(*it);

         if (++it != itEnd)
            m_ostr << ',';
         m_ostr << std::endl;
      }

      --m_nTabDepth;
      m_ostr << std::string(m_nTabDepth, '\t') << ']';
   }
}

TPT_NO_INLINE void Writer::Write_i(const Object& object)
{
   if (object.Empty())
      m_ostr << "{}";
   else
   {
      m_ostr << '{' << std::endl;
      ++m_nTabDepth;

      Object::const_iterator it(object.Begin()),
                             itEnd(object.End());
      while (it != itEnd) {
         m_ostr << std::string(m_nTabDepth, '\t');
         
         Write_i(it->name);

         m_ostr << " : ";
         Write_i(it->element); 

         if (++it != itEnd)
            m_ostr << ',';
         m_ostr << std::endl;
      }

      --m_nTabDepth;
      m_ostr << std::string(m_nTabDepth, '\t') << '}';
   }
}

TPT_NO_INLINE void Writer::Write_i(const Number& numberElement)
{
   m_ostr << std::setprecision(20) << numberElement.Value();
}

TPT_NO_INLINE void Writer::Write_i(const Boolean& booleanElement)
{
   m_ostr << (booleanElement.Value() ? "true" : "false");
}

TPT_NO_INLINE void Writer::Write_i(const String& stringElement)
{
   m_ostr << '"';

   const std::string& s = stringElement.Value();
   std::string::const_iterator it(s.begin()),
                               itEnd(s.end());
   for (; it != itEnd; ++it)
   {
      switch (*it)
      {
         case '"':         m_ostr << "\\\"";   break;
         case '\\':        m_ostr << "\\\\";   break;
         case '\b':        m_ostr << "\\b";    break;
         case '\f':        m_ostr << "\\f";    break;
         case '\n':        m_ostr << "\\n";    break;
         case '\r':        m_ostr << "\\r";    break;
         case '\t':        m_ostr << "\\t";    break;
		 //case '\u':        m_ostr << "\\u";    break; // uh...
         default:          m_ostr << *it;      break;
      }
   }

   m_ostr << '"';   
}

TPT_NO_INLINE void Writer::Write_i(const Null& )
{
   m_ostr << "null";
}

TPT_NO_INLINE void Writer::Write_i(const UnknownElement& unknown)
{
   unknown.Accept(*this); 
}

TPT_NO_INLINE void Writer::Visit(const Array& array)       { Write_i(array); }
TPT_NO_INLINE void Writer::Visit(const Object& object)     { Write_i(object); }
TPT_NO_INLINE void Writer::Visit(const Number& number)     { Write_i(number); }
TPT_NO_INLINE void Writer::Visit(const String& string)     { Write_i(string); }
TPT_NO_INLINE void Writer::Visit(const Boolean& boolean)   { Write_i(boolean); }
TPT_NO_INLINE void Writer::Visit(const Null& null)         { Write_i(null); }



} // End namespace
