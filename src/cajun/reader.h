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
#include <iostream>
#include <vector>

namespace json
{

class Reader
{
public:
   // this structure will be reported in one of the exceptions defined below
   struct Location
   {
      Location();

      unsigned int m_nLine;       // document line, zero-indexed
      unsigned int m_nLineOffset; // character offset from beginning of line, zero indexed
      unsigned int m_nDocOffset;  // character offset from entire document, zero indexed
   };

   // thrown during the first phase of reading. generally catches low-level problems such
   //  as errant characters or corrupt/incomplete documents
   class ScanException : public Exception
   {
   public:
      ScanException(const std::string& sMessage, const Reader::Location& locError) :
         Exception(sMessage),
         m_locError(locError) {}

      Reader::Location m_locError;
   };

   // thrown during the second phase of reading. generally catches higher-level problems such
   //  as missing commas or brackets
   class ParseException : public Exception
   {
   public:
      ParseException(const std::string& sMessage, const Reader::Location& locTokenBegin, const Reader::Location& locTokenEnd) :
         Exception(sMessage),
         m_locTokenBegin(locTokenBegin),
         m_locTokenEnd(locTokenEnd) {}

      Reader::Location m_locTokenBegin;
      Reader::Location m_locTokenEnd;
   };


   // if you know what the document looks like, call one of these...
   static void Read(Object& object, std::istream& istr);
   static void Read(Array& array, std::istream& istr);
   static void Read(String& string, std::istream& istr);
   static void Read(Number& number, std::istream& istr);
   static void Read(Boolean& boolean, std::istream& istr);
   static void Read(Null& null, std::istream& istr);

   // ...otherwise, if you don't know, call this & visit it
   static void Read(UnknownElement& elementRoot, std::istream& istr);

private:
   struct Token
   {
      enum Type
      {
         TOKEN_OBJECT_BEGIN,  //    {
         TOKEN_OBJECT_END,    //    }
         TOKEN_ARRAY_BEGIN,   //    [
         TOKEN_ARRAY_END,     //    ]
         TOKEN_NEXT_ELEMENT,  //    ,
         TOKEN_MEMBER_ASSIGN, //    :
         TOKEN_STRING,        //    "xxx"
         TOKEN_NUMBER,        //    [+/-]000.000[e[+/-]000]
         TOKEN_BOOLEAN,       //    true -or- false
         TOKEN_NULL,          //    null
      };

      Type nType;
      std::string sValue;

      // for malformed file debugging
      Reader::Location locBegin;
      Reader::Location locEnd;
   };

   class InputStream;
   class TokenStream;
   typedef std::vector<Token> Tokens;

   template <typename ElementTypeT>   
   static void Read_i(ElementTypeT& element, std::istream& istr);

   // scanning istream into token sequence
   void Scan(Tokens& tokens, InputStream& inputStream);

   void EatWhiteSpace(InputStream& inputStream);
   std::string MatchString(InputStream& inputStream);
   std::string MatchNumber(InputStream& inputStream);
   std::string MatchExpectedString(InputStream& inputStream, const std::string& sExpected);

   // parsing token sequence into element structure
   void Parse(UnknownElement& element, TokenStream& tokenStream);
   void Parse(Object& object, TokenStream& tokenStream);
   void Parse(Array& array, TokenStream& tokenStream);
   void Parse(String& string, TokenStream& tokenStream);
   void Parse(Number& number, TokenStream& tokenStream);
   void Parse(Boolean& boolean, TokenStream& tokenStream);
   void Parse(Null& null, TokenStream& tokenStream);

   const std::string& MatchExpectedToken(Token::Type nExpected, TokenStream& tokenStream);
};


} // End namespace


#include "reader.inl"